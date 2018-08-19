#include <EEPROM.h>
#include "EepromDB.h"

#define TRACE_EEPROMDB

EepromDB::EepromDB()
{
  /* everything we would do, can result in an error but theres no way to communicate this
   *  thats why you have to initalize the object with the startDB method an check the resul
   */
}

bool EepromDB::setupDB(int startAdress, int recordSize, int generationCount) 
{
  #ifdef TRACE_EEPROMDB
      Serial.println(F(">EepromDB::setupDB"));
  #endif
 
  if(recordSize <1) return false;
  if(startAdress <0) return false;
  /* TODO: check for upper limit */
  if(generationCount <3) return false;

  #ifdef TRACE_EEPROMDB
      Serial.println(F("\tParameters ok"));
      dumpToSerial();
  #endif

  db_startAdress=startAdress;
  db_recordSize=recordSize;
  db_generationCount=generationCount;
  byte rowLength=db_recordSize+1;
  uint8_t prevTransIndex=EEPROM[startAdress+recordSize]; // get the transaction index of the first record
  uint8_t checkTransIndex;
  db_currentRowIndex=generationCount-1; // Set to last row (wich will be the conclusion if we dont find the gap)
  
  /* Analyze transaction index of all entries to check integrity of the DB storage and identify actual entry*/
  for (int rowIndex=1;rowIndex<db_generationCount;rowIndex++) {
    checkTransIndex=EEPROM[startAdress+(rowLength)*rowIndex+recordSize];

    #ifdef TRACE_EEPROMDB
      Serial.print(F("\tCheck "));Serial.print(prevTransIndex);
      Serial.print(F("-> "));Serial.print((prevTransIndex+1)%256);
      Serial.print(F(" == "));Serial.println(checkTransIndex);
    #endif
    if(checkTransIndex==(prevTransIndex+1)%256) // follow up record
    {
      prevTransIndex=checkTransIndex;
      continue;
    } 
    #ifdef TRACE_EEPROMDB
      Serial.print(F(" -> "));Serial.print((prevTransIndex+1)%256);
      Serial.print(F(" == "));Serial.println((checkTransIndex+db_generationCount)%256);
    #endif     
    if((checkTransIndex+db_generationCount)%256==(prevTransIndex+1)%256) // record gap = prev row is the current vaild row
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
  if(db_currentRowIndex==generationCount-1) // last row is the current one
  {
    db_currentTransactionIndex=EEPROM[startAdress+(rowLength)*(db_generationCount-1)+recordSize];
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
  
  for(int rowIndex=0;rowIndex<db_generationCount;rowIndex++) 
  {
    for(int byteIndex=0; byteIndex<db_recordSize;byteIndex++) {
      EEPROM[db_startAdress+rowIndex*rowLength+byteIndex]=0xff; // initialize the cell with FF
    }
    EEPROM[db_startAdress+rowIndex*rowLength+db_recordSize]=rowIndex+250; // Write initial transaction index
  }
  db_currentTransactionIndex=db_generationCount-1;
  db_currentRowIndex=db_generationCount; // this Row Index value is invalid to show we havent written anything yet
  return true;
}

void EepromDB::dumpToSerial()
{
  #ifdef TRACE_EEPROMDB
    Serial.println(F(">EepromDB::dumpToSerial"));
  #endif
  byte rowLength=db_recordSize+1;
    
  for(int rowIndex=0;rowIndex<db_generationCount;rowIndex++) 
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
  int byteIndex=0;
  bool hasSomeRealData=false;
  byte recordAdress=db_startAdress+db_currentRowIndex*(db_recordSize+1);

   /* check if db is initialized correctly */
  if(db_startAdress==-1 
   ||db_currentRowIndex==db_generationCount){
    return false;
   }
   
  /* copy the data */
  for(byteIndex=0;byteIndex<db_recordSize;byteIndex++) 
  {
    buffer[byteIndex]=EEPROM[recordAdress+byteIndex];  
    if(buffer[byteIndex]!=0xff)  hasSomeRealData=true;
  }
  return hasSomeRealData;
}


bool  EepromDB::updateRecord(byte *buffer)
{
  int byteIndex=0;
  bool isNewData=false;
  byte recordAdress=db_startAdress+db_currentRowIndex*(db_recordSize+1);

  /* check if db is initialized correctly */
  if(db_startAdress==-1){
    return false;
   }

    /* Check if there is a change */
  if(db_currentRowIndex!=db_generationCount) // db has a record
  {
    for(byteIndex=0;byteIndex<db_recordSize;byteIndex++) 
    {
      if(buffer[byteIndex]!=EEPROM[recordAdress+byteIndex])
      {
        isNewData=true;
        break;  
      }
    }    
  } else { // db has no record
     isNewData=true;
  }

  if(!isNewData) return true; // Nothing to change

  /* foreward transaction- and rowindex */
  db_currentTransactionIndex++;  // we use the byte overflow to construct a "ring buffer"
  if(++db_currentRowIndex >= db_generationCount) db_currentRowIndex=0;
  recordAdress=db_startAdress+db_currentRowIndex*(db_recordSize+1);

  /* write data to storage */
  for(byteIndex=0;byteIndex<db_recordSize;byteIndex++) 
  {
    EEPROM[recordAdress+byteIndex]=buffer[byteIndex];
  }
  /* Write new transaction index  (commit the data)*/
  EEPROM[recordAdress+db_recordSize]=db_currentTransactionIndex;
  return true;
}


