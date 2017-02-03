#include "aes256.h"
#include <Wire.h>
#include <EEPROMI2C.h> 
#define DUMP(str, i, buf, sz) { Serial.println(str); \
                               for(i=0; i<(sz); ++i) { if(buf[i]<0x10) Serial.print('0'); Serial.print(char(buf[i]), HEX); } \
                               Serial.println(); } //Help function for printing the Output

aes256_context ctxt;
EEPROMI2C eeprom;

// NFC libs
#include <PN532_HSU.h>
#include <PN532.h>
    
PN532_HSU pn532hsu(Serial1);
PN532 nfc(pn532hsu);

boolean DEBUG = false;

int pinLedStatus = 8;

int k;

void setup() 
{
  Wire.begin(); // initialise the connection to EEPROM
  Serial.begin(115200);
  /*while (!Serial)
  {
    ; // Wait for Leonardo
  }*/

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  if (DEBUG) {
    // Got ok data, print it out!
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  }
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  pinMode(pinLedStatus, OUTPUT);
  
  // Signal end of init
  digitalWrite(pinLedStatus, HIGH);
  delay(2000);
  digitalWrite(pinLedStatus, LOW);
}

void loop()
{
  if (DEBUG) Serial.println("Enter command: ");
  
  if (!Serial.available()) {
    delay(100);
    return;
  }
  
  String cmd = Serial.readStringUntil('\n');
  if (DEBUG) Serial.println("You said: " + cmd);
  
  if (cmd.substring(0, 4) == "read") {
    String requestedKeyStr = cmd.substring(4);
    int requestedKey = requestedKeyStr.toInt();
    
    if (DEBUG) Serial.println("Reading key " + requestedKey);
    
    boolean ledStatusHigh = true;
    
    String key = NULL;
    while (key == NULL) {
      digitalWrite(pinLedStatus, HIGH);
      delay(100);
      digitalWrite(pinLedStatus, LOW);
      
      key = getKey(); // Has delay
    }
    
    // Write command back to client
    Serial.println("k:" + requestedKeyStr + ":" + key);
  }
  
  delay(1000);
}

String getKey()
{
  if (DEBUG) Serial.println("Waiting for an ISO14443A Card ...");
  
  uint8_t key[32]; // Fixed length key
  unsigned int keyIdx = 0;
  
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (!success) {
    return NULL;
  }
  
  if (DEBUG) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
  }

  // Mifare Classic card (4 byte UID) processing omitted
  
  if (uidLength == 7) // NFC Ring 
  {
    // We probably have a Mifare Ultralight card ...
    if (DEBUG) Serial.println("Seems to be a Mifare Ultralight tag (7 byte UID)");
    int pages = sizeof(key) / 4; // Pages are 4 bytes
    
    for (int i = 0; i < pages; i++) {
	  
      // Try to read the first general-purpose user page (#4)
      if (DEBUG) {
        Serial.print("Reading page ");
        Serial.println(i+4);
      }
      uint8_t data[4];
      success = nfc.mifareultralight_ReadPage (i+4, data);
      if (success)
      {
        for (int j = 0; j < 4; j++) 
        {
          key[keyIdx] = data[j];
          keyIdx++;
        }
        
        if (DEBUG) { 
          // Data seems to have been read ... spit it out
          nfc.PrintHexChar(data, 4);
          Serial.println("");
        }
		
        // Wait a bit before reading the card again
        if ((i+1) == pages) {
          //delay(1000);
        }
      }
      else
      {
        if (DEBUG) {
          Serial.println("Ooops ... unable to read the requested page!?");
          Serial.println("Trying to read tag again from start...");
        }
        delay(500);
        return NULL;
      }
    }
  }
  
  if (DEBUG) {
    Serial.print("Found key: ");
    nfc.PrintHexChar(key, 32);
  }
  
  // Decrypt data using key
  return decrypt(key);
  
  delay(5000);
}

String decrypt(uint8_t key[]) 
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
    if (DEBUG) Serial.print((char)b); //print content to serial port
    //Serial.print(b, DEC); //print content to serial port
    addr++; //increase address
    b = eeprom.readByte(0x50, addr); //access an address from the memory
  }
  
  if (DEBUG) {
    Serial.println(" ");
    Serial.print("Got to addr ");
    Serial.print(addr);
    Serial.println(" ");
  }
  
  // Decrypt
  
  if (DEBUG) Serial.println("Initializing AES256... ");
  /*uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
  };*/
  //DUMP("Key: ", k, key, sizeof(key));
  aes256_init(&ctxt, key);
  
  if (DEBUG) {
    Serial.print("data size: ");
    Serial.println(sizeof(data));
  }
  
  aes256_decrypt_ecb(&ctxt, data);
  if (DEBUG) DUMP("Back decrypted data: ", k, data, sizeof(data));
  
  String ret = "";
  for (int j = 0; j < sizeof(data); j++)
  {
    ret += (char)data[j];
    if (DEBUG) Serial.print((char)data[j]);
  }
  if (DEBUG) Serial.println(" ");
  
  aes256_done(&ctxt);
  
  return ret;
}
