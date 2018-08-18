#include "mainSettings.h"

#include "LedControl.h"

#ifdef TRACE_ON
//#define TRACE_OUTPUT_FINAL 1
//#define TRACE_OUTPUT_CLOCK_BITSET 1
//#define TRACE_OUTPUT_SLEEP_BITSET 1
//#define TRACE_OUTPUT_RING3_BITSET 1
//#define TRACE_OUTPUT_NUMBER 
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
unsigned long matrix_ring3_bitmap=0;
unsigned int clock_minute_bitmap=0xFFFF;
byte flag_indicator_bitmap=0x00;

#define AFTERNOON_INDICATOR_MASK B00011000


#define ALARM_INDICATOR_2_4_MASK B00100100


LedControl led8x8=LedControl(LED8X8_DATAIN_PIN,LED8X8_CLK_PIN,LED8X8_LOAD_PIN,LED8X8_COUNT);


/* ********************************************************************+
 *            Idle Clock Scene
 * ********************************************************************+
 */

void output_renderTimeTrustScene(){
    output_renderRampupMeasures((radio_getSignalStrength()<<3)/75  // radio signal has a range from 0 to 75,
                                ,clock_getCandidateRDSScore()>>5   // RDS Score has a range from 0 to 255
                                ,clock_getClockRDSScore()>>5);
    
}


/* ################   Scenes   #########################################
 * #####################################################################          
 */

 
/* ***************** Clock Scene *************************************+ 
*/

void output_renderClockScene(int minute_of_the_day,byte alarmIndicator) {

  unsigned int minute_of_the_hour = (minute_of_the_day%60);
  unsigned int hour = (minute_of_the_day/60)%12; /* will be truncated by integer arithmetic */
  unsigned int counterBarPosition=0;
  unsigned long master_pattern=0;
  unsigned long simple_pattern=3;
  const unsigned long fullCounterBarPattern =B01111110;
  const unsigned long alarmCounterBarPattern=B00000000;
  const unsigned long snoozeCounterBarPattern=B00111100;

  #ifdef TRACE_OUTPUT_CLOCK_BITSET 
        Serial.print(hour);Serial.print(":");Serial.println(minute_of_the_hour);
  #endif 

  flag_indicator_bitmap=0;
  matrix_ring3_bitmap=0;


  /* --------  Hour -----------   */
  
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


  /* place counterBar bits according to alarm indicator */
  counterBarPosition=(((hour+1)/3)+2)%4;   /* Determine Side to set big bar */ 

   if(alarmIndicator&ALARM_IDC_SHOW_MASK && ((
    alarmIndicator&ALARM_IDC_BLINK_MASK)==0|| millis()%1000>500) ) 
        master_pattern |= alarmCounterBarPattern<<(counterBarPosition*7);  
   else if((alarmIndicator&ALARM_INDICATOR_SNOOZE) && millis()%2000>1000) 
         master_pattern |= snoozeCounterBarPattern<<(counterBarPosition*7); 
    else master_pattern |= fullCounterBarPattern<<(counterBarPosition*7);  

  /* set alarm off bits in houre pattern */

  /*  8421842<8421842<8421842<8421842<
  /*  ___-dddddd+cccccc+bbbbbb+aaaaaa+     */
  /*0 _____4________________2_________     */
  /*1 _______________1__________2_____     */
  /*2 ________8__________1____________     */
  /*3 ____________8________________4__     */

  if(alarmIndicator&ALARM_IDC_REMOVE_FROM_HOUR_MASK) {
    switch(counterBarPosition) {
      case 0:  master_pattern |= 0x04000200; break;
      case 1:  master_pattern |= 0x00010020; break;
      case 2:  master_pattern |= 0x00801000; break;
      case 3:  master_pattern |= 0x00080004; break;
    }
  }
  /* write hour bitmap to final memory (Inverted to light up the zeroes) */
  clock_hour_bitmap=~master_pattern;

  #ifdef TRACE_OUTPUT_CLOCK_BITSET 
        Serial.println(0x80000000|simple_pattern,BIN);
        Serial.println(0x80000000|master_pattern,BIN);
  #endif  
   



  /* set afternoon flag when showing alarms and time is 12:00 or above */
  if(alarmIndicator&ALARM_IDC_SHOW_MASK && minute_of_the_day>=720)   flag_indicator_bitmap|=AFTERNOON_INDICATOR_MASK;
  
  #ifdef TRACE_OUTPUT_CLOCK_BITSET 
        Serial.print(0x80|flag_indicator_bitmap,BIN);
  #endif 


  /* --------  Minute  -----------   */

  if(alarmIndicator&ALARM_IDC_SHOW_MASK) {
    output_renderMinuteHighresBitmaps(minute_of_the_hour,true) ;                  
  } else { /* create our simple minute display format */
    simple_pattern=0x0db6;                                /* _24_ */
                                                          /* 8  1 */ 
                                                          /* 4  2 */
                                                          /* _18_ */
    if(minute_of_the_hour >MINUTE_FULL_CIRCLE_THRESHOLD) simple_pattern>>=(4-(minute_of_the_hour)/15)*3;

    /* move 12 o clock line to bit 0-3 */
    master_pattern=(simple_pattern<<3)|((simple_pattern>>9)&B00001111);
    
    clock_minute_bitmap=master_pattern;
  
    #ifdef TRACE_OUTPUT_CLOCK_BITSET 
          Serial.print("\t");
          Serial.println(0x8000|master_pattern,BIN);
    #endif
 }

 output_matrix_displayUpdate();
};


