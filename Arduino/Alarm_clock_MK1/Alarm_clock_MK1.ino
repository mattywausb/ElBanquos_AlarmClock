//We always have to include the library


#include "mainSettings.h"

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
  STATE_DEMO  // show fast time progressing
};
CLOCK_STATE clock_state = STATE_IDLE; 

#define TIME_UNKNOWN -1
#define TRIGGER_IS_OFF -2

#define RDS_TRUST_GOOD 20         // Highest trust you can get
#define RDS_TRUST_ACCEPTABLE 10   // Minimal trust to be used
#define RDS_TRUST_CANDIDATE 4     // Number of consecutive occurences to be protected from instant replacement
#define RDS_DERIVATION_TOLERANCE 3 // Minutes of derivation, we accept as correction
#define RDS_SCAN_COOLDOWN_TIME  750   // Milliseconds before we use the next RDS Time Telegram
//#define RDS_MAX_WAIT_TIME  300000    // after 5 Minutes (300 seconds) with no signal we change channel 
#define RDS_MAX_WAIT_TIME  150000    // after 2,5 Minutes (150 seconds) with no signal we change channel 
#define BAD_TRY_UNTIL_CHANNEL_CHANGE 14 // Number of unsuccedfull tries until we change RDS channel

unsigned long clock_sync_time=millis();
int clock_reference_time=-1;
int clock_alarm_time[4]={0,0,0,0};
byte clock_focussed_alarmIndex=0; 
int clock_sleep_stop_time=TRIGGER_IS_OFF;
int clock_alarm_stop_time=TRIGGER_IS_OFF;
int clock_snooze_stop_time=TRIGGER_IS_OFF;
byte clock_alarm_stopprocedure_progress=0;
unsigned long clock_last_wakeup_stop_millis=0;  
byte clock_rds_trust=0;


/* ************************************************************
 *     Clock functions
 * ************************************************************
 */

int clock_getCurrentTime() {
  if(clock_rds_trust <RDS_TRUST_ACCEPTABLE) return TIME_UNKNOWN;
  return (((millis()-clock_sync_time)/60000)+clock_reference_time)%MINUTES_PER_DAY;
}

unsigned long clock_getSyncTime(){
  return clock_sync_time; 
}

int clock_getAgeOfReference(){
  if (clock_sync_time==0) {  /* Seems we dont have any data */
    if(millis()<REFERENCE_AGE_FOR_NOTIFY) return REFERENCE_AGE_FOR_CHECK; /* Warn gracefully */
    return millis();  /* We get worried */
  }
  return (millis()-clock_sync_time)/MILLIES_PER_MINUTE;
}

void clock_setReferenceTime(int measured_time) {
static unsigned long last_measure_time=0;
static byte badTryCount=0;

  if(measured_time>MINUTES_PER_DAY) return; /* this is definietly rubbish */

  if( millis()-last_measure_time > RDS_MAX_WAIT_TIME) { /* if we waited very long we switch channel */
     #ifdef TRACE_CLOCK
        Serial.println(F("RDS Timeout"));
     #endif 
     badTryCount=0;
     radio_switchRdsStation();  
     last_measure_time=millis();   
  }
  #ifdef TRACE_CLOCK
        static byte prev_second=0;
        byte cursecond=(millis()-last_measure_time)/10000;
        if(cursecond!=prev_second) {
          prev_second=cursecond;
          Serial.print(F("No RDS Time since:"));
          Serial.println(cursecond*10);
        }
  #endif
  
  if( millis()-last_measure_time < RDS_SCAN_COOLDOWN_TIME     /* Dont accept values too fast  */
   || radio_getRdsTimeAge() >= RDS_SCAN_COOLDOWN_TIME  ) return;       /* Only accept fresh values in relation to cooldown  */

  last_measure_time=millis();
  
  int previous_measure_now=(((last_measure_time-clock_sync_time)/60000)+clock_reference_time)%MINUTES_PER_DAY;
  
   #ifdef TRACE_CLOCK
      Serial.print(F("RDS:"));
      trace_printTime(measured_time);
   #endif
     
  if(abs(measured_time-previous_measure_now)<RDS_DERIVATION_TOLERANCE) { /* RDS delivered in expected range  */
     if(clock_rds_trust<RDS_TRUST_GOOD) clock_rds_trust+=1;
      clock_sync_time=last_measure_time;
      clock_reference_time=measured_time; 
      if(badTryCount>0) badTryCount--;   
      #ifdef TRACE_CLOCK
       Serial.print(F("\taccept"));
      #endif 
 
  } else {

     if(clock_rds_trust>=RDS_TRUST_CANDIDATE) {
      clock_rds_trust-=2; 
      #ifdef TRACE_CLOCK
       Serial.print(F("\treject"));
      #endif   
       
     } else  { /* Only, when our trust has gone , we try the new RDS value */
      badTryCount++;
      clock_sync_time=last_measure_time;
      clock_reference_time=measured_time;        
      clock_rds_trust=0;
      /* TBD switch station after x attemps */
      #ifdef TRACE_CLOCK
       Serial.print(("\tswitch"));
      #endif    
     }
  }

  if(badTryCount>BAD_TRY_UNTIL_CHANNEL_CHANGE) {
    #ifdef TRACE_CLOCK
       Serial.println(F("RDS bad count limit reached > channelchange"));
    #endif 
    badTryCount=0;
    radio_switchRdsStation();
  }
  
  #ifdef TRACE_CLOCK
    Serial.print(F(" trust:"));
    Serial.print(clock_rds_trust);
    Serial.print(F("\tBadTry:"));
    Serial.println(badTryCount);
  #endif

}


