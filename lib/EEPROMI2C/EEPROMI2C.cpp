/* 
 *  Use the I2C bus with EEPROM 24LC64 
 * 
 *  Author: nebdoo
 *  Date: 17/03/2015
 *  
 *  Based on...
 *  Author: hkhijhe
 *  Date: 01/10/2010
 * 
 *   
 */

#include "Arduino.h"
#include "EEPROMI2C.h"
#include <Wire.h>

EEPROMI2C::EEPROMI2C() {}

void EEPROMI2C::writeByte(int deviceaddress, unsigned int eeaddress, byte data) 
{
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(rdata);
  Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void EEPROMI2C::writePage(int deviceaddress, unsigned int eeaddresspage, byte* data, byte length) 
{
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddresspage >> 8)); // MSB
  Wire.write((int)(eeaddresspage & 0xFF)); // LSB
  byte c;
  for ( c = 0; c < length; c++)
    Wire.write(data[c]);
  Wire.endTransmission();
}

byte EEPROMI2C::readByte(int deviceaddress, unsigned int eeaddress) 
{
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void EEPROMI2C::readBuffer(int deviceaddress, unsigned int eeaddress, byte *buffer, int length) 
{
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,length);
  int c = 0;
  for ( c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();
}
