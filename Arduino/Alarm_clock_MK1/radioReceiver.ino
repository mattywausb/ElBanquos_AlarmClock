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
#define TRACE_RADIO
#endif

#ifdef DEBUG
//#define DEBUG_RDS_MOCKUP_TIME 23*60+54
#define DEBUG_RDS_MOCKUP_TIME 7*60+54
#endif

//#define RADIO_PLUGGED



// ----- Fixed settings here. -----

#define FIX_BAND     RADIO_BAND_FM   ///< The band that will be tuned by this sketch is FM.

#define FINAL_VOLUME   15               ///< The volume that will finally be set by this sketch is level 8.
#define FADE_STEP_INTERVAL  1000      // milliseconds to wait until next fade step
#define RDS_SCAN_VOLUME 0          // Volume to use when scann (for debug reasons can be set >0)
#define RDS_UPTODATE_THRESHOLD 50000  // Milliseconds we declare our information as actual

/* the following presets should provde rds time data in my time zone */
int stationPreset[RADIO_PRESET_COUNT]={ 
                      10260   // RBB FRITZ in Berlin
                      ,9580   // RBB Radio 1 in Berlin
                      ,9230   // RBB Info Radio Berlin
                      ,8880   // RBB 88,8 Berlin
                    } ;

byte currentPlayStation=1;
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
      #ifdef RADIO_PLUGGED
      radio.setVolume(RDS_SCAN_VOLUME);   /* Switch to RDS Station */ 
      radio.setMute(false);
      radio.setBandFrequency(FIX_BAND, stationPreset[currentRdsStation]);  
      #endif
      #ifdef TRACE_RADIO
            displayStatus();
      #endif
    } else {
      #ifdef RADIO_PLUGGED
      radio.setMute(true); /* shutdown radio */
      #endif
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
    #ifdef RADIO_PLUGGED
    radio.setVolume(0);   
    radio.setMute(false);
    radio.setBandFrequency(FIX_BAND, stationPreset[currentPlayStation]);  
    #endif
    #ifdef TRACE_RADIO
          Serial.println(F("Radio: Start Fading in"));
          displayStatus();
    #endif
}

void radio_fadeOut() {
    if(!bitRead (radio_operation_flags,RADIO_FLAG_PLAY)) return; /* Dont start fading when already off */
    bitSet(radio_operation_flags,RADIO_FLAG_FADE_OUT);
    bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
    bitSet(radio_operation_flags,RADIO_FLAG_PLAY);    
    radio_lastFadeFrameTime=millis();
    #ifdef TRACE_RADIO
      Serial.println(F("Radio: Start Fading out"));
    #endif
}

void radio_switchOn(){
  if(!bitRead(radio_operation_flags,RADIO_FLAG_PLAY)){  /* not already playing */
      #ifdef RADIO_PLUGGED
      radio.setVolume(0);   /* Switch to RDS Station */ 
      radio.setMute(false);
      radio.setBandFrequency(FIX_BAND, stationPreset[currentPlayStation]);  
      radio.clearRDS();
      #endif

  }  
  bitSet(radio_operation_flags,RADIO_FLAG_PLAY);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
  #ifdef RADIO_PLUGGED
  radio.setMute(false);
  radio.setVolume(FINAL_VOLUME);
  #endif
  #ifdef TRACE_RADIO
  Serial.println(F("Radio: Switched On"));
  displayStatus();
  #endif
}

void radio_switchOff(){
  #ifdef RADIO_PLUGGED
  radio.setVolume(0);
  #endif
  bitClear(radio_operation_flags,RADIO_FLAG_PLAY);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
  bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
  radio_activateRdsStation();
  #ifdef TRACE_RADIO
  Serial.println(F("Radio: Switched Off"));
  #endif
}

void radio_setSelectedPreset(byte newPlayStation)
{
  currentPlayStation=newPlayStation;
  if(radio_isPlaying()) 
  {
    #ifdef RADIO_PLUGGED
    radio.setBandFrequency(FIX_BAND, stationPreset[currentPlayStation]);  
    radio.clearRDS();  
    #endif
  }

}

