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
#define TRACE_RADIO 1
#define DEBUG_RDS_MOCKUP 1
#endif


// ----- Fixed settings here. -----

#define FIX_BAND     RADIO_BAND_FM   ///< The band that will be tuned by this sketch is FM.

#define FINAL_VOLUME   15               ///< The volume that will finally be set by this sketch is level 8.
#define FADE_STEP_INTERVAL  3500      // milliseconds to wait until next fade step
#define RDS_SCAN_VOLUME 0          // Volume to use when scann (for debug reasons can be set >0)
#define RDS_UPTODATE_THRESHOLD 50000  // Milliseconds we declare our information as actual

/* the following presets should provde rds time data in my time zone */
int stationPreset[]={ 10260   // RBB FRITZ in Berlin
                      ,9580   // RBB Radio 1 in Berlin
                      ,9230   // RBB Info Radio Berlin
                      ,8880   // RBB 88,8 Berlin
                    } ;

byte currentPlayStation=0;
byte currentRdsStation=1;

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


/* ################   interface   #########################################
 * #####################################################################          
 */

 /* ----------- Operations --------------- */



void radio_setRdsScanActive(bool flag)
{
  if(flag==bitRead(radio_operation_flags,RADIO_FLAG_RDS)) return; 
  bitWrite(radio_operation_flags,RADIO_FLAG_RDS,flag);
  if(flag) radio_activateRdsStation();  /* acitvate rds station if possible */
}

void radio_activateRdsStation() {
  if(!bitRead(radio_operation_flags,RADIO_FLAG_PLAY)) {  /* if nobody listens */
    if(bitRead(radio_operation_flags,RADIO_FLAG_RDS)) {
      radio.setVolume(RDS_SCAN_VOLUME);   /* Switch to RDS Station */ 
      radio.setMute(false);
      radio.setBandFrequency(FIX_BAND, stationPreset[currentRdsStation]);  
      #ifdef TRACE_RADIO
            displayStatus();
      #endif
    } else {
      radio.setMute(true); /* shutdown radio */
    }
  }
}


void radio_switchRdsStation() {
  if(++currentRdsStation>=(sizeof(stationPreset)/sizeof(stationPreset[0]))){
    currentRdsStation=0;  
  }
  radio_activateRdsStation();
}


void radio_fadeIn() {
    if(bitRead (radio_operation_flags,RADIO_FLAG_PLAY)) return; /* Dont start fafing when already running */
    bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
    bitSet(radio_operation_flags,RADIO_FLAG_FADE_IN);
    bitSet(radio_operation_flags,RADIO_FLAG_PLAY);    
    radio_lastFadeFrameTime=millis();
    radio.setVolume(0);   /* Switch to RDS Station */ 
    radio.setMute(false);
    radio.setBandFrequency(FIX_BAND, stationPreset[currentPlayStation]);  
    #ifdef TRACE_RADIO
            displayStatus();
    #endif
}

void radio_fadeOut() {
    if(!bitRead (radio_operation_flags,RADIO_FLAG_PLAY)) return; /* Dont start fading when already off */
    bitSet(radio_operation_flags,RADIO_FLAG_FADE_OUT);
    bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
    bitSet(radio_operation_flags,RADIO_FLAG_PLAY);    
    radio_lastFadeFrameTime=millis();
}

void radio_switchOn(){
  if(!bitRead(radio_operation_flags,RADIO_FLAG_PLAY)){  /* not already playing */
      radio.setVolume(0);   /* Switch to RDS Station */ 
      radio.setMute(false);
      radio.setBandFrequency(FIX_BAND, stationPreset[currentPlayStation]);  
      radio.clearRDS();
      #ifdef TRACE_RADIO
            displayStatus();
      #endif
  }  
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
  radio_activateRdsStation();
}


/* -----------  Information ------------- */

int radio_getLastRdsTimeInfo() { 
  #ifdef DEBUG_RDS_MOCKUP
//     return 16*60+29;
     return 23*60+54;
  #endif
  return radio_rdsTimeInfo;
  };


unsigned long radio_getRdsTimeAge() {  /* Returns Age of the rds information in seconds (600000= 10 min = infinity) */
  #ifdef DEBUG_RDS_MOCKUP
  return 0;
  #endif
  if(radio_lastRdsCatchTime==0) return 600000; // obviously we never have seen anything
  return(millis()-radio_lastRdsCatchTime);
};


bool radio_isPlaying()  /* Radio is declared as play when playing or fading in */
{
  return (bitRead(radio_operation_flags,RADIO_FLAG_PLAY)& !bitRead(radio_operation_flags,RADIO_FLAG_FADE_OUT));
}

/* Get signal strengh in dBm */
uint8_t radio_getSignalStrength()
{
  RADIO_INFO radio_info;
  radio.getRadioInfo(&radio_info);
  return radio_info.rssi; // 75 dBm is maximum value from the radio
}

int radio_getFrequency()
{
  if(radio_isPlaying()) return stationPreset[currentPlayStation];
  return stationPreset[currentRdsStation];
}


/* ################   internal functions   #############################
 * #####################################################################          
 */

/* ---------- RDS Callback function, necessary to capture RDS data ------- */
void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}

void catchStationNameFromRDS(char *name)
{
  strcpy(radio_lastStationName,name);
  #ifdef  TRACE_RADIO 1
      Serial.print(F("catched station:"));
      Serial.println(radio_lastStationName);
  #endif
}


void catchTimeFromRDS(uint8_t hour, uint8_t minute)
{
  radio_lastRdsCatchTime=millis();
  radio_rdsTimeInfo=hour*60+minute;
  #ifdef  TRACE_RADIO 1
  Serial.print(F("catched RDS time:"));  Serial.print(hour);  Serial.print(":");  Serial.println(minute);
  #endif
}

void displayStatus() {
      char s[12];
      radio.formatFrequency(s, sizeof(s));
      Serial.print(F("Station:")); 
      Serial.println(s);
      
      Serial.print(F("Radio:")); 
      radio.debugRadioInfo();
      
      Serial.print(F("Audio:")); 
      radio.debugAudioInfo();
}

/* ################   Setup   #############################
 * #####################################################################          
 */


void radio_setup() 
{  

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
  radio.setBandFrequency(FIX_BAND, stationPreset[currentPlayStation]);
  radio.setMute(true);
  radio.setBassBoost(true);
  radio.setMono(true);

  // initialize RDS Features
  radio.attachReceiveRDS(RDS_process);
  rds.attachTimeCallback(catchTimeFromRDS);
  rds.attachServicenNameCallback(catchStationNameFromRDS);

  radio.setMute(false);

  #ifdef TRACE_RADIO
      Serial.println(F("Radio is up and running"));

  #endif
} // setup


/* ################   Tick   #############################
 * #####################################################################          
 */


void radio_loop_tick() 
{
  /* check for RDS data */
  if(bitRead(radio_operation_flags,RADIO_FLAG_RDS)) {
         radio.checkRDS();
  }
  digitalWrite(LED_BUILTIN,radio_operation_flags>0);

  /* fade in procedure */
  if(bitRead(radio_operation_flags,RADIO_FLAG_FADE_IN) && millis()-radio_lastFadeFrameTime>FADE_STEP_INTERVAL) {
    radio_lastFadeFrameTime=millis();
    if(radio.getVolume()<FINAL_VOLUME) radio.setVolume(radio.getVolume()+1);
    else bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
  }
  
  #ifdef TRACE_RADIO
//      Serial.println(F("radio loop tick"));
  #endif

} // loop


