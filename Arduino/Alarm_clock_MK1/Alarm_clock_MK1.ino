

#include "mainSettings.h"
#include "EepromDB.h"

#ifdef TRACE_ON
#define TRACE_CLOCK 1
//#define DEBUG_HIGHRES_ALARMSET 1
#endif

#define REFERENCE_AGE_FOR_CHECK 30
#define REFERENCE_AGE_FOR_NOTIFY 90
#define REFERENCE_AGE_FOR_INVALID 144000

const unsigned long demo_delaytime=400;

/* Some general knowledge about time */
#define MINUTES_PER_DAY 1440
#define MILLIES_PER_DAY 86400000
#define MILLIES_PER_MINUTE 60000

/* Clock variables */

enum CLOCK_STATE {
  STATE_IDLE, // show time, check for alarms, check age of rds time 
  STATE_WAKEUP,
  STATE_STOP_PROCEDURE,
  STATE_ALARM_INFO,  // Show Alarm time
  STATE_ALARM_CHANGE,  // Change and set Alarm time
  STATE_SLEEP_SET,  // Activate Sleep and change sleep time
  STATE_PRESET_SELECT, // Select playing station
  STATE_DEBUG,  // Show some explicit values for debugging
  STATE_DEMO  // show fast time progressing
};
CLOCK_STATE clock_state = STATE_IDLE; 

#define TIME_UNKNOWN -1
#define TRIGGER_IS_OFF -2

#define RDS_TRUST_GOOD 20         // Highest trust you can get
#define RDS_TRUST_ACCEPTABLE 10   // Minimal trust to be used
#define RDS_TRUST_CANDIDATE 4     // Number of consecutive occurences to be protected from instant replacement
#define RDS_DERIVATION_TOLERANCE 10000 // half of millisecods we accept derivation from progress calculateion
#define RDS_SCAN_COOLDOWN_TIME  750   // Milliseconds before we use the next RDS Time Telegram
//#define RDS_MAX_WAIT_TIME  300000    // after 5 Minutes (300 seconds) with no signal we change channel 
#define RDS_MAX_WAIT_TIME  150000    // after 2,5 Minutes (150 seconds) with no signal we change channel 
#define BAD_TRY_UNTIL_CHANNEL_CHANGE 14 // Number of unsuccedfull tries until we change RDS channel

#define MAX_STATION_TRUST 10

unsigned long clock_sync_time=millis();
MinutesOfDay_t clock_reference_time=TIME_UNKNOWN;
int clock_rds_trust=0;

unsigned long candidate_sync_time=millis();
MinutesOfDay_t candidate_reference_time=TIME_UNKNOWN;
int candidate_rds_trust=0;

int station_trust=MAX_STATION_TRUST;

MinutesOfDay_t clock_alarm_time[4]={0,0,0,0};
byte clock_focussed_alarmIndex=0; 
MinutesOfDay_t clock_sleep_stop_time=TRIGGER_IS_OFF;
MinutesOfDay_t clock_alarm_stop_time=TRIGGER_IS_OFF;
MinutesOfDay_t clock_snooze_stop_time=TRIGGER_IS_OFF;
byte clock_alarm_stopprocedure_progress=0;
unsigned long clock_last_wakeup_stop_millis=0;  

EepromDB clockSettingsDB;

struct clockSettingRecord {
  MinutesOfDay_t alarm_time;
  byte radio_preset;
}; 

/* ************************************************************
 *     Main Setup
 * ************************************************************
 */

void setup() 
{
  
  #ifdef TRACE_ON 
     char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
     Serial.begin(9600);
     Serial.println(compile_signature); 
  #endif

  struct clockSettingRecord previous_settings;
   
  radio_setup();
  output_setup();
  input_setup(0, MINUTES_PER_DAY-1,15); /* Encoder Range 24 hoursis,stepping quater hours */
  clockSettingsDB.setupDB(0, sizeof(previous_settings), 4);
  clock_alarm_time[0]=DEFAULT_ALARM_TIME; /* Fall back alarm */

  /* get settings from eeprom */
  if(clockSettingsDB.readRecord((byte*)(&previous_settings)))
  {
    clock_alarm_time[0]=previous_settings.alarm_time;
    radio_setSelectedPreset(previous_settings.radio_preset);
  }
}


