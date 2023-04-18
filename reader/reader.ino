/* Run by client (reader) */

#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include "buzzer.h"
#include "indicator.h"
#include "keypadInput.h"

constexpr uint8_t RST_PIN = 22;     // Reset pin (SCL - GPIO22)
constexpr uint8_t SS_PIN = 21;     // Slave select pin (SDA - GPIO21)

// Initialize an instance of class rfid
MFRC522 rfid(SS_PIN, RST_PIN); 

// Initialize an instance of class passCode
Keypad passKey = Keypad( makeKeymap(hexaKeys), rowPins, colPins, 4, 4); 

String passCode;
String UID;

void setup() {
  Serial.begin(115200);

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  // Set up pinMode for buzzer
  pinMode(buzzerPin, OUTPUT);

  /* LED pinMode */
  pinMode(rPin, OUTPUT);
  pinMode(yPin, OUTPUT);
  pinMode(gPin, OUTPUT);

  digitalWrite(rPin, HIGH); // Power Indicator

  rfid.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  int access;
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) {
    char newKey = passKey.getKey();

    /* No newKey detected */
    if ( ! newKey) {
      return;
    }
    else {
      buzzKey();
      if (newKey == 'D') {
        /* End of passCode */
        Serial.print("Passcode Inserted: ");
        Serial.println(passCode);

        /* Buzzer ON at access */
        if (passCode == "BC1558") {
          access = buzzer_1();
        }
        else {
          access = buzzer_0();
        }
        /* LED indicator */
        if (access) {
          // Granted
          greenIndic();
        }
        else {
          // Denied
          yellowIndic();
        }
        passCode = "";        
        return;
      }
      else if (newKey == '*' && passCode != "") {
        passCode = passCode.substring(0, passCode.length() - 1);
        Serial.println(passCode);
        return;
      }
      else if (newKey == '*' && passCode == "") {
        return;
      }
      /* Get passcode               */
      /* Append passCode to newKey  */
      passCode += newKey;
      Serial.println(passCode);
      return;
    }
  }

  /* Reads UID of card and assign it to String UID */ 
  for (byte i = 0; i < rfid.uid.size; i++) {
    // Add "0" in front of a hex value if less than 0x10
    UID += (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + 
    // Add stringized hex value
    String(rfid.uid.uidByte[i], HEX) +
    // Add ":" between every hex value
    (i != (rfid.uid.size - 1) ? ":" : "");
  }

  /* Display UID to serial monitor */
  UID.toUpperCase();
  Serial.println(UID);

  /* Buzzer ON at access */
  if (UID == "53:A9:3C:10") {
    access = buzzer_1();
  }
  else {
    access = buzzer_0();
  }

  /* LED indicator */
  if (access) {
    // Granted
    greenIndic();
  }
  else {
    // Denied
    yellowIndic();
  }

  UID = "";
  passCode = "";
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}