//We always have to include the library
#include "LedControl.h"


#include "mainSettings.h"

#ifdef TRACE_ON
#define TRACE_CLOCK 1
#endif

#define REFERENCE_TIME_AGE_FOR_CHECK 30
#define REFERENCE_TIME_AGE_FOR_NOTIFY 90
#define REFERENCE_TIME_AGE_FOR_INVALID 144000

const unsigned long demo_delaytime=400;
unsigned long demo_prev_frame_time=0;


/* Some general knowledge about time */
#define MINUTES_PER_DAY 1440
#define MILLIES_PER_DAY 86400000
#define MILLIES_PER_MINUTE 60000

/* Clock variables */

enum CLOCK_STATE {
  STATE_IDLE, // show time, check for alarms, check age of rds time 
  STATE_DEMO  // show fast time progressing
};

CLOCK_STATE clock_state = STATE_IDLE; ///< The state variable is used for parsing input characters.
#define TIME_UNKNOWN -1

unsigned long clock_sync_time=millis();
int clock_reference_time=-1;






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
        Serial.print("Referenc time set to ");
        Serial.print(clock_getCurrentTime()/60);
        Serial.print(":");
        Serial.println(clock_getCurrentTime()%60);
      #endif
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
  input_setup(0, 24*4); /* Encoder Range is quater hours */
  radio_setup();
}


/* ************************************************************
 *     Main Loop
 * ************************************************************
 */

void loop() { 
  static int demo_minutes_of_the_day=0;

  /* Inputs */
  input_switches_scan_tick();
  radio_loop_tick();

  /* logic*/
  switch (clock_state) {
    
    case STATE_IDLE:  // show time, check for alarms, check age of rds time
          if(input_snoozeGotPressed()) { 
            clock_state=STATE_DEMO; 
            output_renderClockBitmaps(demo_minutes_of_the_day);   
            break;
          }

          /* Check RDS Data when necessary */
          if(clock_getReferenceAge()>REFERENCE_TIME_AGE_FOR_CHECK) {
            radio_setRdsScanActive(true);
            if(radio_getRdsIsUptodate()) { /* we got an update */
               clock_setReferenceTime(radio_getLastRdsTimeInfo());
               radio_setRdsScanActive(false);       
            }
          }
          
          output_renderClockBitmaps(clock_getCurrentTime()); 
          break;
    /* ------------------------------------------------------------------ */  
          
    case STATE_DEMO:
         if(input_snoozeGotPressed()) {
            clock_state=STATE_IDLE; 
            output_renderClockBitmaps(clock_getCurrentTime()); 
            break;  
          }
          
        if(millis()-demo_prev_frame_time> demo_delaytime) {
            demo_prev_frame_time=millis();
       
            demo_minutes_of_the_day+=5; 
            if(demo_minutes_of_the_day >= 12*60) {demo_minutes_of_the_day=0;};
            // if(demo_minutes_of_the_day > 3*60 &&demo_minutes_of_the_day < 10*60) minutes_of_the_day=10*60;
            output_renderClockBitmaps(demo_minutes_of_the_day);  
         }  
        break; 
    /* ------------------------------------------------------------------ */  
  }

  /* output */
  output_matrix_displayUpdate() ;

}
