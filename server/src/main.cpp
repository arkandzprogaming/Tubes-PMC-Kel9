/* Run by server                                                                */

/* Preprocessors                                                                */
/* #include <lib>                                                               */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
/********************************************************************************/
/* #include "header.h"                                                          */
#include "espnow.h"
#include "esch.hpp"
#include "sha256.h"
/********************************************************************************/
/* #define CONST [value]                                                        */
#define MAX 2

/* Variable init                                                                */
/* ESCH256                                                                      */
constexpr size_t d_len = 32ul;    // Data byte length
/********************************************************************************/

/* Variable declarations                                                        */
String passCode;
String UID;
int8_t isGivenAccess;   // Variable to store data to be sent (access status)
String success;        // Variable to store if sending data was successful
String data;          // Variable to store data for authorization
/* ESCH256                                                                      */
uint8_t dig0[esch256::DIGEST_LEN];    // Variable to store digest of esch256
/* SHA-256                                                                      */
BYTE digSHA[SHA256_BLOCK_SIZE];        // Variable to store digest of SHA-256
/********************************************************************************/

/* Typedef declarations                                                         */
/* Structure of data to receive from the reader                                 */
typedef struct {
  uint8_t Dig[esch256::DIGEST_LEN];
} receive_data;
// typedef struct {
//   BYTE Dig[SHA256_BLOCK_SIZE];
// } receive_data;
/* Strcuture of data to send to the reader                                      */
typedef struct {
  int8_t Access;
} send_data;
typedef struct {
  uint8_t DigEmpty[esch256::DIGEST_LEN];
  uint8_t DigUid[MAX][esch256::DIGEST_LEN];
  uint8_t DigPasscode[esch256::DIGEST_LEN];
} data_auth;
// typedef struct {
//   uint8_t DigEmpty[SHA256_BLOCK_SIZE];
//   uint8_t DigUid[MAX][SHA256_BLOCK_SIZE];
//   uint8_t DigPasscode[SHA256_BLOCK_SIZE];
// } data_auth;
/********************************************************************************/
send_data accessStatus;                 // A data to send to reader
receive_data mfrcKeypadReadings;       // A data to receive from reader
/********************************************************************************/

