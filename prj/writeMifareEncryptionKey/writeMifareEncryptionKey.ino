/**************************************************************************/
/*! 
    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.

    To enable debug message, define DEBUG in PN532/PN532_debug.h
*/
/**************************************************************************/

/*#if 0
  #include <SPI.h>
  #include <PN532_SPI.h>
  #include "PN532.h"

  PN532_SPI pn532spi(SPI, 10);
  PN532 nfc(pn532spi);
#elif 1*/
  #include <PN532_HSU.h>
  #include <PN532.h>
      
  PN532_HSU pn532hsu(Serial1);
  PN532 nfc(pn532hsu);
/*#else 
  #include <Wire.h>
  #include <PN532_I2C.h>
  #include <PN532.h>
#endif*/

boolean arrayCmp(uint8_t *a, uint8_t *b, int len_a, int len_b){
     int n;

     // if their lengths are different, return false
     if (len_a != len_b) return false;

     // test each element to be the same. if not, return false
     for (n=0;n<len_a;n++) if (a[n]!=b[n]) return false;

     //ok, if we have not returned yet, they are equal :)
     return true;
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
}


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength != 7) // Not NFC Ring 
      return;
      
    uint8_t bensNfcUid[] = { 0x04, 0x27, 0xa8, 0x32, 0x93, 0x36, 0x80 };
      
    // Check for known UID
    if (!arrayCmp(uid, bensNfcUid, sizeof(uid), sizeof(bensNfcUid))) { 
      Serial.println("I don't know this NFC tag");
      delay(1000);
      return;
    }
    
    Serial.println("Found Ben's NFC Ring, writing data...");
    
    // Write some data
    //unsigned char dataToWrite[] = { 's', 'a', 'm', 'm' };
    uint8_t encKey[] = { // Decryption key (multiple of 4)
      //0xff, // First byte is separator
      
      // Fixed lenfth key of 32 bytes
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
      
      //0xff, 0x00, 0x00 // Make up separator to multiple of 4
    };
    
    int nfcPage = 4; // Start to write the first general-purpose user page (#4)
    for (int i = 0; i < sizeof(encKey); i+=4) {
      Serial.print("Writing ");
      Serial.print(i);
      Serial.print("+4 to page ");
      Serial.println(nfcPage);
      
      uint8_t dataToWrite [] = { encKey[i], encKey[i+1], encKey[i+2], encKey[i+3] };
      
      success = nfc.mifareultralight_WritePage(nfcPage, dataToWrite);
      nfcPage++;
      
      delay(500);
    }
    
    if (success) {
      Serial.println("Done");
    }
    else {
      Serial.println("");
      Serial.println("ERROR");
    }
    
    
    delay(5000);
    return;
      
    // We probably have a Mifare Ultralight card ...
    Serial.println("Seems to be a Mifare Ultralight tag (7 byte UID)");
    int pages = 2; // Arbitary
    
    for (int i = 0; i < pages; i++) {
	  
      // Try to read the first general-purpose user page (#4)
      Serial.print("Reading page ");
      Serial.println(i+4);
      uint8_t data[32];
      success = nfc.mifareultralight_ReadPage (i+4, data);
      if (success)
      {
        // Data seems to have been read ... spit it out
        nfc.PrintHexChar(data, 4);
        Serial.println("");
		
        // Wait a bit before reading the card again
        if ((i+1) == pages) {
          delay(1000);
        }
      }
      else
      {
        Serial.println("Ooops ... unable to read the requested page!?");
      }
    }
    
    delay(1000);
  }
}