/* ***************** Stop Procedure Scene  *************************************+ 
*/

void output_renderStopProcedureScene(int value) {
    clock_hour_bitmap=0;
    flag_indicator_bitmap=AFTERNOON_INDICATOR_MASK;
    clock_minute_bitmap=0;
    output_renderRing3Progress(value);
    output_matrix_displayUpdate();
}


/* ***************** Sleep Procedure Scene  *************************************+ 
*/

void output_renderSleepScene(int minutes) {
    clock_hour_bitmap=0;
    flag_indicator_bitmap=0;
    matrix_ring3_bitmap=0;
    output_renderMinuteHighresBitmaps(minutes,false);
    output_matrix_displayUpdate();
}



/* ***************** render Letter Scene  *************************************+ 
*/

void output_renderLetterScene(byte letter_index) 
{
  output_render_letter(letter_index,2) ;
}

/* ***************** Preset select Procedure Scene  *************************************+ 
*/

void output_renderPresetSelectScene(byte preset_index) 
{
  byte columnBitmap[3];
   
  output_render_letter(4,0); // P

  getDigitColumnPattern(columnBitmap,preset_index+1);
  for(int i=0;i<3;i++)
  {
      led8x8.setRow(LED8X8_PANEL_0,7-i,mirroredPattern((columnBitmap[2-i])));
  }
}


/* ***************** Debug Scene  *************************************+ 
*/


void output_renderDebugDigitScene(int value,byte debug_screen_number)
{
  output_render_number(value,0xff>>(8-debug_screen_number));
}

void output_renderDebugLetterScene(int letter_index, byte debug_screen_number)
{
  output_render_letter(letter_index,1);
  led8x8.setRow(LED8X8_PANEL_0,7,0);
  led8x8.setRow(LED8X8_PANEL_0,0,0);
  led8x8.setRow(LED8X8_PANEL_0,1,0);
  led8x8.setColumn(LED8X8_PANEL_0,7,0xff<<(8-debug_screen_number));
}

