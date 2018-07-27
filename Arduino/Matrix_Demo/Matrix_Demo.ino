//We always have to include the library
#include "LedControl.h"

//#define OUTPUT_TRACE 1

#define LED8X8_DATAIN_PIN 12 
#define LED8X8_CLK_PIN 11 
#define LED8X8_LOAD_PIN 10 
#define LED8X8_COUNT 1
#define LED8X8_PANEL_0 0


LedControl led8x8=LedControl(LED8X8_DATAIN_PIN,LED8X8_CLK_PIN,LED8X8_LOAD_PIN,LED8X8_COUNT);

/* we always wait a bit between updates of the display */
unsigned long delaytime=300;
unsigned long clock_hour_bitmap=0xFFFFFFFC;
byte clock_position=0;

void setup() {
 #ifdef OUTPUT_TRACE 
 Serial.begin(9600);
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
        Serial.println(clock_hour_bitmap,BIN);
         Serial.println("-------");       
      #endif 
}





void loop() { 
  displayUpdate() ;
  clock_hour_bitmap=clock_hour_bitmap<<1|0x01;
  if( clock_hour_bitmap==0xCFFFFFFF) clock_hour_bitmap=0xFFFFFFFC;
  
  delay(delaytime);
  
}
