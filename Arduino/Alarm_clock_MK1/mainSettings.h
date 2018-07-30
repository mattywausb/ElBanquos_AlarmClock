#define TRACE_ON 1

/* Parameters for Display preferences */
#define MINUTE_FULL_CIRCLE_THRESHOLD 4  // Minute until we will show full circle
#define MINUTE_SHIFT_THRESHOLD 45        // Minute that will shift hour gap by 1 pixel

#define ALARM_DURATION  4  // Number of Minutes the Radio will play after last alarm trigger
#define SNOOZE_DURATION 1   // Number of Minutes for the snooze time

#define DEFAULT_ALARM_TIME 16*60+30 

#define SECONDS_UNTIL_FALLBACK_LONG 15   // Number of seconds the clock will wait before falling back 
#define SECONDS_UNTIL_FALLBACK_SHORT 5   // Number of seconds the clock will wait before falling back 


/// The SI4703 board has to be connected by using the following connections:
/// | Arduino UNO pin    | Radio chip signal  | 
/// | -------------------| -------------------| 
/// | 3.3V (red)         | VCC                | 
/// | GND (black)        | GND                | 
/// | A5 or SCL (yellow) | SCLK               | 
/// | A4 or SDA (blue)   | SDIO               | 
/// | D2 (white)         | RST                |


/* Global necessary constants */

#define ALARM_INDICATOR_OFF  B00000001
#define ALARM_INDICATOR_EDIT B00000110
#define ALARM_INDICATOR_ON   B00000000
#define ALARM_INDICATOR_SHOW B00000010
#define ALARM_INDICATOR_SNOOZE  B10000000


/* Dont use the following outside Output, just placed here for convinience*/
#define ALARM_IDC_REMOVE_FROM_HOUR_MASK  B00000001
#define ALARM_IDC_SHOW_MASK              B00000010
#define ALARM_IDC_BLINK_MASK             B00000100



