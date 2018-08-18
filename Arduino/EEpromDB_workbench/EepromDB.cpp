#include <EEPROM.h>
#include "EepromDB.h"

#define TRACE_EEPROMDB

EepromDB::EepromDB()
{
  /* everything we would do, can result in an error but theres no way to communicate this
   *  thats why you have to initalize the object with the startDB method an check the resul
   */
}

bool EepromDB::setupDB(int startAdress, int recordSize, int recordCount) 
{
  #ifdef TRACE_EEPROMDB
      Serial.println(F(">EepromDB::setupDB"));
  #endif
 
  if(recordSize <1) return false;
  if(startAdress <0) return false;
  /* TODO: check for upper limit */
  if(recordCount <3) return false;

  #ifdef TRACE_EEPROMDB
      Serial.println(F("\tParameters ok"));
  #endif

  db_startAdress=startAdress;
  db_recordSize=recordSize;
  db_recordCount=recordCount;
  byte rowLength=db_recordSize+1;
  byte prevTransIndex=EEPROM[startAdress+recordSize]; // get the transaction index of the first record
  byte checkTransIndex;
  db_currentRowIndex=recordCount-1; // Set to last row (wich will be the conclusion if we dont find the gap)
  
  /* Analyze transaction index of all entries to check integrity of the DB storage and identify actual entry*/
  for (int rowIndex=1;rowIndex<db_recordCount;rowIndex++) {
    checkTransIndex=EEPROM[startAdress+(rowLength)*rowIndex+recordSize];

    #ifdef TRACE_EEPROMDB
      Serial.print(F("\tCheck "));Serial.print(prevTransIndex);
      Serial.print(F(" -> "));Serial.println(checkTransIndex);
    #endif
    if(checkTransIndex==prevTransIndex+1) // follow up record
    {
      prevTransIndex=checkTransIndex;
      continue;
    } 
    
    if(checkTransIndex==prevTransIndex-db_recordCount) // record gap = prev row is the current vaild row
    {
      db_currentTransactionIndex=prevTransIndex;
      db_currentRowIndex=rowIndex-1;
      prevTransIndex=checkTransIndex;
      continue;
    } 
        
     // TransactionIndex is not in order= this memory is not formatted correctly
      formatDB();
      break;
  }
  if(db_currentRowIndex==recordCount-1) // last row is the current one
  {
    db_currentTransactionIndex=EEPROM[startAdress+(rowLength)*(db_recordCount-1)+recordSize];
  }
  #ifdef TRACE_EEPROMDB
      Serial.print(F("\tTRIDX: "));Serial.print(db_currentTransactionIndex);
      Serial.print(F(" row index "));Serial.println(db_currentRowIndex);
  #endif
  return true;  
}

bool EepromDB::formatDB()
{
  byte rowLength=db_recordSize+1;
  #ifdef TRACE_EEPROMDB
  Serial.println(F(">EepromDB::formatDB"));
  #endif
  
  for(int rowIndex=0;rowIndex<db_recordCount;rowIndex++) 
  {
    for(int byteIndex=0; byteIndex<db_recordSize;byteIndex++) {
      EEPROM[db_startAdress+rowIndex*rowLength+byteIndex]=0xff; // initialize the cell with FF
    }
    EEPROM[db_startAdress+rowIndex*rowLength+db_recordSize]=rowIndex; // Write initial transaction index
  }
  db_currentTransactionIndex=db_recordCount-1;
  db_currentRowIndex=db_recordCount;
  return true;
}

void EepromDB::dumpToSerial()
{
  #ifdef TRACE_EEPROMDB
    Serial.println(F(">EepromDB::dumpToSerial"));
  #endif
  byte rowLength=db_recordSize+1;
    
  for(int rowIndex=0;rowIndex<db_recordCount;rowIndex++) 
  {
    Serial.print(F("\tRow "));
    Serial.print(rowIndex);
    Serial.print(F("\t: "));
    
    for(int byteIndex=0; byteIndex<db_recordSize;byteIndex++) {
      Serial.print(EEPROM[db_startAdress+rowIndex*rowLength+byteIndex],HEX);
      Serial.print(F(" "));
    }
    Serial.print(F("\t TRIDX:"));
    Serial.println(EEPROM[db_startAdress+rowIndex*rowLength+db_recordSize]); 
  }
}

bool EepromDB::readRecord(byte *buffer)
{

  /* Stub */
  return false;
}


bool  EepromDB::updateRecord(byte *buffer)
{
   /* Stub */
   return false;
}


