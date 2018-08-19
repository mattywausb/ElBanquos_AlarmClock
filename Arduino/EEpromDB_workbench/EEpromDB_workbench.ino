
#define TRACE_ON

#include "EepromDB.h"

#define alarmSetting_eprom_startadress 0
#define alarmSetting_eeprom_generationCount 6

#define WRITE_BUTTON_PIN 3
#define READ_BUTTON_PIN 4

struct MyAlarmSettingRecord {
  int alarm_time;
  int sleep_time;
  byte radio_preset;
} sram_record;

EepromDB myFirstDB;


void dumpToSerial_MyAlarmSettingRecord(struct MyAlarmSettingRecord record)
{
  Serial.print(F("  alarm_time:"));Serial.println(record.alarm_time);
  Serial.print(F("  sleep_time:"));Serial.println(record.sleep_time);
  Serial.print(F("  radio_preset:"));Serial.println(record.radio_preset);    
}

void setup() {

  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif

  pinMode(WRITE_BUTTON_PIN,INPUT_PULLUP);
  pinMode(READ_BUTTON_PIN,INPUT_PULLUP);

  if(myFirstDB.setupDB(1, sizeof(sram_record), 4))  myFirstDB.dumpToSerial();
  else {
    Serial.println(F("-- DB INIT ERROR --")); 
  }

  sram_record.alarm_time=0;
  sram_record.sleep_time=0;
  sram_record.radio_preset=0;
  
  if(myFirstDB.readRecord((byte*)(&sram_record)))
  {    
    Serial.println(F("Got Initial data from EEPROM:")); 
    dumpToSerial_MyAlarmSettingRecord(sram_record);
  }
  
  #ifdef TRACE_ON
  Serial.println(F("--- Setup complete ---")); 
  #endif 
}

void loop() {
struct MyAlarmSettingRecord localBuffer;

  if(digitalRead(READ_BUTTON_PIN)==LOW) {
    myFirstDB.readRecord((byte*)(&localBuffer));
    Serial.println(F("Current DB Record:")); 
    dumpToSerial_MyAlarmSettingRecord(localBuffer);   
    sram_record.alarm_time=random(0,24);
    sram_record.sleep_time+=7;
    sram_record.radio_preset++;  
    Serial.println(F("Current Sram Record:")); 
    dumpToSerial_MyAlarmSettingRecord(sram_record);   
    delay(1500);  
  }

  if(digitalRead(WRITE_BUTTON_PIN)==LOW)
  {
    Serial.println(F("Writing-->"));
    dumpToSerial_MyAlarmSettingRecord(sram_record);   
    if(myFirstDB.updateRecord((byte*)(&sram_record)))
    {
        Serial.println(F("--commited--"));        
    }
    myFirstDB.dumpToSerial();
    delay(1500);
  }
}
