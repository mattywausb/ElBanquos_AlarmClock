#include "mainSettings.h"

#ifdef TRACE_ON
//#define TRACE_OUTPUT_FINAL 1
//#define TRACE_OUTPUT_BITSET 1
#endif

/* Parameters for Device Connection */

#define LED8X8_DATAIN_PIN 12 
#define LED8X8_CLK_PIN 11 
#define LED8X8_LOAD_PIN 10 
#define LED8X8_COUNT 1
#define LED8X8_PANEL_0 0

/* Output variables and constants */

                             /*84218421 84218421 84218421 84218421 */   
                             /*00001100 11011001 10110011 01100110 */ 
                             /*0   C    D   9    B   3    6   6 */
#define TIME_UNKNOWN_PATTERN 0x0cd9b366;

unsigned long clock_hour_bitmap=TIME_UNKNOWN_PATTERN;
unsigned int clock_minute_bitmap=0xFFFF;
byte flag_indicator_bitmap=0x00;
#define ALARM_INDICATOR_1_MASK B10000001
#define ALARM_INDICATOR_2_MASK B00100000
#define ALARM_INDICATOR_3_MASK B01000010
#define ALARM_INDICATOR_4_MASK B00000100
#define AFTERNOON_INDICATOR_MASK B00011000


#define ALARM_INDICATOR_2_4_MASK B00100100


LedControl led8x8=LedControl(LED8X8_DATAIN_PIN,LED8X8_CLK_PIN,LED8X8_LOAD_PIN,LED8X8_COUNT);


/* ******************************************************************** 
 *     Interface 
 * *******************************************************************
 */
void output_matrix_displayUpdate() {
  byte rowPattern;
  #ifdef TRACE_OUTPUT_FINAL 
        Serial.println("-------");     
  #endif 
  for(int row=0;row<8;row++) {

    /* Add elemens of hour and minute bitmap to row pattern */
    /* map=0  H7  H6  H5  H4  H3  H2  H1  H0 
     *     1  H27     A2          A4      H8
     *     2  H26 A3  M3  M2  M1  M0  A3  H9
     *     3  H25     M11         M4      H10
     *     4  H24     M10         M5      H11
     *     5  H23 A1  M9  M8  M7  M6  A1  H12
     *     6  H22     A2          A4      H13
     *     7  H21 H20 H19 H18 H17 H16 H15 H14
     *     
     *     Minutt bits    = cba9876543210
     *     Minute bitmap  = 0110110110110
     *     Row target bit = _555432225432
     *     Row              _345555432222 
     *     row                   76543210        
     *     
    */
    switch(row) {
      case 0: rowPattern=mirroredPattern(clock_hour_bitmap); break;
      case 1: rowPattern=((clock_hour_bitmap>>20)&B10000000)|((clock_hour_bitmap>>8)&B00000001)
                          | (flag_indicator_bitmap&ALARM_INDICATOR_2_4_MASK); break;
      case 2: rowPattern=((clock_hour_bitmap>>19)&B10000000)|((clock_hour_bitmap>>9)&B00000001)
                          | mirroredPattern((clock_minute_bitmap<<2)&B00111100)
                          | (flag_indicator_bitmap&ALARM_INDICATOR_3_MASK);; break;
      case 3: rowPattern=((clock_hour_bitmap>>18) & B10000000)|((clock_hour_bitmap>>10)&B00000001)
                          | ((clock_minute_bitmap>>2 & B00000100))
                          | ((clock_minute_bitmap>>6 & B00100000))
                          | (flag_indicator_bitmap&AFTERNOON_INDICATOR_MASK); break;
      case 4: rowPattern=((clock_hour_bitmap>>17)&B10000000)|((clock_hour_bitmap>>11)&B00000001)
                          | ((clock_minute_bitmap>>3 & B00000100))
                          | ((clock_minute_bitmap>>5 & B00100000))
                          | (flag_indicator_bitmap&AFTERNOON_INDICATOR_MASK); break;
      case 5: rowPattern=((clock_hour_bitmap>>16)&B10000000)|((clock_hour_bitmap>>12)&B00000001)
                          | ((clock_minute_bitmap>>4)&B00111100)
                          | ((flag_indicator_bitmap&ALARM_INDICATOR_1_MASK)<<1)
                          | ((flag_indicator_bitmap&ALARM_INDICATOR_1_MASK)>>1); break;
      case 6: rowPattern=((clock_hour_bitmap>>15)&B10000000)|((clock_hour_bitmap>>13)&B00000001)
                          | (flag_indicator_bitmap&ALARM_INDICATOR_2_4_MASK); break;
      case 7: rowPattern=clock_hour_bitmap>>14;break;
      } // switch
   
    led8x8.setRow(LED8X8_PANEL_0,row,rowPattern);   
    #ifdef TRACE_OUTPUT_FINAL 
      Serial.println(rowPattern,BIN);
    #endif
  }
  #ifdef TRACE_OUTPUT_FINAL 
    Serial.println("-------");       
  #endif 
}

