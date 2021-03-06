#include "aes256.h"
#include <Wire.h>
#include <EEPROMI2C.h> 
#define DUMP(str, i, buf, sz) { Serial.println(str); \
                               for(i=0; i<(sz); ++i) { if(buf[i]<0x10) Serial.print('0'); Serial.print(char(buf[i]), HEX); } \
                               Serial.println(); } //Help function for printing the Output

aes256_context ctxt;
EEPROMI2C eeprom;

int i;


void setup() 
{
  Wire.begin(); // initialise the connection
  Serial.begin(19200);
  while (!Serial)
  {
    ; // Wait for Leonardo
  }
}

void loop() 
{
  int addr=0; //first address
  byte b = eeprom.readByte(0x50, 0); // access the first address from the memory
  /*Serial.print("First byte: ");
  Serial.print((char)b);
  Serial.print(" (");
  Serial.print(b, DEC);
  Serial.println(")");*/
  
  uint8_t data[32]; // Read buffer (needs to be the right size)

  while (b!=0) 
  {
    data[addr] = (uint8_t)b; // Append to buffer
    Serial.print((char)b); //print content to serial port
    //Serial.print(b, DEC); //print content to serial port
    addr++; //increase address
    b = eeprom.readByte(0x50, addr); //access an address from the memory
  }
  Serial.println(" ");
  Serial.print("Got to addr ");
  Serial.print(addr);
  Serial.println(" ");
  
  // Decrypt
  
  Serial.println("Initializing AES256... ");
  uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
  };
  DUMP("Key: ", i, key, sizeof(key));
  aes256_init(&ctxt, key);
  
  Serial.print("data size: ");
  Serial.println(sizeof(data));
  
  aes256_decrypt_ecb(&ctxt, data);
  DUMP("Back decrypted data: ", i, data, sizeof(data));
  
  for (int j = 0; j < sizeof(data); j++)
  {
    Serial.print((char)data[j]);
  }
  Serial.println(" ");
  
  aes256_done(&ctxt);
  
  delay(5000);

}
