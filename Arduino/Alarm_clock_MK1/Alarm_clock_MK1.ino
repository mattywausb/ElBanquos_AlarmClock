//We always have to include the library


#include "mainSettings.h"

#ifdef TRACE_ON
#define TRACE_CLOCK 1
#endif

#define REFERENCE_TIME_AGE_FOR_CHECK 30
#define REFERENCE_TIME_AGE_FOR_NOTIFY 90
#define REFERENCE_TIME_AGE_FOR_INVALID 144000

const unsigned long demo_delaytime=400;



/* Some general knowledge about time */
#define MINUTES_PER_DAY 1440
#define MILLIES_PER_DAY 86400000
#define MILLIES_PER_MINUTE 60000

/* Clock variables */

enum CLOCK_STATE {
  STATE_IDLE, // show time, check for alarms, check age of rds time 
  STATE_ALARM_INFO,  // Show Alarm time
  STATE_ALARM_CHANGE,  // Change and set Alarm time
  STATE_SLEEP_SET,  // Activate Sleep and change sleep time
  STATE_DEMO  // show fast time progressing
};

CLOCK_STATE clock_state = STATE_IDLE; ///< The state variable is used for parsing input characters.
#define TIME_UNKNOWN -1
#define SLEEP_OFF -2

unsigned long clock_sync_time=millis();
int clock_reference_time=-1;
int clock_alarm_time[4]={0,0,0,0};
byte clock_focussed_alarmIndex=0; 
int clock_sleep_stop_time=SLEEP_OFF;



/* ************************************************************
 *     Clock functions
 * ************************************************************
 */

int clock_getCurrentTime() {
  if(clock_reference_time==TIME_UNKNOWN) return TIME_UNKNOWN;
  return (((millis()-clock_sync_time)/60000)+clock_reference_time)%MINUTES_PER_DAY;
}

unsigned long clock_getSyncTime(){
  return clock_sync_time; 
}

int clock_getReferenceAge(){
  if (clock_sync_time==0) {  /* Seems we dont have any data */
    if(millis()<REFERENCE_TIME_AGE_FOR_NOTIFY) return REFERENCE_TIME_AGE_FOR_CHECK; /* Warn gracefully */
    return millis();  /* We get worried */
  }
  return (millis()-clock_sync_time)/MILLIES_PER_MINUTE;
}

void clock_setReferenceTime(int measured_time) {
  clock_sync_time=millis();
  clock_reference_time=measured_time;
        #ifdef TRACE_CLOCK
        Serial.print("Reference time set to ");
        Serial.print(clock_getCurrentTime()/60);
        Serial.print(":");
        Serial.println(clock_getCurrentTime()%60);
      #endif
}
/* *************** STATE_IDLE ***************** */

void enter_STATE_IDLE(){
 clock_state=STATE_IDLE;
 output_renderClockBitmaps(clock_getCurrentTime(),ALARM_INDICATOR_OFF);   
         #ifdef TRACE_CLOCK
         Serial.println("Entered STATE_IDLE");
         #endif
}

void process_STATE_IDLE(){
 if(input_snoozeGotPressed()) {     
  if(clock_getCurrentTime()==TIME_UNKNOWN) return;  /* Dont start sleep when time is not settled */ 
  /* TBD:dont start sleep when alarm is running */
  enter_STATE_SLEEP_SET();
  }
  
  if(input_checkEncoderChangeEvent()) {enter_STATE_ALARM_INFO();return;}
  if(input_selectGotPressed()) {enter_STATE_ALARM_CHANGE();return;}
 
  /* Check RDS Data when necessary */
  if(clock_getReferenceAge()>REFERENCE_TIME_AGE_FOR_CHECK) {
    radio_setRdsScanActive(true);
    if(radio_getRdsIsUptodate()) { /* we got an update */
       clock_setReferenceTime(radio_getLastRdsTimeInfo());
       radio_setRdsScanActive(false);       
    }
  }  
  output_renderClockBitmaps(clock_getCurrentTime(),ALARM_INDICATOR_OFF);
}

/* *************** STATE_ALARM_INFO ***************** */

void enter_STATE_ALARM_INFO(){
  
  clock_state=STATE_ALARM_INFO; 
  input_getEncoderValue(); // Read out Encoder to get of the last event 
  output_renderClockBitmaps( clock_alarm_time[clock_focussed_alarmIndex],ALARM_INDICATOR_SHOW);
           #ifdef TRACE_CLOCK
         Serial.println("Entered STATE_ALARM_INFO");
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
   output_renderClockBitmaps( clock_alarm_time[clock_focussed_alarmIndex],ALARM_INDICATOR_SHOW);
  
}

/* *************** STATE_ALARM_CHANGE ***************** */