void output_renderClockBitmaps(int minute_of_the_day,byte alarmIndicator) {

  unsigned int minute_of_the_hour = (minute_of_the_day%60);
  unsigned int hour = (minute_of_the_day/60)%12; /* will be truncated by integer arithmetic */
  unsigned int switchOffBar=0;
  unsigned long master_pattern=0;
  unsigned long simple_pattern=3;
  const unsigned long switchOffPattern=B01111110;

  #ifdef TRACE_OUTPUT_BITSET 
        Serial.print(hour);Serial.print(":");Serial.println(minute_of_the_hour);
  #endif 

  if(minute_of_the_day<0) {
    clock_hour_bitmap=TIME_UNKNOWN_PATTERN;
                        /* 000000010 01001001 */
    clock_minute_bitmap=0x0249;
    flag_indicator_bitmap=0x00;
    return;
  }
  /* ********** *
   *   Hour
   * **********
   */
  simple_pattern<<=(2+hour*2); /* this will place hour 0 at bit 2 and 3, 1 at 4+5 and so on */
                                /* FEDCBA98 76543210 FEDCBA98 76543210 */
                                /* 00000000 00000000 00000000 00001100  = 0 Hrs */
                                /* 00000000 00000000 01100000 00000000  = 6 Hrs */
                                /* 00000011 00000000 00000000 00000000  = 11 Hrs */
                                /* ____aaaa ddddddcc ccccbbbb bbaaaa__  = simple pattern sides abcd*/

  if(minute_of_the_hour >= MINUTE_SHIFT_THRESHOLD) 
            simple_pattern<<=1; /* Shift the hole pattern to indicate upcoming hour */

  

  /* Add corner bits (+) to the pattern and overlay 11th to 1st hour */
  /*  ____aaaa ddddddcc ccccbbbb bbaaaaa_  => */
  /*  ____dddd dd+ccccc c+bbbbbb +aaaaaa+     */
  /*  84218421 84218421 84218421 84218421     */
  master_pattern=   ((simple_pattern & 0x0000003c)<<1)   /* -__aaaa- from 0 hr to 1 */
                  | ((simple_pattern & 0x0f000000)>>23)  /* -aaaa__- from 11 hr to 0 */       
                  | ((simple_pattern & 0x00000fc0)<<2)   /* -bbbbbb- from 2 hr to 4 */       
                  | ((simple_pattern & 0x0003f000)<<3)   /* -cccccc- from 5 hr to 7 */       
                  | ((simple_pattern & 0x00fc0000)<<4)   /* -dddddd- from 5 hr to 7 */       
                  | (((simple_pattern>>23)&(simple_pattern>>24)& 0x00000001)) /* left upper corner */       
                  | (((simple_pattern<<2) &(simple_pattern<<1)&  0x00000080))   /* Right upper corner */       
                  | (((simple_pattern<<3) &(simple_pattern<<2)&  0x00004000))   /* Right lower corner */       
                  | (((simple_pattern<<4) &(simple_pattern<<3)&  0x00200000)) ;  /* left lower corner */       


  switchOffBar=(((hour+1)/3)+2)%4;   /* Determine Side to set big bar */ 
  master_pattern |= switchOffPattern<<(switchOffBar*7);  

  /* set alarm off bits in houre pattern */

  /*  8421842<8421842<8421842<8421842<
  /*  ___-dddddd+cccccc+bbbbbb+aaaaaa+     */
  /*0 _____4________________2_________     */
  /*1 _______________1__________2_____     */
  /*2 ________8__________1____________     */
  /*3 ____________8________________4__     */

  if(alarmIndicator&ALARM_IDC_REMOVE_FROM_HOUR_MASK) {
    switch(switchOffBar) {
      case 0:  master_pattern |= 0x04000200; break;
      case 1:  master_pattern |= 0x00010020; break;
      case 2:  master_pattern |= 0x00801000; break;
      case 3:  master_pattern |= 0x00080004; break;
    }
  }

  /* write hour bitmap to final memory (Inverted to light up the zeroes) */
  clock_hour_bitmap=~master_pattern;

  #ifdef TRACE_OUTPUT_BITSET 
        Serial.println(0x80000000|simple_pattern,BIN);
        Serial.println(0x80000000|master_pattern,BIN);
  #endif  
   
  /* set alarm show bits in ring 3 */
  if(alarmIndicator&ALARM_IDC_SHOW_ON_RING3_MASK && ((
    alarmIndicator&ALARM_IDC_BLINK_MASK)==0|| millis()%1000>500) ) {
    switch(switchOffBar) {
      case 0: flag_indicator_bitmap=ALARM_INDICATOR_3_MASK;break;
      case 1: flag_indicator_bitmap=ALARM_INDICATOR_4_MASK;break;
      case 2: flag_indicator_bitmap=ALARM_INDICATOR_1_MASK;break;
      case 3: flag_indicator_bitmap=ALARM_INDICATOR_2_MASK;break;
    }
  } else flag_indicator_bitmap=0x00;

  /* set afternoon flag when showing alarms and time is 12:00 or above */
  if(alarmIndicator&ALARM_IDC_SHOW_ON_RING3_MASK && minute_of_the_day>=720)   flag_indicator_bitmap|=AFTERNOON_INDICATOR_MASK;
  
  #ifdef TRACE_OUTPUT_BITSET 
        Serial.println(0x80|flag_indicator_bitmap,BIN);
  #endif 

   /* *************
    *    Minute
    * ************  */

                     
  simple_pattern=0x0db6;                                /* _24_ */                                                       /* 8  1 */                                                       /* 4  2 */
                                                        /* _18_ */
  if(minute_of_the_hour <MINUTE_FULL_CIRCLE_THRESHOLD) simple_pattern>>=(4-(minute_of_the_hour)/15)*3;
  /* move 12 o clock line to bit 0-3 */
  master_pattern=(simple_pattern<<3)|((simple_pattern>>9)&B00001111);
  
  clock_minute_bitmap=master_pattern;

  #ifdef TRACE_OUTPUT_BITSET 
        Serial.print("\t");
        Serial.println(0x8000|master_pattern,BIN);
  #endif
  
};


/* *********************************************************************
/*                    internal helpers                                  */
byte mirroredPattern (byte pattern)
{
  byte newPattern=0;
    newPattern|=pattern&B10000000; /* Copy highest bit */

  for(byte i=1;i<8;i++) {
    pattern<<=1;
    newPattern>>=1;
    newPattern|=pattern&B10000000; 
  }
  return newPattern;
}



void output_setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  led8x8.shutdown(LED8X8_PANEL_0,false);
  /* Set the brightness to a medium values */
  led8x8.setIntensity(LED8X8_PANEL_0,1);
  /* and clear the display */
  led8x8.clearDisplay(LED8X8_PANEL_0);
}