void output_renderUint32Time(uint32_t value,byte debug_screen_number)
{
  byte seconds=(value/  1000)%60;
  byte minutes=(value/ 60000)%60;
  byte hours=(value/ 3600000)%24;
  byte days=(value/ 86400000)%100;

  led8x8.setColumn(LED8X8_PANEL_0,7,0xff<<(8-debug_screen_number));
   for(byte col=4;col<7;col++) led8x8.setColumn(LED8X8_PANEL_0,col,0);
  led8x8.setColumn(LED8X8_PANEL_0,3,(days/10)<<4|(days%10));
  led8x8.setColumn(LED8X8_PANEL_0,2,((hours/10)<<4|(hours%10)));  
  led8x8.setColumn(LED8X8_PANEL_0,1,(minutes/10)<<4|(minutes%10));
  led8x8.setColumn(LED8X8_PANEL_0,0,(seconds/10)<<4|(seconds%10)); 
   
}

void output_renderUint32Value(uint32_t value,byte debug_screen_number)
{
  for(byte col=0;col<7;col++) {
    led8x8.setColumn(LED8X8_PANEL_0,col,value>>((col)<<3));
  }
   led8x8.setColumn(LED8X8_PANEL_0,7,0xff<<(8-debug_screen_number));
}

/* ################     sequences (Animations)   ########################
 * #####################################################################          
 */

/* ***************** sequence_acknowlegde  *************************************+ 
*/

  /*     8 4 2 1 8 4 2 1
   * 7                 X    
   * 6               X X 
   * 5               X X   
   * 4             X X     
   * 3     X       X X  
   * 2     X X   X X      
   * 1       X X X      
   * 0         X         
   */

void output_sequence_acknowlegde(){
   const byte colPattern[7]={0x0c,0x06,0x03,0x06,0x1c,0x78,0xe0}; /* twisted */
   led8x8.clearDisplay(LED8X8_PANEL_0); 
   for(byte i=0;i<sizeof(colPattern);i++){
       led8x8.setRow(LED8X8_PANEL_0,i,mirroredPattern(colPattern[i]));     /* using row to turn picture-90 degree */
       delay(30);
   }
   delay(300);
}


/* ***************** sequence_escape  *************************************+ 
*/    

void output_sequence_escape(){
   for(byte i=0;i<8;i++){
       led8x8.setColumn(LED8X8_PANEL_0,7-i,0); /* using column to turn picture-90 degree */    
       delay(20);
   }
   delay(100);  
}

 
/* ***************** sequence_snooze  *************************************+ 
*/

void output_sequence_snooze(){
   output_sequence_acknowlegde();
}

/* ***************** sequence_watchdog_alert  *************************************+ 
*/

  /*     8 4 2 1 8 4 2 1
   * 7   x x x x x x x x    
   * 6   x           x x 
   * 5   x   x x x x x x   
   * 4   x         X X x   
   * 3   x         X X x
   * 2   x   X x x x x x  
   * 1   x           x x
   * 0   x x x x x x x x 
   */
void output_sequence_alert()
{
   const byte colPattern[]={0xff,0x81,0xa5,0xa5,0xa5,0xbd,0xff}; /* twisted */
   led8x8.clearDisplay(LED8X8_PANEL_0); 
   for(byte i=0;i<sizeof(colPattern);i++){
       led8x8.setRow(LED8X8_PANEL_0,i,mirroredPattern(colPattern[i]));     /* using row to turn picture-90 degree */
   }
   delay(1000); 
}

/* *********************************************************************
/*                    internal functions                                  */
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