/* Variable init                                                                */
/* Esch256                                                                      */
data_auth dataAuth = {
  .DigEmpty = {
    0xbf, 0x18, 0x25, 0xeb, 0xc8, 0x54, 0xd6, 0x66, 
    0x2d, 0x00, 0xb9, 0xc4, 0x49, 0x22, 0xe2, 0x76, 
    0x97, 0xdc, 0x76, 0x63, 0x94, 0x46, 0x02, 0x05, 
    0x9c, 0x02, 0x84, 0x51, 0x7c, 0xa8, 0x4f, 0xdc
  },
  // .DigUid = {"53A93C10", "89011816"},
  .DigUid = {
    {
      0xf6, 0x1b, 0x8d, 0x45, 0x17, 0x3f, 0xb2, 0x1d, 
      0x22, 0x14, 0x5f, 0xd6, 0x59, 0x85, 0xa8, 0xbf, 
      0xc9, 0x3e, 0x17, 0x1e, 0xfe, 0xb1, 0x80, 0xd9, 
      0x06, 0x3c, 0xda, 0x34, 0x4e, 0x42, 0x37, 0xf7
    },
    {
      0xdd, 0xfd, 0x0e, 0xc9, 0xb7, 0xf1, 0x70, 0xea, 
      0x08, 0xff, 0xa8, 0x50, 0x71, 0x3c, 0xc3, 0x23, 
      0xbb, 0x65, 0x1b, 0xe4, 0xda, 0x8c, 0x56, 0xd8, 
      0x5c, 0x92, 0xb4, 0x93, 0xac, 0xc3, 0xfb, 0x2c
    }
  },
  // .DigPasscode = "BC144785"
  .DigPasscode = {
    0x00, 0x84, 0xe7, 0x67, 0x34, 0xa8, 0x61, 0x10,
    0x75, 0xd0, 0x99, 0x7c, 0xb2, 0x48, 0xb8, 0x1a, 
    0x64, 0x0c, 0xf0, 0x81, 0x97, 0x50, 0x34, 0x62, 
    0x43, 0x7f, 0x24, 0x17, 0xdb, 0xf0, 0x42, 0x18
  }
};
// /* SHA-256                                                                      */
// data_auth dataAuth = {
//   .DigEmpty = {
//     0x66, 0x68, 0x7a, 0xad, 0xf8, 0x62, 0xbd, 0x77, 
//     0x6c, 0x8f, 0xc1, 0x8b, 0x8e, 0x9f, 0x8e, 0x20, 
//     0x08, 0x97, 0x14, 0x85, 0x6e, 0xe2, 0x33, 0xb3, 
//     0x90, 0x2a, 0x59, 0x1d, 0x0d, 0x5f, 0x29, 0x25
//   },
//   // .DigUid = {"53A93C10", "89011816"},
//   .DigUid = {
//     {
//       // only this one is correct
//       0xc4, 0x4e, 0x97, 0xc1, 0xf2, 0xbd, 0x9d, 0x67, 
//       0xc2, 0x99, 0xe4, 0xa5, 0x6c, 0x46, 0x90, 0x84, 
//       0x7b, 0x21, 0x04, 0x20, 0x95, 0xd2, 0x8e, 0x01, 
//       0xef, 0x57, 0x28, 0x2a, 0xe0, 0x68, 0x72, 0x76
//     },
//     {
//       0xdd, 0xfd, 0x0e, 0xc9, 0xb7, 0xf1, 0x70, 0xea, 
//       0x08, 0xff, 0xa8, 0x50, 0x71, 0x3c, 0xc3, 0x23, 
//       0xbb, 0x65, 0x1b, 0xe4, 0xda, 0x8c, 0x56, 0xd8, 
//       0x5c, 0x92, 0xb4, 0x93, 0xac, 0xc3, 0xfb, 0x2c
//     }
//   },
//   // .DigPasscode = "BC144785"
//   .DigPasscode = {
//     0x00, 0x84, 0xe7, 0x67, 0x34, 0xa8, 0x61, 0x10,
//     0x75, 0xd0, 0x99, 0x7c, 0xb2, 0x48, 0xb8, 0x1a, 
//     0x64, 0x0c, 0xf0, 0x81, 0x97, 0x50, 0x34, 0x62, 
//     0x43, 0x7f, 0x24, 0x17, 0xdb, 0xf0, 0x42, 0x18
//   }
// };
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
  memcpy(&mfrcKeypadReadings, incomingData, sizeof(mfrcKeypadReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  memcpy(dig0, mfrcKeypadReadings.Dig, sizeof(dig0));
}
/* End of ESP-NOW Functions                                                     */
/********************************************************************************/

/********************************************************************************/
/*Start of main functions                                                       */
/********************************************************************************/
void setup() {
  Serial.begin(115200);

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
}

void loop() {
  /******************************************************************************/
  /* Start of Server System (Data Authorization)                                */
  /******************************************************************************/
  int8_t found = -1;
  if (memcmp(dataAuth.DigEmpty, dig0, sizeof(dig0)) == 0) {
    // found = -1
    // Do nothing
  }
  else {
    if (memcmp(dataAuth.DigPasscode, dig0, sizeof(dig0)) == 0) {
      // Data is an authenticated passcode
      found = 1;
    }
    else {
      for (uint8_t i = 0; i < MAX; i++) {
        if (memcmp(dataAuth.DigUid[i], dig0, sizeof(dig0)) == 0) {
          // Data is an authenticated UID
          found = 1;
          break;
        }

        // Data cannot be verified
        found = 0;
      }
    }
  }
  Serial.print("Digest 0 (HEX): ");
  for (int i = 0; i < 32; i++) {
    Serial.print("0x");
    if (dig0[i] < 0x10) {
      Serial.print("0"); // Add a leading zero for single-digit hex values
    }
    Serial.print(dig0[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Digest 0 (Decimal): ");
  for (int i = 0; i < 32; i++) {
    Serial.print(dig0[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("DigEmpty: ");
  for (int i = 0; i < 32; i++) {
    Serial.print("0x");
    if (dataAuth.DigEmpty[i] < 0x10) {
      Serial.print("0"); // Add a leading zero for single-digit hex values
    }
    Serial.print(dataAuth.DigEmpty[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  isGivenAccess = found;
  Serial.print("Access Status/found: ");
  Serial.println(isGivenAccess);

  /* End of Server System                                                       */
  /******************************************************************************/
  
  /******************************************************************************/
  /* Start of ESP-NOW System                                                    */
  /******************************************************************************/

  /*Set up values to send                                                       */
  accessStatus.Access = isGivenAccess;

  /* Send message via ESP-NOW                                                   */
  esp_err_t result = esp_now_send(bcAddress, (uint8_t *) &accessStatus, 
                                  sizeof(accessStatus));
  Serial.println((result == ESP_OK) ? 
    "Sent with success" : 
    "Error sending the data"
  );
 
  /* End of ESP-NOW System                                                      */
  /******************************************************************************/
}

/* End of main functions                                                        */
/********************************************************************************/