void trace_printTime(int timeValue)
{
    Serial.print(timeValue/60);
    Serial.print(F(":"));
    Serial.print(timeValue%60);  
}

/* *************** STATE_IDLE ***************** */

void enter_STATE_IDLE(){
     clock_state=STATE_IDLE;
     output_renderIdleClockScene(clock_getCurrentTime(),input_masterSwitchIsSet()?ALARM_INDICATOR_ON:ALARM_INDICATOR_OFF,(clock_rds_trust*16)/RDS_TRUST_ACCEPTABLE);
     #ifdef TRACE_CLOCK
     Serial.println(F("#IDLE"));
     #endif
}

void process_STATE_IDLE(){
 if(input_snoozeGotPressed()) {     
    enter_STATE_SLEEP_SET();
  }
  
  if(input_checkEncoderChangeEvent()) {enter_STATE_ALARM_INFO();return;}
  if(input_selectGotPressed()) {enter_STATE_ALARM_CHANGE();return;}
 
  /* Check RDS Data when necessary */
  if(clock_getAgeOfReference()>REFERENCE_AGE_FOR_CHECK ||
     clock_rds_trust<RDS_TRUST_GOOD) {
     radio_setRdsScanActive(true);
     clock_setReferenceTime(radio_getLastRdsTimeInfo());
  }  else radio_setRdsScanActive(false);       

  output_renderIdleClockScene(clock_getCurrentTime()
                             ,input_masterSwitchIsSet()?ALARM_INDICATOR_ON:ALARM_INDICATOR_OFF
                             ,(clock_rds_trust*16)/RDS_TRUST_ACCEPTABLE);  
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
   output_renderIdleClockScene(clock_getCurrentTime()
                               ,clock_snooze_stop_time==TRIGGER_IS_OFF?0:ALARM_INDICATOR_SNOOZE
                               |(input_masterSwitchIsSet()?ALARM_INDICATOR_ON :ALARM_INDICATOR_OFF)
                               ,(clock_rds_trust*16)/RDS_TRUST_ACCEPTABLE);
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
  
  if(input_checkEncoderChangeEvent()) {enter_STATE_STOP_PROCEDURE();return;}

  if( clock_snooze_stop_time!=TRIGGER_IS_OFF) alarmIndicator|=ALARM_INDICATOR_SNOOZE;
  if(!input_masterSwitchIsSet()) alarmIndicator|=ALARM_INDICATOR_OFF;

  output_renderIdleClockScene(clock_getCurrentTime()
                             ,alarmIndicator
                             ,(clock_rds_trust*16)/RDS_TRUST_ACCEPTABLE);

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
    
    if(input_checkEncoderChangeEvent()) input_getEncoderValue(); 
    
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
        enter_STATE_IDLE();return;       
      }

  #ifdef DEBUG_HIGHRES_ALARMSET
    if(input_checkEncoderChangeEvent()) {
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
  input_setEncoderRange(0, 70,5,sleepMinutes);
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

  if(input_selectGotPressed() 
  || input_getSecondsSinceLastEvent()> SECONDS_UNTIL_FALLBACK_SHORT) {
    if(input_getEncoderValue()>60) {
      enter_STATE_DEMO();
      return;
    }
    if(input_getEncoderValue()==0) {
      #ifdef TRACE_CLOCK
         Serial.println(F("SLEEP cancel by 0"));
      #endif
       radio_switchOff(); /* TBD: only if we are still in a wakeup interval */
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


/* ************************************************************
 *     Main Setup
 * ************************************************************
 */

void setup() {
 #ifdef TRACE_ON 
 Serial.begin(9600);
 Serial.println(F("--------->Start<------------"));
 #endif
 
  output_setup();
  input_setup(0, MINUTES_PER_DAY-1,15); /* Encoder Range 24 hoursis,stepping quater hours */
  radio_setup();

  clock_alarm_time[0]=DEFAULT_ALARM_TIME; /* Fall back alarm */
}


/* ************************************************************
 *     Main Loop
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

    case STATE_DEMO:  process_STATE_DEMO(); break;

 /* ------------------------------------------------------------------ */  
  
    default: output_renderClockScene(TIME_UNKNOWN,ALARM_INDICATOR_ON); 
      output_matrix_displayUpdate() ;
      delay(1000);
      enter_STATE_IDLE();
  }

  /* output */
  /* is managed by scenes and sequences call from state functions */
  /* --- no output code here ------*/

  #ifdef TRACE_CLOCK
  static int last_traced_time;
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