void output_matrix_displayUpdate() {
  byte rowPattern;
  #ifdef TRACE_OUTPUT_FINAL 
        Serial.println(F("-------"));     
  #endif 
  for(int row=0;row<8;row++) {

    /* Add elemens of hour and minute bitmap to row pattern */
    /* map=0  H0  H1  H2  H3  H4  H5  H6  H7 
     *     1  H27 R0  R1  R2  R3  R4  R5  H8
     *     2  H26 R9  M0  M1  M2  M3  R6  H9
     *     3  H25 R8  M11         M4  R7  H10
     *     4  H24 R7  M10         M5  R8  H11
     *     5  H23 R6  M9  M8  M7  M6  R9  H12
     *     6  H22 R5  R4  R3  R2  R1  RA  H13
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
                          | mirroredPattern(matrix_ring3_bitmap<<1)&B01111110
                          ; break;
      case 2: rowPattern=((clock_hour_bitmap>>19)&B10000000)|((clock_hour_bitmap>>9)&B00000001)
                          | ((matrix_ring3_bitmap>>5) & B00000010)
                          | ((matrix_ring3_bitmap>>13)& B01000000)
                          |  mirroredPattern(clock_minute_bitmap<<2) & B00111100
                          ; break;
      case 3: rowPattern=((clock_hour_bitmap>>18) & B10000000)|((clock_hour_bitmap>>10)&B00000001)
                          | ((matrix_ring3_bitmap>>6) & B00000010)
                          | ((matrix_ring3_bitmap>>12)& B01000000)
                          | ((clock_minute_bitmap>>2) & B00000100)
                          | ((clock_minute_bitmap>>6) & B00100000)
                          | (flag_indicator_bitmap&AFTERNOON_INDICATOR_MASK); break;
      case 4: rowPattern=((clock_hour_bitmap>>17)&B10000000)|((clock_hour_bitmap>>11)&B00000001)
                          | ((matrix_ring3_bitmap>>7) & B00000010)
                          | ((matrix_ring3_bitmap>>11)& B01000000)
                          | ((clock_minute_bitmap>>3 & B00000100))
                          | ((clock_minute_bitmap>>5 & B00100000))
                          | (flag_indicator_bitmap&AFTERNOON_INDICATOR_MASK); break;
      case 5: rowPattern=((clock_hour_bitmap>>16)&B10000000)|((clock_hour_bitmap>>12)&B00000001)
                          | ((matrix_ring3_bitmap>>8) & B00000010)
                          | ((matrix_ring3_bitmap>>10)& B01000000)
                          | ((clock_minute_bitmap>>4)&B00111100)
                          ; break;
      case 6: rowPattern=((clock_hour_bitmap>>15)&B10000000)|((clock_hour_bitmap>>13)&B00000001)
                          | ((matrix_ring3_bitmap>>9)&B01111110);
                           break;
      case 7: rowPattern=clock_hour_bitmap>>14;break;
      } // switch
   
    led8x8.setColumn(LED8X8_PANEL_0,7-row,rowPattern);   /* using column to turn picture-90 degree */
    #ifdef TRACE_OUTPUT_FINAL 
      Serial.println(rowPattern,BIN);
    #endif
  }
  #ifdef TRACE_OUTPUT_FINAL 
    Serial.println(F("-------"));       
  #endif 
}

/* *********    renderMinuteHighresBitmaps *******************************+
 */

void output_renderMinuteHighresBitmaps(int minutes,bool zeroIs60)
{ 

    unsigned int simple_pattern=0x0fff>>(12-(minutes/5));
    /* ____BA9876543210 */
    /* ____dd+cc+bb+aa+ */
    /* ____9876543210BA */
     
    if(minutes>0) clock_minute_bitmap=(simple_pattern<<2)|((simple_pattern>>10)&B00000011);
    else if(zeroIs60) clock_minute_bitmap=0xffff;
                 else clock_minute_bitmap=0x0249; /* 0000002004008001 */
    


    #ifdef TRACE_OUTPUT_SLEEP_BITSET 
    Serial.print(minutes);Serial.print(">");
        Serial.println(0x8000|clock_minute_bitmap,BIN);
    #endif
}

/* *********    Render ring3 progress *******************************+
 */

 void output_renderRing3Progress(int value) {

  if(value==0) matrix_ring3_bitmap=0;
  if(value>0) matrix_ring3_bitmap=0x000fffff>>(20-value);
  if(value<0) matrix_ring3_bitmap=0x000fffff<<(20+value);
    #ifdef TRACE_OUTPUT_RING3_BITSET 

        Serial.print(value);
        Serial.print("\t");
        Serial.println(0x80000000|matrix_ring3_bitmap,BIN);
    #endif
 }