void enter_STATE_ALARM_CHANGE(){
  clock_state=STATE_ALARM_CHANGE; 
  input_setEncoderRange(0, MINUTES_PER_DAY-1,5,clock_alarm_time[clock_focussed_alarmIndex]);
  output_renderClockBitmaps(input_getEncoderValue(),ALARM_INDICATOR_EDIT);
           #ifdef TRACE_CLOCK
         Serial.println("Entered STATE_ALARM_CHANGE");
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

      output_renderClockBitmaps(input_getEncoderValue(),ALARM_INDICATOR_EDIT); 
}


/* *************** STATE_SLEEP_SET ***************** */

void enter_STATE_SLEEP_SET(){
  clock_state=STATE_SLEEP_SET; 
  int sleepMinutes=45;
  if(clock_sleep_stop_time!=SLEEP_OFF) sleepMinutes=clock_sleep_stop_time-clock_getCurrentTime();
  if(sleepMinutes<0) sleepMinutes+MINUTES_PER_DAY;
  input_setEncoderRange(0, 70,5,sleepMinutes);
  output_renderSleepBitmaps(input_getEncoderValue());
  radio_switchOn();
  #ifdef TRACE_CLOCK
         Serial.println("Entered STATE_SLEEP_SET");
  #endif
}

void process_STATE_SLEEP_SET(){
  if(input_snoozeGotPressed() )
  {
    #ifdef TRACE_CLOCK
         Serial.println("SLEEP canceled by snooze");
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
         Serial.println("SLEEP canceled with 0 value");
      #endif
       radio_switchOff(); /* TBD: only if we are still in a wakeup interval */
       clock_sleep_stop_time=SLEEP_OFF;
       enter_STATE_IDLE();
      return;
    }
    clock_sleep_stop_time=(input_getEncoderValue()+clock_getCurrentTime()) % MINUTES_PER_DAY;
    output_sequence_acknowlegde();
    #ifdef TRACE_CLOCK
         Serial.print("SLEEP active until ");
         Serial.print(clock_sleep_stop_time/60);
         Serial.print(":");
         Serial.println(clock_sleep_stop_time%60);         
    #endif
    enter_STATE_IDLE();
    return;       
  }
  output_renderSleepBitmaps(input_getEncoderValue());
}

/* *************** STATE_DEMO ***************** */
void enter_STATE_DEMO(){
  clock_state=STATE_DEMO; 
  #ifdef TRACE_CLOCK
         Serial.println("Entered STATE_DEMO");
  #endif
}

void process_STATE_DEMO(){
  static int demo_minutes_of_the_day=0;
  static unsigned long demo_prev_frame_time=0;
  
          if(input_snoozeGotPressed()) {
            enter_STATE_IDLE();
            return;  
          }
          
        if(millis()-demo_prev_frame_time> demo_delaytime) {
            demo_prev_frame_time=millis();
       
            demo_minutes_of_the_day+=5; 
            if(demo_minutes_of_the_day >= 12*60) {demo_minutes_of_the_day=0;};
            // if(demo_minutes_of_the_day > 3*60 &&demo_minutes_of_the_day < 10*60) minutes_of_the_day=10*60;
            output_renderClockBitmaps(demo_minutes_of_the_day,ALARM_INDICATOR_ON);  
         }   
}


/* ************************************************************
 *     Main Setup
 * ************************************************************
 */

void setup() {
 #ifdef TRACE_ON 
 Serial.begin(9600);
 Serial.println("--------->Start<------------");
 #endif
 
  output_setup();
  input_setup(0, MINUTES_PER_DAY-1,15); /* Encoder Range 24 hoursis,stepping quater hours */
  radio_setup();

  clock_alarm_time[0]=6*60+30; /* Fall back alarm = 6:30 */
}


/* ************************************************************
 *     Main Loop
 * ************************************************************
 */

void loop() { 


  /* Inputs */
  input_switches_scan_tick();
  radio_loop_tick();


  /* Timer logic */
  if(clock_getCurrentTime()==clock_sleep_stop_time) {
    /* TBD: Check if we are still in an alarm situation */
    radio_switchOff();
    clock_sleep_stop_time=SLEEP_OFF;
    #ifdef TRACE_CLOCK
         Serial.println("SLEEP timed out");
    #endif
  }

  /* UI logic */
  switch (clock_state) {
    
    case STATE_IDLE:          process_STATE_IDLE(); break;
          
    case STATE_ALARM_INFO:    process_STATE_ALARM_INFO(); break;
          
    case STATE_ALARM_CHANGE:  process_STATE_ALARM_CHANGE(); break;

    case STATE_SLEEP_SET:  process_STATE_SLEEP_SET(); break;

    case STATE_DEMO:  process_STATE_DEMO(); break;

 /* ------------------------------------------------------------------ */  
  
    default: output_renderClockBitmaps(TIME_UNKNOWN,ALARM_INDICATOR_ON); 
      output_matrix_displayUpdate() ;
      delay(1000);
      enter_STATE_IDLE();
  }

  /* output */
  output_matrix_displayUpdate() ;

}
