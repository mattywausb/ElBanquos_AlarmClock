//We always have to include the library
#include "LedControl.h"


#define TRACE_ON 1

#ifdef TRACE_ON
//#define OUTPUT_TRACE 1
#define TRACE_CLOCK 1
#endif

#define LED8X8_DATAIN_PIN 12 
#define LED8X8_CLK_PIN 11 
#define LED8X8_LOAD_PIN 10 
#define LED8X8_COUNT 1
#define LED8X8_PANEL_0 0

#define MINUTE_SHIFT_THRESHOLD 30



LedControl led8x8=LedControl(LED8X8_DATAIN_PIN,LED8X8_CLK_PIN,LED8X8_LOAD_PIN,LED8X8_COUNT);

/* we always wait a bit between updates of the display */
unsigned long delaytime=1000;
unsigned long clock_hour_bitmap=0xFFFFFFFC;
byte clock_position=0;

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

  input_setup(0, 24*4); /* quater hours */
}

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

    /* Add elemens of hour bitmap to row pattern */
    switch(row) {
      case 0: rowPattern=mirroredPattern(clock_hour_bitmap); break;
      case 1: rowPattern=((clock_hour_bitmap>>20)&B10000000)|((clock_hour_bitmap>>8)&B00000001); break;
      case 2: rowPattern=((clock_hour_bitmap>>19)&B10000000)|((clock_hour_bitmap>>9)&B00000001); break;
      case 3: rowPattern=((clock_hour_bitmap>>18)&B10000000)|((clock_hour_bitmap>>10)&B00000001); break;
      case 4: rowPattern=((clock_hour_bitmap>>17)&B10000000)|((clock_hour_bitmap>>11)&B00000001); break;
      case 5: rowPattern=((clock_hour_bitmap>>16)&B10000000)|((clock_hour_bitmap>>12)&B00000001); break;
      case 6: rowPattern=((clock_hour_bitmap>>15)&B10000000)|((clock_hour_bitmap>>13)&B00000001); break;
      case 7: rowPattern=clock_hour_bitmap>>14;break;
      } // switch
   
    led8x8.setRow(LED8X8_PANEL_0,row,rowPattern);   
    #ifdef OUTPUT_TRACE 
      Serial.println(rowPattern,BIN);
    #endif
  }
  #ifdef OUTPUT_TRACE 
    Serial.println("-------");       
  #endif 
}

void renderClockHourBitmap(int minute_of_the_day) {

  unsigned int minute_of_the_hour = (minute_of_the_day%60);
  unsigned int hour = (minute_of_the_day/60)%12; /* will be truncated by integer arithmetic */

  unsigned long master_pattern=0;
  unsigned long simple_pattern=3;

  #ifdef TRACE_CLOCK 
        Serial.print(hour);Serial.print(":");Serial.println(minute_of_the_hour);
  #endif 
  
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

 /* Output in inverse, since our 1 should not be lit up  #TBD move this to output */

 
  clock_hour_bitmap=~master_pattern;
 #ifdef TRACE_CLOCK 
        Serial.println(0x80000000|simple_pattern,BIN);
        Serial.println(0x80000000|master_pattern,BIN);
  #endif    
};



void loop() { 
  static int minutes_of_the_day=0;
  
  input_switches_scan_tick();
  minutes_of_the_day+=30; 
  if(minutes_of_the_day >= 12*60) minutes_of_the_day=0;
 // if(minutes_of_the_day > 3*60 && minutes_of_the_day < 10*60) minutes_of_the_day=10*60;
  renderClockHourBitmap(minutes_of_the_day);
  displayUpdate() ;
  delay(delaytime);  
}
