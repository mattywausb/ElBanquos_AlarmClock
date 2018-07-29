#define TRACE_ON 1

/* Parameters for Display preferences */
#define MINUTE_FULL_CIRCLE_THRESHOLD 55  // Minute that will show full circle
#define MINUTE_SHIFT_THRESHOLD 45        // Minute that will shift hour gap by 1 pixel




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
#define ALARM_INDICATOR_EDIT B00000111
#define ALARM_INDICATOR_ON   B00000000
#define ALARM_INDICATOR_SHOW B00000011

/* Dont use the following outside Output, just placed here for convinience*/
#define ALARM_IDC_REMOVE_FROM_HOUR_MASK  B00000001
#define ALARM_IDC_SHOW_ON_RING3_MASK     B00000010
#define ALARM_IDC_BLINK_MASK             B00000110



