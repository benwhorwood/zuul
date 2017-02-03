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
#ifndef EEPROMI2C_h
#define EEPROMI2C_h

#include "Arduino.h"
//#include <Wire.h> // I2C library

/*
http://playground.arduino.cc/code/I2CEEPROM

Arduino analog pin 4 to EEPROM pin 5
Arduino analog pin 5 to EEPROM pin 6
Arduino 5V to EEPROM pin 8
Arduino GND to EEPROM pin 1,2,3,4

===

http://arduino.cc/en/reference/wire
Board	I2C / TWI pins
Uno, Ethernet	A4 (SDA), A5 (SCL)
Mega2560	20 (SDA), 21 (SCL)
Leonardo	2 (SDA), 3 (SCL)
Due	20 (SDA), 21 (SCL), SDA1, SCL1

*/

class EEPROMI2C
{
  public:
    EEPROMI2C();
    void writeByte(int deviceaddress, unsigned int eeaddress, byte data);
    void writePage(int deviceaddress, unsigned int eeaddresspage, byte* data, byte length);
    byte readByte(int deviceaddress, unsigned int eeaddress);
    void readBuffer(int deviceaddress, unsigned int eeaddress, byte *buffer, int length);
};

#endif
