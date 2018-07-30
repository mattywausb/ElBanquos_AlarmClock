/// The SI4703 board has to be connected by using the following connections:
/// | Arduino UNO pin    | Radio chip signal  | 
/// | -------------------| -------------------| 
/// | 3.3V (red)         | VCC                | 
/// | GND (black)        | GND                | 
/// | A5 or SCL (yellow) | SCLK               | 
/// | A4 or SDA (blue)   | SDIO               | 
/// | D2 (white)         | RST                |

#include "mainSettings.h"

#include <Arduino.h>
#include <Wire.h>
#include <radio.h>
#include <si4703.h>
#include <RDSParser.h>

#ifdef TRACE_ON
//#define TRACE_RADIO 1
//#define DEBUG_RDS_MOCKUP 1
#endif


// ----- Fixed settings here. -----

#define FIX_BAND     RADIO_BAND_FM   ///< The band that will be tuned by this sketch is FM.
#define FIX_STATION  10260            // FRITZ in Berlin
//#define FIX_STATION  9580            // Radio 1 in Berlin
#define FINAL_VOLUME   15               ///< The volume that will finally be set by this sketch is level 8.
#define FADE_STEP_INTERVAL  3500      // milliseconds to wait until next fade step

#define RDS_UPTODATE_THRESHOLD 50000  // Milliseconds we declare our information as actual

// --- State memory and objects
SI4703 radio;    // Create an instance of Class for Si4703 Chip
RDSParser rds;

#define RADIO_FLAG_PLAY     1
#define RADIO_FLAG_FADE_IN  2
#define RADIO_FLAG_FADE_OUT 3
#define RADIO_FLAG_RDS      5

byte radio_operation_flags=0;

int radio_rdsTimeInfo=-1; // Time of day in minutes, -1= no time available
unsigned long  radio_lastRdsCatchTime=0;
unsigned long  radio_lastFadeFrameTime=0;
char radio_lastStationName[20]="<unknown>";

/* interface */
int radio_getLastRdsTimeInfo() { 
  #ifdef DEBUG_RDS_MOCKUP
     return 16*60+29;
  #endif
  return radio_rdsTimeInfo;
  };

bool radio_getRdsIsUptodate() {

  #ifdef DEBUG_RDS_MOCKUP
  return true;
  #endif
  if(radio_lastRdsCatchTime==0) return false; // obviously we never have seen anything
  if(millis()-radio_lastRdsCatchTime <  RDS_UPTODATE_THRESHOLD)  return true;
  return false;
  };

void radio_setRdsScanActive(bool flag)
{
  bitWrite(radio_operation_flags,RADIO_FLAG_RDS,flag);
}

bool radio_isPlaying()
{
  return (bitRead(radio_operation_flags,RADIO_FLAG_PLAY)|bitRead(radio_operation_flags,RADIO_FLAG_FADE_IN))>0;
}

void radio_fadeIn() {
    if(bitRead (radio_operation_flags,RADIO_FLAG_PLAY)) return;
    bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
    bitSet(radio_operation_flags,RADIO_FLAG_FADE_IN);
    bitSet(radio_operation_flags,RADIO_FLAG_PLAY);    
    radio_lastFadeFrameTime=millis();
}

void radio_switchOn(){
  bitSet(radio_operation_flags,RADIO_FLAG_PLAY);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
  radio.setMute(false);
  radio.setVolume(FINAL_VOLUME);
}

void radio_switchOff(){
  radio.setVolume(0);
  bitClear(radio_operation_flags,RADIO_FLAG_PLAY);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
}

/* internal functions */
void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}

void catchStationNameFromRDS(char *name)
{
  strcpy(radio_lastStationName,name);
  #ifdef  TRACE_RADIO 1
      Serial.print("catched station:");
      Serial.println(radio_lastStationName);
  #endif
}


void catchTimeFromRDS(uint8_t hour, uint8_t minute)
{
  radio_lastRdsCatchTime=millis();
  radio_rdsTimeInfo=hour*60+minute;
  #ifdef  TRACE_RADIO 1
  Serial.print("catched RDS time:");  Serial.print(hour);  Serial.print(":");  Serial.println(minute);
  #endif
}

void displayStatus() {
      char s[12];
      radio.formatFrequency(s, sizeof(s));
      Serial.print("Station:"); 
      Serial.println(s);
      
      Serial.print("Radio:"); 
      radio.debugRadioInfo();
      
      Serial.print("Audio:"); 
      radio.debugAudioInfo();

      Serial.print("Last fetched Station Name:");
      Serial.println(radio_lastStationName);
}


/// Setup a FM only radio configuration
/// with some debugging on the Serial port
void radio_setup() {  


  // Enable information to the Serial port
  #ifdef TRACE_RADIO
    radio.debugEnable();
  #endif

  
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,false);
   
  // Initialize the Radio 
  radio.init();


    
  // Set all radio setting to the fixed values.
  radio.setVolume(0);
  radio.setBandFrequency(FIX_BAND, FIX_STATION);
  radio.setMute(true);
  radio.setBassBoost(true);
  radio.setMono(true);

  // initialize RDS Features
  radio.attachReceiveRDS(RDS_process);
  rds.attachTimeCallback(catchTimeFromRDS);
  rds.attachServicenNameCallback(catchStationNameFromRDS);

  radio.setMute(false);

  #ifdef TRACE_RADIO
      Serial.println("Radio is up and running");
      displayStatus();
  #endif
} // setup


// Babysit the radio for rds or sound fading

void radio_loop_tick() {
  if(bitRead(radio_operation_flags,RADIO_FLAG_RDS)) {
         radio.checkRDS();
  }
  digitalWrite(LED_BUILTIN,radio_operation_flags>0);

  if(bitRead(radio_operation_flags,RADIO_FLAG_FADE_IN) && millis()-radio_lastFadeFrameTime>FADE_STEP_INTERVAL) {
    radio_lastFadeFrameTime=millis();
    if(radio.getVolume()<FINAL_VOLUME) radio.setVolume(radio.getVolume()+1);
    else bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
  }
  
  #ifdef TRACE_RADIO
//      Serial.println("radio loop tick");
  #endif

} // loop

// End.
