/* Run by client (reader)                                                       */

/* Preprocessors                                                                */
/* #include <lib>                                                               */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <Keypad.h>
/********************************************************************************/
/* #include "header.h"                                                          */
#include "MFRC522.h"
#include "buzzer.h"
#include "indicator.h"
#include "keypadInput.h"
#include "espnow.h"
#include "esch.hpp"
/********************************************************************************/

/* Variable init                                                                */
/* MFRC522 pins                                                                 */
constexpr uint8_t RST_PIN = 22;     // Reset pin (SCL - GPIO22)
constexpr uint8_t SS_PIN = 21;     // Slave select pin (SDA - GPIO21)
/********************************************************************************/
/* ESCH256                                                                      */
constexpr size_t d_len = 32ul;    // Data byte length
/********************************************************************************/

/* Class init                                                                   */
/* Initialize an instance of class rfid                                         */
MFRC522 rfid(SS_PIN, RST_PIN); 
/********************************************************************************/
/* Initialize an instance of class passCode                                     */
Keypad passKey = Keypad(makeKeymap(hexaKeys), rowPins, colPins, 4, 4); 
/********************************************************************************/

/* Variable declarations                                                        */
String passCode;
String UID;
String plainData;        // Variable to store data temporarily (UID / passcode)
int8_t isGivenAccess;   // Variable to store data to be received
String success;        // Variable to store if sending data was successful
/* ESCH256                                                                      */
uint8_t decimalData[d_len];            // Variable to store decimal plainData
uint8_t dig0[esch256::DIGEST_LEN];    // Variable to store digest of esch256
// uint8_t dig1[esch384::DIGEST_LEN];
/********************************************************************************/

/* Typedef declarations                                                         */
/* Structure of data to send to the server                                      */
typedef struct {
  uint8_t Dig[esch256::DIGEST_LEN];
} send_data;
/* Strcuture of data to receive from the server                                 */
typedef struct {
  int8_t Access;
} receive_data;
/********************************************************************************/
send_data mfrcKeypadReadings;        // A data to send to server
receive_data accessStatus;          // A data to receive from server
/********************************************************************************/

esp_now_peer_info_t peerInfo;

/********************************************************************************/
/* Start of ESP-NOW Functions                                                   */
/********************************************************************************/
/* Callback when data is sent                                                   */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? 
    "Delivery Success" : 
    "Delivery Fail"
  );
  if (status == 0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

/* Callback when data is received                                               */
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&accessStatus, incomingData, sizeof(accessStatus));
  Serial.print("Bytes received: ");
  Serial.println(len);
  isGivenAccess = accessStatus.Access;
}
/* End of ESP-NOW Functions                                                     */
/********************************************************************************/