/* ****************Main Loop********************************** 
 * ************************************************************
 */

void loop() { 


  /* Inputs */
  input_switches_scan_tick();
  radio_loop_tick();


  /* Sleep Timer logic */
  if(clock_getCurrentTime()==clock_sleep_stop_time) {
    radio_switchOff();
    clock_sleep_stop_time=TRIGGER_IS_OFF;
    #ifdef TRACE_CLOCK
         Serial.println(F("SLEEP timed out"));
    #endif
  }

  /* Alarm on logic */
  if(clock_getCurrentTime()==clock_alarm_time[clock_focussed_alarmIndex]
   &&clock_alarm_stop_time==TRIGGER_IS_OFF          // TBD: for multi alarm time, check if new alarm is running
   &&clock_state != STATE_ALARM_CHANGE
   &&clock_state != STATE_WAKEUP
   &&input_masterSwitchIsSet()
   &&(millis()-clock_last_wakeup_stop_millis>60000)  // after active stop, we must wait at least 60 seconds to perevent reactivation in case of fast reaction
   ) {
      enter_STATE_WAKEUP();
      return;
  }

  /* Snooze end logic */
  if(clock_getCurrentTime()==clock_snooze_stop_time) {
    resume_STATE_WAKEUP();
    return;
  }

  /* Alarm off logic */
  if(clock_getCurrentTime()==clock_alarm_stop_time) {
    exit_STATE_WAKEUP();
    return;
  }

  
  /* UI logic */
  switch (clock_state) {
    
    case STATE_IDLE:            process_STATE_IDLE(); break;
          
    case STATE_ALARM_INFO:      process_STATE_ALARM_INFO(); break;
          
    case STATE_ALARM_CHANGE:    process_STATE_ALARM_CHANGE(); break;

    case STATE_WAKEUP:          process_STATE_WAKEUP(); break;
    case STATE_STOP_PROCEDURE:  process_STATE_STOP_PROCEDURE(); break;
    
    case STATE_SLEEP_SET:       process_STATE_SLEEP_SET(); break;

    case STATE_PRESET_SELECT:  process_STATE_PRESET_SELECT(); break;
     
    case STATE_DEBUG:  process_STATE_DEBUG(); break;
    
    case STATE_DEMO:  process_STATE_DEMO(); break;

 /* ------------------------------------------------------------------ */  
  
    default: output_sequence_alert(); 
      enter_STATE_IDLE();
  }

  /* output */
  /* is managed by scenes and sequences call from state functions */
  /* --- no output code here ------*/

  #ifdef TRACE_CLOCK
  static MinutesOfDay_t last_traced_time;
  if( last_traced_time!=clock_getCurrentTime()) {
    last_traced_time=clock_getCurrentTime();
    Serial.print(F("Cur :"));
    Serial.print(last_traced_time/60);
    Serial.print(":");
    Serial.print(last_traced_time%60);  
    Serial.print(F(" Alm:"));
    Serial.print(clock_alarm_time[clock_focussed_alarmIndex]/60);
    Serial.print(F(":"));
    Serial.println(clock_alarm_time[clock_focussed_alarmIndex]%60);  
  }
  #endif

}

/* ************************************************************
 *     Clock functions
 * ************************************************************
 */

/*  ------------  Information ----------------- */

MinutesOfDay_t clock_getCurrentTime() {
  if(clock_rds_trust <RDS_TRUST_ACCEPTABLE) return TIME_UNKNOWN;
  return (((millis()-clock_sync_time)/60000)+clock_reference_time)%MINUTES_PER_DAY;
}

unsigned long clock_getSyncTime(){
  return clock_sync_time; 
}