/* ***************** RampupMeasures   *************************************+ 
*/

void output_renderRampupMeasures(int leftMeasure,int midMeasure,byte rightMeasure)
{
   /* render "unkown" screen with trust progress bar */
    byte rowPattern=0;  

    for(byte row=0;row<8;row++) { // col 0 oben 
     
      switch(row) {
        case 0:
        case 7: rowPattern=B10000001;break;
        default:  rowPattern=0; break;
      }      
      if((7-row)<leftMeasure) rowPattern|=      B01000000;
      if((7-row)<rightMeasure) rowPattern|=     B00000010;
      if((row)<midMeasure) rowPattern|=B00011000;
      led8x8.setColumn(LED8X8_PANEL_0,7-row,rowPattern); /* using column to turn picture-90 degree */
    }
    return;
}
  

/* ***************** Letter   *************************************+ 
*/

/*     8 4 2 1 8 4 2 1         8 4 2 1 8 4 2 1        8 4 2 1 8 4 2 1                                                     
 * 8                       8                       8                                    
 * 4   x x x x x           4   X X X               4                                       
 * 2       x               2   X     X             2                                         
 * 1       x               1   X     X             1  X X   X X                               
 * 8       x               8   X     X             8                                        
 * 4       x               4   X     X             4                                        
 * 2       x               2   X X X               2                                      
 * 1                       1                    1                                     
 *        Trace                   Demo                  
 */ 

 /*     8 4 2 1 8 4 2 1        8 4 2 1 8 4 2 1        8 4 2 1 8 4 2 1                                                     
 * 8                       8                       8                                    
 * 4      X X X            4   X X X               4                                       
 * 2    X                  2   X     X             2                                         
 * 1      X X              1   X     X             1                                            
 * 8          X            8   X X X               8                                        
 * 4          X            4   X                   4                                        
 * 2    X X X              2   X                   2                                      
 * 1                       1                       1                                     
 *        Station                                       
 */ 
void output_render_letter(byte letter_index,  byte first_column) 
{
   const byte colPattern[][5]={      /* twisted 90 degree to left */
    /* 0 = D */ {0x7e,0x42,0x42,0x3c,0x00},
    /* 1 = T */ {0x40,0x40,0x7e,0x40,0x40},
    /* 2 = --*/ {0x10,0x10,0x00,0x10,0x10},
    /* 3 = S */ {0x22,0x52,0x52,0x4c,0x00},
    /* 4 = P */ {0x7e,0x48,0x48,0x30,0x00}
   };

   for(byte i=0;i<5;i++){
       led8x8.setRow(LED8X8_PANEL_0,i+first_column,(mirroredPattern(colPattern[letter_index][i]>>1)));     /* using row to turn picture-90 degree */
   }
}

/* *********    Render number *******************************+
 */

/*     8 4 2 1 8 4 2 1        8 4 2 1 8 4 2 1      8 4 2 1 8 4 2 1                                                     
 *                                                                
 * 1'  X 1'X       X       1  X 1'X   X 1'X     1  X   X   X 1'X   X 1'X   X 1'X   X 1'X     X 1'X
 * 8   8'  8       8       8      8       8     8  8'  8   8'      8'          8   8'  8     8'  8
 * 4   x   X       X       4  X 4 X   X 4 X     4  X 4 X   X 4 X   X 4 X       X   X 4 X     X 4 X
 * 2   2'  2       2       2  2'          2     2      2       2   2'  2       2   2'  2         2
 * 1   x 1 X       X       1  X 1 X   X 1 X     1      X   X 1 X   X 1 X       X   X 1 X     X 1 X
 *                                                                                       
 *                                                                                     
 */ 
/* Displays a 2 digit number */