/********************************************************************************/
/*Start of main functions                                                       */
/********************************************************************************/
void setup() {
  Serial.begin(115200);

  /* Init for MFRC522                                                           */
  SPI.begin(); // SPI bus
  rfid.PCD_Init(); // MFRC522

  /******************************************************************************/
  /* Start of ESP-NOW Setup                                                     */
  /******************************************************************************/
  /* Initialize ESP-NOW                                                         */
  WiFi.mode(WIFI_STA);  // Set as wifi station
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  /* register for Send CB to get the status of trasnmitted packet               */
  esp_now_register_send_cb(OnDataSent);

  /* Register peer                                                              */
  memcpy(peerInfo.peer_addr, bcAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  /* Add peer                                                                   */
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  /* Register for a callback function that will be called when data is received */
  esp_now_register_recv_cb(OnDataRecv);

  /* End of ESP-NOW Setup                                                       */
  /******************************************************************************/

  /* Set up pinMode for buzzer                                                  */
  pinMode(buzzerPin, OUTPUT);

  /* Set up pinMode for LED                                                     */
  pinMode(rPin, OUTPUT);
  pinMode(yPin, OUTPUT);
  pinMode(gPin, OUTPUT);

  digitalWrite(rPin, HIGH); // Power Indicator

  /* Show details of PCD - MFRC522 Card Reader details                          */
  rfid.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  /******************************************************************************/
  /* Start of Reading System                                                    */
  /******************************************************************************/
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) {
    /****************************************************************************/
    /*Start of Keypad Section                                                   */
    /****************************************************************************/
    char newKey = passKey.getKey();

    /* No newKey detected                                                       */
    if ( ! newKey) {
      /**************************************************************************/
      /* Start of Data Hashing System (plainData)                               */
      /**************************************************************************/
      if (plainData.length() > 0) {
        size_t idx = 0;
        /* Convert String to an array of ASCII decimal encoding                 */
        for (size_t i = 0ul; i < d_len; i++) {
          decimalData[i] = plainData.charAt(idx++);    // Get decimal value
          if (idx == 7) {
            idx = 0;
          }
        }
      }
      else {
        // plainData is empty
        // Assign 0 to decimalData
        std::memset(decimalData, 0, sizeof(decimalData));
      }
        
      Serial.print("Decimal values: ");
      for (int i = 0; i < d_len; i++) {
        Serial.print(decimalData[i]);
      }
      Serial.println();
      plainData = "";
      
      /* Init dig0 with 0                                                       */
      std::memset(dig0, 0, sizeof(dig0));
      /* Compute Esch256 digest                                                 */
      esch256::hash(decimalData, d_len, dig0);

      /* End of Data Hashing System                                             */
      /**************************************************************************/

      /**************************************************************************/
      /* Start of ESP-NOW System                                                */
      /**************************************************************************/
      /*Set up values to send                                                   */
      memcpy(mfrcKeypadReadings.Dig, dig0, sizeof(dig0));
      Serial.println();
      Serial.print("Digest 0 (mfrcKeypadReadings.Dig): ");
      for (int i = 0; i < 32; i++) {
        Serial.print("0x");
        if (mfrcKeypadReadings.Dig[i] < 0x10) {
          Serial.print("0"); // Add a leading zero for single-digit hex values
        }
        Serial.print(mfrcKeypadReadings.Dig[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      /* Send message via ESP-NOW                                               */
      esp_err_t result = esp_now_send(bcAddress, 
                                      (uint8_t *) &mfrcKeypadReadings, 
                                      sizeof(mfrcKeypadReadings));
      Serial.println((result == ESP_OK) ? 
        "Sent with success" : 
        "Error sending the data"
      );
    
      /* End of ESP-NOW System                                                  */
      /**************************************************************************/

      /**************************************************************************/
      /* Start of Indicator System                                              */
      /**************************************************************************/
      if (isGivenAccess == 1) {
        buzzer_1();
      }
      else if (isGivenAccess == 0) {
        buzzer_0();
      }
      else {
        // isGivenAccess = -1
        // Do nothing
      }
      Serial.print("Access Status: ");
      Serial.println(isGivenAccess);

      /* End of Indicator System                                                */
      /**************************************************************************/
      return;
    }
    /* newKey detected                                                          */
    else {
      buzzKey();
      if (newKey == 'D') {
        /* End of passCode                                                      */
        Serial.print("Passcode Inserted: ");
        Serial.println(passCode);

        plainData = passCode;
        passCode = "";        
        return;
      }
      /* Backspace input passcode                                               */
      else if (newKey == '*' && passCode != "") {
        passCode = passCode.substring(0, passCode.length() - 1);
        Serial.println(passCode);
        return;
      }
      else if (newKey == '*' && passCode == "") {
        return;
      }
      /* Get passcode                                                           */
      /* Append passCode to newKey                                              */
      passCode += newKey;
      Serial.println(passCode);
      return;
    }
    /* End of Keypad Section                                                    */
    /****************************************************************************/
  }

  /******************************************************************************/
  /* Start of RFID Section                                                      */
  /******************************************************************************/
  /* Reads UID of card and assign it to String UID                              */ 
  for (byte i = 0; i < rfid.uid.size; i++) {
    // Add "0" in front of a hex value if less than 0x10
    UID += (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + 
    // Add stringized hex value
    String(rfid.uid.uidByte[i], HEX);
  }

  /* Display UID to serial monitor */
  UID.toUpperCase();
  Serial.println(UID);

  plainData = UID;
  UID = "";
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  /* End of RFID Section                                                        */ 
  /******************************************************************************/

  /* End of Reading System                                                      */
  /******************************************************************************/
}

/* End of main functions                                                        */
/********************************************************************************/