MinutesOfDay_t clock_getAgeOfReference(){
  if (clock_sync_time==0) {  /* Seems we dont have any data */
    if(millis()<REFERENCE_AGE_FOR_NOTIFY) return REFERENCE_AGE_FOR_CHECK; /* Warn gracefully */
    return millis();  /* We get worried */
  }
  return (millis()-clock_sync_time)/MILLIES_PER_MINUTE;
}

int clock_getClockRDSScore()
{
  return max((clock_rds_trust*256)/ RDS_TRUST_GOOD,0);
}

int clock_getCandidateRDSScore()
{
  return max((candidate_rds_trust*256)/RDS_TRUST_ACCEPTABLE,0);
}


/* ############  Operation  ############## */

/* -------------- evaluateNewRDSTime ----------------------         
 * Evaluates the new given time 
 */

void clock_evaluateRDSTime(MinutesOfDay_t measured_time) {
static unsigned long evaluation_time;


  if( millis()-evaluation_time < RDS_SCAN_COOLDOWN_TIME     /* Dont accept values too fast  */
   || radio_getRdsTimeAge() >= RDS_SCAN_COOLDOWN_TIME  ) return;       /* Only accept fresh values in relation to cooldown  */

  evaluation_time=millis();

  
  /* change station, if we lost trust or it is not sending usable RDS timestamps */
  if( millis()-candidate_sync_time > RDS_MAX_WAIT_TIME || station_trust<=0) { 
     #ifdef TRACE_CLOCK
        Serial.println(F("RDS Timeout"));
     #endif 
     radio_switchRdsStation();  
     station_trust=MAX_STATION_TRUST;
     candidate_sync_time=evaluation_time;   
  }
  #ifdef TRACE_CLOCK
        static byte prev_second=0;
        byte cursecond=(millis()-candidate_sync_time)/10000;
        if(cursecond!=prev_second) {
          prev_second=cursecond;
          Serial.print(F("No RDS Time since:"));
          Serial.println(cursecond*10);
        }
  #endif

  /* reject rubbish */
  if(measured_time>MINUTES_PER_DAY) 
  {
    station_trust--;
    return; 
  }

   /* compare to progress calculation */
  int candidate_progress_time=(((evaluation_time-candidate_sync_time+RDS_DERIVATION_TOLERANCE)/60000)+candidate_reference_time)%MINUTES_PER_DAY;
  
   #ifdef TRACE_CLOCK
      Serial.print(F("RDS:"));
      trace_printTime(measured_time);
   #endif
     
  if(measured_time==candidate_progress_time) { /* RDS delivered in expected range  */
      if(candidate_rds_trust<RDS_TRUST_GOOD) candidate_rds_trust+=1;
      candidate_sync_time=evaluation_time; 
      candidate_reference_time=measured_time; 
      if(station_trust<MAX_STATION_TRUST) station_trust++;   
      #ifdef TRACE_CLOCK
       Serial.print(F("\tgood"));
      #endif 

      if(candidate_rds_trust>=clock_rds_trust) 
      {
        clock_sync_time=candidate_sync_time;
        clock_reference_time=candidate_reference_time;
        clock_rds_trust=candidate_rds_trust;
        #ifdef TRACE_CLOCK
          Serial.print(F("\tused by clock"));
        #endif 
      }
 
  } else {

     if(candidate_rds_trust>=RDS_TRUST_CANDIDATE) {
      candidate_rds_trust-=2; 
      #ifdef TRACE_CLOCK
       Serial.print(F("\treject"));
      #endif         
     } else  { /* Only, when our trust has gone , we try the new RDS value */
      station_trust--;
      candidate_sync_time=evaluation_time; 
      candidate_reference_time=measured_time;      
      candidate_rds_trust=0;
      #ifdef TRACE_CLOCK
       Serial.print(("\texchange"));
      #endif    
     }
  }
  
 #ifdef TRACE_CLOCK
    Serial.print(F("\tcandidate trust:"));
    Serial.print(candidate_rds_trust);
    Serial.print(F("\tclock trust:"));
    Serial.print(clock_rds_trust);
    Serial.print(F("\tstation trust:"));
    Serial.println(station_trust);
  #endif

}
void storeSettings() 
{
   struct clockSettingRecord current_settings;

  /* write settings to eeprom */
  current_settings.alarm_time=clock_alarm_time[0];
  current_settings.radio_preset=radio_getSelectedPreset();
  clockSettingsDB.updateRecord((byte*)(&current_settings));

}


