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
#endif


// ----- Fixed settings here. -----

#define FIX_BAND     RADIO_BAND_FM   ///< The band that will be tuned by this sketch is FM.
#define FIX_STATION  10260            // FRITZ in Berlin
#define FINAL_VOLUME   8               ///< The volume that will finally be set by this sketch is level 8.


// --- State memory and objects
SI4703 radio;    // Create an instance of Class for Si4703 Chip
RDSParser rds;

int radio_rdsTimeInfo=-1; // Time of day in minutes, -1= no time available
unsigned long  radio_lastRdsCatchTime=0;
char radio_lastStationName[20]="<unknown>";

/* interface */
int radio_getLastRdsTimeInfo() { return radio_rdsTimeInfo;};

int radio_getLastRdsTimeAge() { 
#define MILLIES_OF_A_DAY 86400000
  unsigned long age_in_millis= (millis())-radio_lastRdsCatchTime;
  if(age_in_millis>MILLIES_OF_A_DAY) return MILLIES_OF_A_DAY/1000;
  return (millis()-radio_lastRdsCatchTime)/60000;
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
  
  // Initialize the Radio 
  radio.init();

  radio.setVolume(0);
  radio.setBandFrequency(FIX_BAND, FIX_STATION);
  radio.setMute(true);
  delay(200);
    
  // Set all radio setting to the fixed values.
  radio.setMono(true);

  // initialize RDS Features
  radio.attachReceiveRDS(RDS_process);
  rds.attachTimeCallback(catchTimeFromRDS);
  rds.attachServicenNameCallback(catchStationNameFromRDS);

  radio.setMute(false);

  #ifdef TRACE_RADIO
      Serial.println(".Radio  is running");
      displayStatus();
  #endif
} // setup


/// show the current chip data every 3 seconds.
void radio_loop_tick() {

  radio.checkRDS();
  #ifdef TRACE_RADIO
//      Serial.println("radio loop tick");
  #endif

} // loop

// End.
