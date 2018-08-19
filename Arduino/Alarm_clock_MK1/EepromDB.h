/* Implements a simple method to use eeprom storage in a failsafe way and with preventig fast waerdown*/ 
#ifndef __EEPROM_DB_H__
#define __EEPROM_DB_H__

#include <Arduino.h>

#define EEPROMDB_NOT_INITIALIZED -1

class EepromDB {

  public:
    EepromDB();

     /* Initialize the DB object, check the storage 
      *  Returns true when DB was initialized successfully
      */
     bool setupDB(int startAdress, int recordsize, int generationCount);

     /* get the current record from the buffer
      *  Parameter: Poiter to the buffer, to write the data to
      *  Return: true - data was read from DB
      *          false - data could not be read from DB (no record or not intialized), buffer will be set to FF
      */
     bool readRecord(byte *buffer);

     /* write a new version of the record to the eeprom, will not write new record when it is identical to current 
      *   Parameter: Pointer to the buffer wich has to be written
      *   Return: true - data was written successfully
     */
     bool updateRecord(byte *buffer);

     /* provide first Adress, not occupoed by DB */
     int getAdressBehindDB() {return db_startAdress+(db_recordSize+1)*db_generationCount;}

     /* Dump the whole DB Storage to the Serial Interface */
     void dumpToSerial(void);
     
  protected:

     bool formatDB();
     
     int db_startAdress=-1;
     int db_recordSize=0;
     int db_generationCount=0;
     byte db_currentTransactionIndex=255;
     byte db_currentRowIndex=0;
};

#endif