/* ###########  Output helper ########## */

void callIdleScene(int minute_of_the_day, byte alarmIndicator)
{
  if(minute_of_the_day!=TIME_UNKNOWN) {
    output_renderClockScene((minute_of_the_day+PRESENTATION_SHIFT_MINUTES)%MINUTES_PER_DAY,alarmIndicator);
  }
  else output_renderTimeTrustScene();
}

void trace_printTime(int timeValue)
{
    Serial.print(timeValue/60);
    Serial.print(F(":"));
    Serial.print(timeValue%60);  
}


/* ************************************************************
 *    STATE functions
 * ************************************************************
 */

/* *************** STATE_IDLE ***************** */

void enter_STATE_IDLE(){
     clock_state=STATE_IDLE;
     callIdleScene(clock_getCurrentTime(),input_masterSwitchIsSet()?ALARM_INDICATOR_ON:ALARM_INDICATOR_OFF);
     #ifdef TRACE_CLOCK
     Serial.println(F("#IDLE"));
     #endif
}

void process_STATE_IDLE(){
 if(input_snoozeGotPressed()) {    
    enter_STATE_SLEEP_SET();
    return;
  }
  
  if(input_hasEncoderChangeEvent()) {enter_STATE_ALARM_INFO();return;}
  if(input_selectGotPressed()) {enter_STATE_ALARM_CHANGE();return;}
 
  /* Check RDS Data when necessary */
  if(clock_getAgeOfReference()>REFERENCE_AGE_FOR_CHECK ||
     clock_rds_trust<RDS_TRUST_GOOD) {
     radio_setRdsScanActive(true);
     clock_evaluateRDSTime(radio_getLastRdsTimeInfo());
  }  else radio_setRdsScanActive(false);       

  callIdleScene(clock_getCurrentTime()
                             ,input_masterSwitchIsSet()?ALARM_INDICATOR_ON:ALARM_INDICATOR_OFF);  
}

/* *************** STATE_WAKEUP ***************** */

void enter_STATE_WAKEUP(){
   clock_state=STATE_WAKEUP;
   if(clock_alarm_stop_time==TRIGGER_IS_OFF) radio_fadeIn();
   clock_sleep_stop_time=TRIGGER_IS_OFF;
   clock_alarm_stop_time=clock_getCurrentTime()+ALARM_DURATION ;
   clock_snooze_stop_time=TRIGGER_IS_OFF;
   
   #ifdef TRACE_CLOCK
     Serial.println(F("#WAKEUP"));
   #endif
   callIdleScene(clock_getCurrentTime()
                               ,clock_snooze_stop_time==TRIGGER_IS_OFF?0:ALARM_INDICATOR_SNOOZE
                               |(input_masterSwitchIsSet()?ALARM_INDICATOR_ON :ALARM_INDICATOR_OFF));
}

void resume_STATE_WAKEUP(){
   clock_state=STATE_WAKEUP;
   clock_snooze_stop_time=TRIGGER_IS_OFF;
   radio_switchOn();
   #ifdef TRACE_CLOCK
     Serial.println(F(">WAKEUP"));
   #endif
}

void process_STATE_WAKEUP(){
  byte alarmIndicator=0;
 if(input_snoozeGotPressed()) {  
    clock_alarm_stop_time=clock_getCurrentTime()+ALARM_DURATION ;   
    clock_snooze_stop_time=clock_getCurrentTime()+SNOOZE_DURATION;
    radio_switchOff();
    output_sequence_acknowlegde();
   #ifdef TRACE_CLOCK
     Serial.print(F("WAKEUP:Snooze "));
     trace_printTime(clock_snooze_stop_time);
     Serial.println();
   #endif
    return;
  }
  
  if(input_hasEncoderChangeEvent()) {enter_STATE_STOP_PROCEDURE();return;}

  if( clock_snooze_stop_time!=TRIGGER_IS_OFF) alarmIndicator|=ALARM_INDICATOR_SNOOZE;
  if(!input_masterSwitchIsSet()) alarmIndicator|=ALARM_INDICATOR_OFF;

  callIdleScene(clock_getCurrentTime()
                             ,alarmIndicator);

}

