/*
 * To pair NFC ring with EEPROM means that if ring is lost haven't
 * lost encryption keys.
 *
 * This sketch writes encryption keys which are additionally 
 * encrypted using key stored on ring (they must be decrypted 
 * before use).
 */

#include "aes256.h"
#include <Wire.h>
#include <EEPROMI2C.h> 
#define DUMP(str, i, buf, sz) { Serial.println(str); \
                               for(i=0; i<(sz); ++i) { if(buf[i]<0x10) Serial.print('0'); Serial.print(char(buf[i]), HEX); } \
                               Serial.println(); } //Help function for printing the Output

aes256_context ctxt;
EEPROMI2C eeprom;


void setup() 
{
  Wire.begin(); // initialise the connection
  Serial.begin(19200);
  while (!Serial)
  {
    ; // Wait for Leonardo
  }
  
  int i;
  
  Serial.println("Initializing AES256... ");
  uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
  };
  DUMP("Key: ", i, key, sizeof(key));
  aes256_init(&ctxt, key);
  
  // 
  
  /* Sets of 32 byte keys to be encrypted and written to 
   * EEPROM, can get hex values using:
   *
   * $ echo -n "iamnot32chars" | xxd
   */
  uint8_t data[] = {
    // First key, for login (addr 0..31)
    0x31, 0x2d, 0x5f, 0x2d, 0x77, 0x68, 0x61, 0x74, 
    0x61, 0x62, 0x65, 0x61, 0x75, 0x74, 0x69, 0x66, 
    0x75, 0x6c, 0x64, 0x61, 0x79, 0x69, 0x6e, 0x61, 
    0x70, 0x72, 0x69, 0x6c, 0x2d, 0x5f, 0x2d, 0x31
    
    // Second key (addr 32..63)
    
    // ...
  };
  DUMP("Unencrypted data: ", i, data, sizeof(data));
  Serial.print("  ");
  for (int j = 0; j < sizeof(data); j++ )
  {
    Serial.print((char)data[j]);
  }
  Serial.println(" ");
  
  aes256_encrypt_ecb(&ctxt, data);
  DUMP("Encrypted data: ", i, data, sizeof(data));
  
  /*aes256_decrypt_ecb(&ctxt, data);
  DUMP("Back decrypted data: ", i, data, sizeof(data));*/
  
  aes256_done(&ctxt);
  
  
  
  
  delay(3000);
  //eeprom.writePage(0x50, 0, (byte *)somedata, sizeof(somedata)); // write to EEPROM 
  //delay(10); //add a small delay
  
  for (int i = 0; i < sizeof(data); i++) 
  {
    //Serial.println((byte)somedata[i]);
    //Serial.println();
    
    //eeprom.writeByte(0x50, i, (byte)data[i]);
    eeprom.writeByte(0x50, i, data[i]);
    delay(10);
  }
  
  // Write closing byte to stop reading from EEPROM
  eeprom.writeByte(0x50, i, 0);
  delay(10);
    

  Serial.print("Memory written (");
  Serial.print(sizeof(data));
  Serial.println(" bytes)");
  //delay(2000);
  
  delay(5000);
  Serial.println(" ");
  Serial.println("Reading back...");
}

void loop() 
{
  int addr=0; //first address
  byte b = eeprom.readByte(0x50, 0); // access the first address from the memory
  Serial.print("First byte: ");
  Serial.print((char)b);
  Serial.print(" (");
  Serial.print(b, DEC);
  Serial.println(")");

  while (b!=0) 
  //while (b!=255) 
  //while(((uint8_t)b) != 0x7c) // Pipe |
  {
    Serial.print((char)b); //print content to serial port
    //Serial.print(b, DEC); //print content to serial port
    addr++; //increase address
    b = eeprom.readByte(0x50, addr); //access an address from the memory
    Serial.print((char)b); //print content to serial port
  }
  Serial.println(" ");
  delay(2000);

}
