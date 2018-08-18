
#define TRACE_ON

#include "EepromDB.h"

#define alarmSetting_eprom_startadress 0
#define alarmSetting_eeprom_recordCount 6

struct MyAlarmSettingRecord {
  int alarm_time;
  int sleep_time;
  byte radio_preset;
  byte eeprom_transaction_index;
} the_record;

EepromDB myFirstDB;

void setup() {

 #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
 #endif

  if(myFirstDB.setupDB(0, sizeof(the_record)+1, 4))  myFirstDB.dumpToSerial();
  else {
    Serial.println(F("-- DB INIT ERROR --")); 
  }
  
#ifdef TRACE_ON
Serial.println(F("--- Setup complete ---")); 
#endif 
}

void loop() {
  // put your main code here, to run repeatedly:

}