void exit_STATE_WAKEUP() {
   #ifdef TRACE_CLOCK
     Serial.println(F("<WAKEUP"));
   #endif
  radio_switchOff();
  clock_snooze_stop_time=TRIGGER_IS_OFF;
  clock_alarm_stop_time=TRIGGER_IS_OFF; 
  clock_sleep_stop_time=TRIGGER_IS_OFF; 
  if(clock_state==STATE_WAKEUP) enter_STATE_IDLE();
}

/* *************** STATE_STOP_PROCEDURE ***************** */

void enter_STATE_STOP_PROCEDURE(){
   clock_state=STATE_STOP_PROCEDURE;
   input_setEncoderRange(-20,20,1,20);  
   #ifdef TRACE_CLOCK
     Serial.println(F("#STOP_PROCEDURE"));
   #endif
   output_renderStopProcedureScene(input_getEncoderValue());
}


void process_STATE_STOP_PROCEDURE(){
 if(input_snoozeGotPressed()) {     
    #ifdef TRACE_CLOCK
     Serial.println(F("STOP_PROCEDURE:Snoozing"));
    #endif
    clock_alarm_stop_time=clock_getCurrentTime()+ALARM_DURATION ;  
    clock_snooze_stop_time=clock_getCurrentTime()+SNOOZE_DURATION;
    output_sequence_acknowlegde();
    radio_switchOff();
    resume_STATE_WAKEUP();
  }

  if(input_getEncoderValue()==0) {
    clock_last_wakeup_stop_millis=millis();
    output_sequence_acknowlegde();
    exit_STATE_WAKEUP();
    enter_STATE_IDLE();
    return;
  }

  if(input_getSecondsSinceLastEvent()> SECONDS_UNTIL_FALLBACK_SHORT)
  {
    resume_STATE_WAKEUP();
    return;
  }
  
  output_renderStopProcedureScene(input_getEncoderValue());
}

  


/* *************** STATE_ALARM_INFO ***************** */

void enter_STATE_ALARM_INFO(){
  
  clock_state=STATE_ALARM_INFO; 
  input_getEncoderValue(); // Read out Encoder to get of the last event 
  output_renderClockScene( clock_alarm_time[clock_focussed_alarmIndex],ALARM_INDICATOR_SHOW);
  #ifdef TRACE_CLOCK
         Serial.println(F("#ALARM_INFO"));
         trace_printTime( clock_alarm_time[clock_focussed_alarmIndex]);
         Serial.println();
  #endif
}

void process_STATE_ALARM_INFO(){
     if(input_snoozeGotPressed()
        || input_getSecondsSinceLastEvent()> SECONDS_UNTIL_FALLBACK_SHORT) {
            enter_STATE_IDLE(); return;
          }
    
    if(input_hasEncoderChangeEvent()) input_getEncoderValue(); 
    
    if(input_selectGotPressed()) {
             enter_STATE_ALARM_CHANGE();return;
             return;
    }
   output_renderClockScene( clock_alarm_time[clock_focussed_alarmIndex],ALARM_INDICATOR_SHOW);
  
}

/* *************** STATE_ALARM_CHANGE ***************** */

void enter_STATE_ALARM_CHANGE(){
  clock_state=STATE_ALARM_CHANGE; 
  input_setEncoderRange(0, MINUTES_PER_DAY-1,5,clock_alarm_time[clock_focussed_alarmIndex]);
  #ifdef DEBUG_HIGHRES_ALARMSET
    input_setEncoderRange(0, MINUTES_PER_DAY-1,1,clock_alarm_time[clock_focussed_alarmIndex]);
  #endif
  output_renderClockScene(input_getEncoderValue(),ALARM_INDICATOR_EDIT);
   #ifdef TRACE_CLOCK
         Serial.println(F("#ALARM_CHANGE"));
   #endif
}