/* -----------  Information ------------- */

int radio_getLastRdsTimeInfo() { 
  #ifdef DEBUG_RDS_MOCKUP_TIME
     return (DEBUG_RDS_MOCKUP_TIME+millis()/60000)%1440;
  #endif
  return radio_rdsTimeInfo;
  };


/* get age of the rds information in millis (600000= 10 min = infinity) */
unsigned long radio_getRdsTimeAge() {  /* Returns Age of the rds information in millis (600000= 10 min = infinity) */
  #ifdef DEBUG_RDS_MOCKUP
  return 0;
  #endif

  #ifdef DEBUG_RDS_MOCKUP_TIME
     if(millis()-radio_lastRdsCatchTime>1000) {
        radio_lastRdsCatchTime=millis();
        Serial.print(F("Mocking RDS with time "));
        Serial.println((DEBUG_RDS_MOCKUP_TIME+millis()/60000)%1440);
        return 500; // age below rds cooldown
     } else return 600000; // Fake very high age
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
  #ifdef RADIO_PLUGGED
  radio.getRadioInfo(&radio_info);
  return radio_info.rssi; // 75 dBm is maximum value from the radio
  #else
  return 0 ;   // no radio in the system
  #endif
}

int radio_getFrequency()
{
  if(radio_isPlaying()) return stationPreset[currentPlayStation];
  return stationPreset[currentRdsStation];
}

byte radio_getSelectedPreset()
{
  return currentPlayStation;
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
      #ifdef RADIO_PLUGGED
      radio.formatFrequency(s, sizeof(s));
      Serial.print(F("Station:")); 
      Serial.println(s);
      
      Serial.print(F("Radio:")); 
      radio.debugRadioInfo();
      
      Serial.print(F("Audio:")); 
      radio.debugAudioInfo();
      #else
      Serial.println(F("No Radio plugged"));       
      #endif
}

/* ################   Setup   #############################
 * #####################################################################          
 */


void radio_setup() 
{  

  // Enable information to the Serial port
  #ifdef TRACE_RADIO
    #ifdef RADIO_PLUGGED
    radio.debugEnable();
    #endif
  #endif
  
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,false);
   
  // Initialize the Radio 
  #ifdef RADIO_PLUGGED
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
  #endif

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
         #ifdef RADIO_PLUGGED
         radio.checkRDS();
         #endif
  }

  /* Set BUILTIN LED according to PLAY State */
  digitalWrite(LED_BUILTIN,bitRead(radio_operation_flags,RADIO_FLAG_PLAY));
  //digitalWrite(LED_BUILTIN,radio_operation_flags>0);

  /* fade in procedure */
  if(bitRead(radio_operation_flags,RADIO_FLAG_FADE_IN) && millis()-radio_lastFadeFrameTime>FADE_STEP_INTERVAL) {
    radio_lastFadeFrameTime=millis();
    #ifdef RADIO_PLUGGED
    if(radio.getVolume()<FINAL_VOLUME) radio.setVolume(radio.getVolume()+1);
    else bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
    #else
    bitClear(radio_operation_flags,RADIO_FLAG_FADE_IN);
    #endif
  }

  /* fade out procedure */
  if(bitRead(radio_operation_flags,RADIO_FLAG_FADE_OUT) && millis()-radio_lastFadeFrameTime>FADE_STEP_INTERVAL) {
    radio_lastFadeFrameTime=millis();
    #ifdef RADIO_PLUGGED
    if(radio.getVolume()>0) radio.setVolume(radio.getVolume()-1);
    else {
      bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
      radio_switchOff();
    }
    #else
      bitClear(radio_operation_flags,RADIO_FLAG_FADE_OUT);
    #endif
  }
  
  #ifdef TRACE_RADIO
//      Serial.println(F("radio loop tick"));
  #endif

} // loop


