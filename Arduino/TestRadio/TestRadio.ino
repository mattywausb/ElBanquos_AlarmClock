///
/// \file  TestSI4703.ino
/// \brief An Arduino sketch to operate a SI4703 chip based radio using the Radio library.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// \details
/// This sketch implements a "as simple as possible" radio without any possibility to modify the settings after initializing the chip.\n
/// The radio chip is initialized and setup to a fixed band and frequency. These settings can be changed by modifying the 
/// FIX_BAND and FIX_STATION definitions. 
///
/// Open the Serial console with 57600 baud to see the current radio information.
///
/// Wiring
/// ------ 
/// The SI4703 board has to be connected by using the following connections:
/// | Arduino UNO pin    | Radio chip signal  | 
/// | -------------------| -------------------| 
/// | 3.3V (red)         | VCC                | 
/// | GND (black)        | GND                | 
/// | A5 or SCL (yellow) | SCLK               | 
/// | A4 or SDA (blue)   | SDIO               | 
/// | D2 (white)         | RST                |
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// CHangeLog:
/// ----------
/// * 05.08.2014 created.
/// * 03.05.2015 corrections and documentation.

#include <Arduino.h>
#include <Wire.h>
#include <radio.h>
#include <si4703.h>
#include <RDSParser.h>

// ----- Fixed settings here. -----

#define FIX_BAND     RADIO_BAND_FM   ///< The band that will be tuned by this sketch is FM.
#define FIX_STATION  10260            // FRITZ in Berlin
#define FIX_VOLUME   4               ///< The volume that will be set by this sketch is level 4.

SI4703 radio;    // Create an instance of Class for Si4703 Chip
RDSParser rds;

char radio_lastStationName[20]="<unknown>";

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}


void catchStationNameFromRDS(char *name)
{
  Serial.print("Station Name:");
  Serial.println(name);

  strcpy(radio_lastStationName,name);
}


void catchTimeFromRDS(uint8_t hour, uint8_t minute)
{
  Serial.print("Time:");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
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
void setup() {  
  // open the Serial port
  Serial.begin(9600);
  Serial.println("Radio");
  delay(200);

  // Enable information to the Serial port
  radio.debugEnable();

  // Initialize the Radio 
  radio.init();

  radio.setVolume(0);
  radio.setBandFrequency(FIX_BAND, FIX_STATION);
  radio.setMute(true);
  delay(200);
  


    
  // Set all radio setting to the fixed values.
    radio.setMono(true);
      radio.attachReceiveRDS(RDS_process);
  rds.attachTimeCallback(catchTimeFromRDS);
  rds.attachServicenNameCallback(catchStationNameFromRDS);

  radio.setMute(false);
  displayStatus();
  delay(5000);
  for(int i=1;i<=FIX_VOLUME;i++) {
    Serial.print(".");
    radio.setVolume(i);
    delay(5000);
  }
  Serial.println(".... running");
  
} // setup


/// show the current chip data every 3 seconds.
void loop() {


  static unsigned long lastDisplayTime=0;

  if(millis()-lastDisplayTime>60000)  {
      lastDisplayTime=millis();

      displayStatus();
   
  } /* minut display */

    // check for RDS data
  radio.checkRDS();
  delay(10);

} // loop

// End.