void process_STATE_ALARM_CHANGE(){
      if(input_snoozeGotPressed() 
             || input_getSecondsSinceLastEvent()> SECONDS_UNTIL_FALLBACK_LONG) {
             output_sequence_escape();
             enter_STATE_IDLE();return;
          }

      if(input_selectGotPressed()) {
        output_sequence_acknowlegde();
        clock_alarm_time[clock_focussed_alarmIndex]=input_getEncoderValue();
        storeSettings() ;
        enter_STATE_IDLE();return;       
      }

  #ifdef DEBUG_HIGHRES_ALARMSET
    if(input_hasEncoderChangeEvent()) {
      trace_printTime(input_getEncoderValue());
      Serial.println();
    }
  #endif
  
  output_renderClockScene(input_getEncoderValue(),ALARM_INDICATOR_EDIT); 
}


/* *************** STATE_SLEEP_SET ***************** */

void enter_STATE_SLEEP_SET(){
  clock_state=STATE_SLEEP_SET; 
  int sleepMinutes=45;
  if(clock_sleep_stop_time!=TRIGGER_IS_OFF) sleepMinutes=clock_sleep_stop_time-clock_getCurrentTime();
  if(sleepMinutes<0) sleepMinutes+MINUTES_PER_DAY;
  input_setEncoderRange(0, 75,5,sleepMinutes); // 65+ for Setting Screens
  output_renderSleepScene(input_getEncoderValue());
  radio_switchOn();
  #ifdef TRACE_CLOCK
         Serial.println(F("#SLEEP_SET"));
  #endif
}

void process_STATE_SLEEP_SET(){
  if(input_snoozeGotPressed() )
  {
    #ifdef TRACE_CLOCK
         Serial.println(F("SLEEP cancel by snooze"));
    #endif
    radio_switchOff(); /* TBD: only if we are still in a wakeup interval */
    output_sequence_escape();
    enter_STATE_IDLE();
    return;
  }

   if(input_getEncoderValue()<=60) {  // normal sleep setting
      if(input_selectGotPressed() || input_getSecondsSinceLastEvent()> SECONDS_UNTIL_FALLBACK_SHORT)
      {
      if(input_getEncoderValue()==0) {
        #ifdef TRACE_CLOCK
           Serial.println(F("SLEEP cancel by 0"));
        #endif
         radio_switchOff(); 
         clock_sleep_stop_time=TRIGGER_IS_OFF;
         enter_STATE_IDLE();
        return;
      }
      clock_sleep_stop_time=(input_getEncoderValue()+clock_getCurrentTime()) % MINUTES_PER_DAY;
      output_sequence_acknowlegde();
      #ifdef TRACE_CLOCK
           Serial.print(F("SLEEP until "));
           trace_printTime(clock_sleep_stop_time);
           Serial.println();         
      #endif
      enter_STATE_IDLE();
      return;       
    }
    output_renderSleepScene(input_getEncoderValue());
  } else { // out of sleep setting range, so we reach demo and debug modes

     if(input_getSecondsSinceLastEvent()> SECONDS_UNTIL_FALLBACK_SHORT) 
     {
        radio_switchOff(); 
        clock_sleep_stop_time=TRIGGER_IS_OFF;
        enter_STATE_IDLE();
        return;
     }
     
     switch(input_getEncoderValue()) {
      case 65:
            if(input_selectGotPressed()) 
            {
              enter_STATE_PRESET_SELECT();
              return;  
            }
            output_renderLetterScene(4); // P
            break; 
      case 70:
            if(input_selectGotPressed()) 
            {
              enter_STATE_DEMO();
              return;  
            }
            output_renderLetterScene(0); // D
            break;  
      case 75:
            if(input_selectGotPressed()) 
            {
              enter_STATE_DEBUG();
              return;  
            }
            output_renderLetterScene(1); // D
            break;     
      default:
            output_sequence_alert();          
     }
  }
}


/* *************** STATE_PRESET_SELECT ***************** */