byte getDigitColumnPattern(byte *columnBitmap,byte digitValue) 
{
                        // 0    1    2    3    4    5    6    7    8    9    <spc>
  byte digitBitmap[11]={0xbb,0x0a,0x3d,0x1f,0x8e,0x97,0xb7,0x1a,0xbf,0x9f};

      columnBitmap[0]=(digitBitmap[digitValue]>>4 &    B00001010) // main bits
                    | ( digitBitmap[digitValue]>>3 & B00010100) // fill up left
                    | ( digitBitmap[digitValue]>>5 & B00000101) // fill up right
                    | ( digitBitmap[digitValue] &    B00010101); // fill up from middle
  
    columnBitmap[1]=(digitBitmap[digitValue] & B00010101); // main bits
  
    columnBitmap[2]=(digitBitmap[digitValue] &       B00001010) // main bits
                    | ( digitBitmap[digitValue]<<1 & B00010100) // fill up left
                    | ( digitBitmap[digitValue]>>1 & B00000101) // fill up right
                    | ( digitBitmap[digitValue] &    B00010101); // fill up from middle

}


void output_render_number(byte theNumber,byte top_row_pattern) 
{
  byte columnBitmap[3];
  byte currentDigit;
  byte finalPattern;

  #ifdef TRACE_OUTPUT_NUMBER 
  Serial.print(F("render_number:"));Serial.println(theNumber);
  Serial.print(F("top_row_pattern:"));Serial.println(top_row_pattern);
  #endif
  
  led8x8.setRow(LED8X8_PANEL_0,4,(bitRead(top_row_pattern,4)));  // Empty space between  digits
  led8x8.setRow(LED8X8_PANEL_0,0,(bitRead(top_row_pattern,0)));  // Empty space between left of the digits
  for(int digit_index=0;digit_index<2;digit_index++)
  {
    
    currentDigit=theNumber%10; // Retrieve the lowest digit
    #ifdef TRACE_OUTPUT_NUMBER 
      Serial.print(F("digit"));Serial.println(currentDigit);
      Serial.print(F("digitBitmap"));Serial.println(digitBitmap[currentDigit],BIN);
    #endif
   
    getDigitColumnPattern(columnBitmap,currentDigit);

    for(int i=0;i<3;i++)
    {
      if(digit_index==0 || currentDigit>0) 
            finalPattern=mirroredPattern((columnBitmap[2-i]))| bitRead(top_row_pattern,7-i-digit_index*4);
      else  finalPattern=bitRead(top_row_pattern,7-i-digit_index*4); 
      led8x8.setRow(LED8X8_PANEL_0,7-i-digit_index*4,finalPattern);
      #ifdef TRACE_OUTPUT_NUMBER 
         Serial.print(F("Col:"));Serial.println(finalPattern,BIN);
      #endif
    }
  theNumber/=10;          // shift digit in input one to the right for next loop
  }        
               
}

void output_renderUint32Time_TEST()
{
  for(uint32_t time=0;time <28*3600000;time+=60000)
  {
    output_renderUint32Time(time,0);
    delayMicroseconds(500);
    if(time==3600000) delay(3000);
    if(time==6*3600000) delay(3000);
    if(time==12*3600000) delay(3000);
    if(time==18*3600000) delay(3000);
    if(time==24*3600000) delay(3000);
    if(time==25*3600000) delay(3000);
    if(time==28*3600000) delay(3000);
  }
}

void output_renderDemoRampupMeasures()
{
  for(int i=0;i<8;i++)
  {
    output_renderRampupMeasures(i,i,i);
    delay(1000);
  }
  
  for(int i=0;i<24;i++)
  {
    output_renderRampupMeasures(i,i/2,i/3);
    delay(1000);
  }
}

void output_setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  led8x8.shutdown(LED8X8_PANEL_0,false);
  /* Set the brightness to a medium values */
  led8x8.setIntensity(LED8X8_PANEL_0,0);
  /* and clear the display */
  led8x8.clearDisplay(LED8X8_PANEL_0);

  //output_renderUint32Time_TEST();
  //output_renderDemoRampupMeasures();

  output_sequence_acknowlegde();
  output_sequence_escape();
}
