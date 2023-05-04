/* Run by server                                                                */

/* Preprocessors                                                                */
/* #include <lib>                                                               */
#include <WiFi.h>
#include <esp_now.h>
/********************************************************************************/
/* #include "header.h"                                                          */
#include "espnow.h"
/********************************************************************************/
/* #define CONST                                                                */
#define MAX 30

/* Variable declarations                                                        */
String passCode;
String UID;
String decryptedData;    // Variable to store data to be received
int8_t isGivenAccess;   // Variable to store data to be sent (access status)
String success;        // Variable to store if sending data was successful
String data;          // Variable to store data for authorization
/********************************************************************************/

/* Typedef declarations                                                         */
/* Structure of data to receive from the reader                                 */
typedef struct {
  String Data;
} receive_data;
/* Strcuture of data to send to the reader                                      */
typedef struct {
  int8_t Access;
} send_data;
typedef struct {
  String Uid[MAX];
  String Passcode;
} data_auth;
/********************************************************************************/
send_data accessStatus;                 // A data to send to reader
receive_data mfrcKeypadReadings;       // A data to receive from reader
/********************************************************************************/

/* Variable init                                                                */
data_auth dataAuth = {
  .Uid = {"53A93C10", "89011816"},
  .Passcode = "BC144781"
};
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
  decryptedData = mfrcKeypadReadings.Data;
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
  /* Start of data decryption                                                   */
  /******************************************************************************/
  // Data decryption

  /* End of data decryption                                                     */
  /******************************************************************************/

  /******************************************************************************/
  /* Start of Server System (Data Authorization)                                */
  /******************************************************************************/
  int8_t found = -1;
  bool iterate = true;
  if (decryptedData == "") {
    // found = -1
    // Do nothing
  }
  else {
    if (decryptedData == dataAuth.Passcode) {
      // Data is an authenticated passcode
      found = 1;
    }
    else {
      for (int i = 0; i < MAX; i++) {
        if (decryptedData == dataAuth.Uid[i]) {
          // Data is an authenticated UID
          found = 1;
          break;
        }
        found = 0;
      }
    }
  }

  isGivenAccess = found;

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