//We always have to include the library
#include "LedControl.h"


#define TRACE_ON 1

#ifdef TRACE_ON
#define OUTPUT_TRACE 1
#define TRACE_CLOCK 1
#endif

#define LED8X8_DATAIN_PIN 12 
#define LED8X8_CLK_PIN 11 
#define LED8X8_LOAD_PIN 10 
#define LED8X8_COUNT 1
#define LED8X8_PANEL_0 0

#define MINUTE_SHIFT_THRESHOLD 45

#define FRAME_DELAY 750


enum MAIN_STATE {
  STATE_DEMO_CLOCK, 
  STATE_DEMO_RDS_TRUST_BAR,
  STATE_DEMO_ROTARY_TEST
};

MAIN_STATE main_state = STATE_DEMO_ROTARY_TEST; 


LedControl led8x8=LedControl(LED8X8_DATAIN_PIN,LED8X8_CLK_PIN,LED8X8_LOAD_PIN,LED8X8_COUNT);

/* we always wait a bit between updates of the display */
unsigned long delaytime=250;

#define MINUTE_FULL_CIRCLE_THRESHOLD 55
unsigned long clock_hour_bitmap=0xFFFFFFFF;
unsigned int clock_minute_bitmap=0xFFFF;



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

void displayUpdate() {
  byte rowPattern;
  #ifdef OUTPUT_TRACE 
        Serial.println("-------");     
  #endif 
  for(int row=0;row<8;row++) {

    /* Add elemens of hour and minute bitmap to row pattern */
    /* map=0  H7  H6  H5  H4  H3  H2  H1  H0 
     *     1  H27                         H8
     *     2  H26     M3  M2  M1  M0      H9
     *     3  H25     M11         M4      H10
     *     4  H24     M10         M5      H11
     *     5  H23     M9  M8  M7  M6      H12
     *     6  H22                         H13
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
      case 1: rowPattern=((clock_hour_bitmap>>20)&B10000000)|((clock_hour_bitmap>>8)&B00000001); break;
      case 2: rowPattern=((clock_hour_bitmap>>19)&B10000000)|((clock_hour_bitmap>>9)&B00000001)
                          | mirroredPattern((clock_minute_bitmap<<2)&B00111100); break;
      case 3: rowPattern=((clock_hour_bitmap>>18) & B10000000)|((clock_hour_bitmap>>10)&B00000001)
                          | ((clock_minute_bitmap>>2 & B00000100))
                          | ((clock_minute_bitmap>>6 & B00100000)); break;
      case 4: rowPattern=((clock_hour_bitmap>>17)&B10000000)|((clock_hour_bitmap>>11)&B00000001)
                          | ((clock_minute_bitmap>>3 & B00000100))
                          | ((clock_minute_bitmap>>5 & B00100000)); break;
      case 5: rowPattern=((clock_hour_bitmap>>16)&B10000000)|((clock_hour_bitmap>>12)&B00000001)
                          | ((clock_minute_bitmap>>4)&B00111100); break;
      case 6: rowPattern=((clock_hour_bitmap>>15)&B10000000)|((clock_hour_bitmap>>13)&B00000001); break;
      case 7: rowPattern=clock_hour_bitmap>>14;break;
      } // switch
   
    led8x8.setColumn(LED8X8_PANEL_0,7-row,rowPattern);   
    #ifdef OUTPUT_TRACE 
      Serial.println(rowPattern,BIN);
    #endif
  }
  #ifdef OUTPUT_TRACE 
    Serial.println("-------");       
  #endif 
}

void renderClockBitmaps(int minute_of_the_day) {

  unsigned int minute_of_the_hour = (minute_of_the_day%60);
  unsigned int hour = (minute_of_the_day/60)%12; /* will be truncated by integer arithmetic */
  unsigned int switchOffBar=0;
  unsigned long master_pattern=0;
  unsigned long simple_pattern=3;
  const unsigned long switchOffPattern=B01111110;

  #ifdef TRACE_CLOCK 
        Serial.print(hour);Serial.print(":");Serial.println(minute_of_the_hour);
  #endif 

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

  /* write hour bitmap to final memory (Inverted to light up the zeroes) */
  clock_hour_bitmap=~master_pattern;

 
 #ifdef TRACE_CLOCK 
        Serial.println(0x80000000|simple_pattern,BIN);
        Serial.print(0x80000000|master_pattern,BIN);
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

  #ifdef TRACE_CLOCK 
        Serial.print("\t");
        Serial.println(0x8000|master_pattern,BIN);
  #endif
  
};


void displayTrust(byte rds_trust_base16){
    #ifdef TRACE_ON
      Serial.println( rds_trust_base16);
    #endif
    byte rowPattern=0;
    
    if(rds_trust_base16>15)rds_trust_base16=15;

    for(byte row=0;row<8;row++) {
     
      switch(row) {
        case 0:
        case 7: rowPattern=B11000011;break;
        case 1:
        case 6: rowPattern=B10000001;break;
        default:  rowPattern=0; break;
      }      
      if((7-row)*2<rds_trust_base16) rowPattern|=B0001000;
      if((row)*2<rds_trust_base16+1) rowPattern|=B00010000;
      led8x8.setColumn(LED8X8_PANEL_0,7-row,rowPattern); 
    }
}
;


void setup() {
 #ifdef TRACE_ON 
 Serial.begin(9600);
 Serial.println("--------->Start<------------");
 #endif
  
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  led8x8.shutdown(LED8X8_PANEL_0,false);
  /* Set the brightness to a medium values */
  led8x8.setIntensity(LED8X8_PANEL_0,1);
  /* and clear the display */
  led8x8.clearDisplay(LED8X8_PANEL_0);

  input_setup(0, 8); /* Leds */
}

void loop() { 
  static unsigned long last_frame_change_time=0;
  static int presscount;
  static int minutes_of_the_day=0;
  byte pattern;
  byte pattern2;

              static byte trust=0;

              
    #ifdef TRACE_ON
      //Serial.println( "--Tick--");
    #endif
  /* Inputs */
  input_switches_scan_tick();

  switch(main_state) {
    case STATE_DEMO_CLOCK:

            if(input_selectGotPressed()) {
              main_state=STATE_DEMO_RDS_TRUST_BAR;
              break;
            }
            if(millis()-last_frame_change_time>FRAME_DELAY) {
              last_frame_change_time=millis();
              
              minutes_of_the_day+=5; 
              if(minutes_of_the_day >= 12*60) {minutes_of_the_day=0;};
              // if(minutes_of_the_day > 3*60 && minutes_of_the_day < 10*60) minutes_of_the_day=10*60;
              renderClockBitmaps(minutes_of_the_day);
              
              /* output */
              displayUpdate() ;
            }
 
            break;  
    case STATE_DEMO_RDS_TRUST_BAR:
            if(input_selectGotPressed()) {
              main_state=STATE_DEMO_ROTARY_TEST;
              led8x8.clearDisplay(LED8X8_PANEL_0);              
              break;
            }

            if(millis()-last_frame_change_time>FRAME_DELAY) {
              last_frame_change_time=millis();
              displayTrust(trust);
              if(++trust>15){
                trust=0;
              }
            }
            break;
     case STATE_DEMO_ROTARY_TEST:
           if(input_selectGotPressed()) {
            main_state=STATE_DEMO_CLOCK;
            break;
           }

           if(input_snoozeGotPressed()){
              if(++presscount>7) presscount=-7; 
           }
            if(presscount<0) pattern2=0xff<<abs(presscount);
            else  pattern2=0xff>>presscount;
            pattern=0xff>>input_getEncoderValue();

            led8x8.setColumn(LED8X8_PANEL_0,7,input_getRawRegisterHigh(0));
            led8x8.setColumn(LED8X8_PANEL_0,6,input_getRawRegisterLow(0));
 
            led8x8.setColumn(LED8X8_PANEL_0,5,input_getRawState());
            led8x8.setColumn(LED8X8_PANEL_0,4,input_getDebouncedState());
            led8x8.setColumn(LED8X8_PANEL_0,3,input_getTickState());
            led8x8.setColumn(LED8X8_PANEL_0,1,pattern2);
            led8x8.setColumn(LED8X8_PANEL_0,0,pattern);
            break;
     default:
                         
    #ifdef TRACE_ON
      Serial.println( "### BAD MAIN STATE ###");
    #endif
  }
  /* logic*/
  
}