void enter_STATE_PRESET_SELECT(){
  
  clock_state=STATE_PRESET_SELECT; 
  input_getEncoderValue(); // Read out Encoder to get of the last event 
  input_setEncoderRange(0, RADIO_PRESET_COUNT-1,1,radio_getSelectedPreset()); 
  
  output_renderPresetSelectScene(input_getEncoderValue());
  #ifdef TRACE_CLOCK
         Serial.println(F("#PRESET_SELECT"));
  #endif
}

void process_STATE_PRESET_SELECT(){
    if(input_selectGotPressed()||input_getSecondsSinceLastEvent()> SECONDS_UNTIL_FALLBACK_LONG
        ) {
          storeSettings();
            enter_STATE_IDLE();
            return;
    }
    if(input_snoozeGotPressed()) {
        if(radio_isPlaying()) radio_switchOff();
        else radio_switchOn();
    }
    
    if(input_hasEncoderChangeEvent()) {
      radio_setSelectedPreset(input_getEncoderValue()); 
    }
    
   output_renderPresetSelectScene(input_getEncoderValue());
  
}
/* *************** STATE_DEBUG ***************** */
void enter_STATE_DEBUG(){
  clock_state=STATE_DEBUG; 
  input_setEncoderRange(0, 6,1,0);
  input_setEncoderValue(0);
  #ifdef TRACE_CLOCK
         Serial.println(F("#DEBUG"));
  #endif
}

void process_STATE_DEBUG(){

  int strength_int;
  
       if(input_selectGotPressed() || input_getSecondsSinceLastEvent()>30) {
            output_sequence_escape();
            enter_STATE_IDLE();
            return;  
       }
       if(input_snoozeGotPressed()) {
        if(radio_isPlaying()) radio_switchOff();
        else radio_switchOn();
       }
       switch(input_getEncoderValue()) {
        case 0:           /* time hour */
                if(clock_getCurrentTime()>=0) 
                {
                  output_renderDebugDigitScene(clock_getCurrentTime()/60,1);                
                } else {
                  output_renderDebugLetterScene(2,1);                
                }
                
                break;
        case 1:           /* time minute */
                if(clock_getCurrentTime()>=0) 
                {
                  output_renderDebugDigitScene(clock_getCurrentTime()%60,2);
                } else {
                  output_renderDebugLetterScene(2,2);                
                }
                break;   
        case 2:           /* time trust (complete) */
                output_renderTimeTrustScene();
                break;     
        case 3:           /* signal strenght */
                strength_int=radio_getSignalStrength();
                output_renderDebugDigitScene((strength_int*100)>>8,4);
                break;   
        case 4:  /* upper frequency digits (wihtout 100 part)*/
                output_renderDebugDigitScene((radio_getFrequency()/100)%100,5);
                break;

        case 5:  /* lower  frequency */
                output_renderDebugDigitScene(radio_getFrequency()%100,6);
                break;
        case 6:  /* Runtime in millis */
                output_renderUint32Time(millis(),7);
                break;
                
        default: 
             output_sequence_alert();
             break;
       }
        
}

/* *************** STATE_DEMO ***************** */
void enter_STATE_DEMO(){
  clock_state=STATE_DEMO; 
  input_setEncoderRange(0, MINUTES_PER_DAY-1,5,0);
  #ifdef TRACE_CLOCK
         Serial.println(F("#DEMO"));
  #endif
}

void process_STATE_DEMO(){
  static unsigned long demo_prev_frame_time=0;
  
          if(input_snoozeGotPressed()||input_selectGotPressed() || input_getSecondsSinceLastEvent()>180) {
            output_sequence_escape();
            enter_STATE_IDLE();
            return;  
          }

        if(millis()-demo_prev_frame_time> demo_delaytime ) {
            demo_prev_frame_time=millis();
            if(input_getSecondsSinceLastEvent()>5) input_setEncoderValue(input_getEncoderValue()+5);
         }  
         output_renderClockScene(input_getEncoderValue(),ALARM_INDICATOR_OFF); 
}



