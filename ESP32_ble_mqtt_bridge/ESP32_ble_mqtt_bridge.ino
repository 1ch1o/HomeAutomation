typedef struct {
  String  address;
  bool    isDiscovered;
  long    lastDiscovery;
  int     magnet;
  char    battery[4 + 1];
  char    manufacturerData[26 + 1]; // last 1 is 0-termination
  int     sensorType;
  String  location;
} BLETrackedDevice;


#include "config.h"
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#if defined(DEBUG_SERIAL)
#define     DEBUG_PRINT(x)    Serial.print(x)
#define     DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define     DEBUG_PRINT(x)
#define     DEBUG_PRINTLN(x)
#endif

BLEScan*      pBLEScan;
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);

long lastScan = 0;
bool printed = false;

///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////
/*
 * Function called do setup WiFi
 */
void setupWiFi() {
  // init the WiFi connection
  DEBUG_PRINT(F("INFO: WiFi Connecting to:        "));
  DEBUG_PRINTLN(WIFI_SSID);
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  randomSeed(micros());
  
  while (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINT(F("."));
    delay(500);
  }

  DEBUG_PRINTLN();
  DEBUG_PRINTLN(F("INFO: WiFi connected"));
  DEBUG_PRINT(F("INFO: WiFi IP:                   "));
  DEBUG_PRINTLN(WiFi.localIP());
}
///////////////////////////////////////////////////////////////////////////
//   MQTT
///////////////////////////////////////////////////////////////////////////
volatile unsigned long lastMQTTConnection = 0;
#ifndef MQTT_CLIENT_ID
 char MQTT_CLIENT_ID[18] = {0};
#endif
char MQTT_LOCATION[sizeof(MQTT_LOCATION_TEMPLATE)-1 + sizeof(ZONE)-1 + sizeof(FLOOR)-1 + sizeof(LOCATION)-1 + sizeof(MQTT_CLIENT_ID)-1] = {0};
char MQTT_AVAILABILITY_TOPIC[sizeof(MQTT_AVAILABILITY_TOPIC_TEMPLATE)-1 + sizeof(MQTT_LOCATION)-1] = {0};
//have to test the following:
char MQTT_COMMAND_TOPIC[sizeof(MQTT_COMMAND_TOPIC_TEMPLATE)-1 + sizeof(MQTT_LOCATION)-1] = {0};
/*
 * Function called do setup MQTT
 */
void setupMQTT(){
  //MQTT_CLIENT_ID
  #ifndef MQTT_CLIENT_ID
  uint8_t MAC_array[6];
   WiFi.macAddress(MAC_array);
    for (int i = 0; i < sizeof(MAC_array); ++i){
     sprintf(MQTT_CLIENT_ID,"%s%02x:",MQTT_CLIENT_ID,MAC_array[i]);
   }
  #endif 
  //MQTT_LOCATION
  sprintf(MQTT_LOCATION, MQTT_LOCATION_TEMPLATE, ZONE, FLOOR, LOCATION, MQTT_CLIENT_ID);

  DEBUG_PRINT(F("INFO: MQTT LOCATION:             "));
  DEBUG_PRINTLN(MQTT_LOCATION);
  
  ///Availability-Topic  
  sprintf(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABILITY_TOPIC_TEMPLATE, MQTT_LOCATION);

  DEBUG_PRINT(F("INFO: MQTT AVAILABILITY_TOPIC:   "));
  DEBUG_PRINTLN(MQTT_AVAILABILITY_TOPIC);

  ///Command-Topic
  sprintf(MQTT_COMMAND_TOPIC, MQTT_COMMAND_TOPIC_TEMPLATE, MQTT_LOCATION);

  DEBUG_PRINT(F("INFO: MQTT COMMAND_TOPIC:  "));
  DEBUG_PRINTLN(MQTT_COMMAND_TOPIC); 

  
//// init the MQTT connection
  DEBUG_PRINT(F("INFO: MQTT SERVER_IP:            "));
  DEBUG_PRINT(MQTT_SERVER_IP);
  DEBUG_PRINT(F(":"));
  DEBUG_PRINTLN(MQTT_SERVER_PORT);
  mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  mqttClient.setCallback(callback);
}

/*
  Function called to publish to a MQTT topic with the given payload
*/

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  DEBUG_PRINT(F("INFO: MQTT message arrived ["));  
  DEBUG_PRINT(p_topic);
  DEBUG_PRINT(F("] "));
  String payload = "";
  for (int i=0;i<p_length;i++) {
    payload = payload + (char)p_payload[i];
  } 
  DEBUG_PRINT(F("payload: "));
  DEBUG_PRINTLN(payload);
  checkTopicsAndPayloadsForActions(p_topic, payload);
}
/*
 * Function to check for Actions on recieved Topics and Payloads
 */
void checkTopicsAndPayloadsForActions(char*p_topic, String payload){
  DEBUG_PRINTLN(F("INFO: MQTT checking for actions to do"));

  //check Command-Topic
  if (strcmp(p_topic, MQTT_COMMAND_TOPIC) == 0) {
  //If p_topic = Command_Topic then 
    if (payload == MQTT_PAYLOAD_ON) {     
    } 
    //If p_topic = Command_Topic then 
    else if (payload == MQTT_PAYLOAD_OFF) {     
    }
    else {
      DEBUG_PRINTLN(F("INFO: MQTT nothing to do"));
    }
  }
  //If nothing to do
  else {
    DEBUG_PRINTLN(F("INFO: MQTT nothing to do"));
  }
}
/*
  Function called to publish to a MQTT topic with the given payload
*/
void publishToMQTT(char* p_topic, char* p_payload) {
  if (mqttClient.publish(p_topic, p_payload, true)) {
    DEBUG_PRINT(F("INFO: MQTT message published ["));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F("] payload: "));
    DEBUG_PRINTLN(p_payload);
  } else {
    DEBUG_PRINT(F("ERROR: MQTT message not published, either connection lost, or message too large. ["));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F("] payload: "));
    DEBUG_PRINTLN(p_payload);
  }
}
/*
  Function called to connect/reconnect to the MQTT broker and subscribe to Topic
*/
void connectToMQTT() {
  if (!mqttClient.connected()) {
    if (lastMQTTConnection < millis()) {
      if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_AVAILABILITY_TOPIC, 0, 1, MQTT_PAYLOAD_UNAVAILABLE)) {
        DEBUG_PRINTLN(F("INFO: MQTT client connected to broker"));
        publishToMQTT(MQTT_AVAILABILITY_TOPIC, MQTT_PAYLOAD_AVAILABLE);
        subscribeTopic(MQTT_COMMAND_TOPIC);
      } else {
        DEBUG_PRINTLN(F("ERROR: MQTT connection to broker failed"));
        DEBUG_PRINT(F("INFO: MQTT username: "));
        DEBUG_PRINTLN(MQTT_USERNAME);
        DEBUG_PRINT(F("INFO: MQTT password: "));
        DEBUG_PRINTLN(MQTT_PASSWORD);
        DEBUG_PRINT(F("INFO: MQTT broker IP: "));
        DEBUG_PRINTLN(MQTT_SERVER_IP);
      }
      lastMQTTConnection = millis() + MQTT_CONNECTION_TIMEOUT;
    }
  }
}
/*
  Function called to subscribe different Topics
*/
void subscribeTopic(char* p_topic) {
  if (mqttClient.subscribe(p_topic)){
  DEBUG_PRINT(F("INFO: MQTT subscribed to ["));
  DEBUG_PRINT(p_topic);
  DEBUG_PRINTLN(F("]"));
  } else {
    DEBUG_PRINT(F("ERROR: MQTT could not subscribe to ["));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINTLN(F("]"));
    DEBUG_PRINT(F("ERROR: MQTT either connection lost, or message too large"));
  }
}
///////////////////////////////////////////////////////////////////////////
//   OTA
///////////////////////////////////////////////////////////////////////////
/*
 * Function called do setup OTA
 */
 void setupOTA(){
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  DEBUG_PRINT(F("INFO: OTA  HOSTNAME:             "));
  DEBUG_PRINTLN(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  DEBUG_PRINT(F("INFO: OTA  PORT:                 "));
  DEBUG_PRINTLN(OTA_PORT);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    DEBUG_PRINT(F("INFO: OTA  start updating "));
    DEBUG_PRINTLN(type);
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN(F("INFO: OTA  update successfull"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINT(F("INFO: OTA  upload progress: "));
    DEBUG_PRINTLN(progress / (total / 100));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA  Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA  Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA  Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA  Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA  End Failed"));
    }
  });
  ArduinoOTA.begin();
}  
///////////////////////////////////////////////////////////////////////////
//   BLUETOOTH
///////////////////////////////////////////////////////////////////////////
class MyAdvertisedDeviceCallbacks:
  public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      DEBUG_PRINTLN(F("INFO: BLE  --------------------------------"));
      DEBUG_PRINT(F("INFO: BLE  Advertised Device: "));
      DEBUG_PRINTLN(advertisedDevice.toString().c_str());
      
        //iterate through known devices and check if advertised Device is listed and set them to discovered
        for (int i = 0; i < NB_OF_BLE_TRACKED_DEVICES ; i++){
          if (strcmp(advertisedDevice.getAddress().toString().c_str(), BLETrackedDevices[i].address.c_str()) == 0){  
              long timer = millis();
              if (timer - BLETrackedDevices[i].lastDiscovery > BLE_SCANNING_PERIOD * 1000){
                //save discovery time and set discovered to true
                BLETrackedDevices[i].lastDiscovery = millis();
                BLETrackedDevices[i].isDiscovered = true;
                DEBUG_PRINT(F("INFO: BLE  "));
                DEBUG_PRINT(BLETrackedDevices[i].location);
                DEBUG_PRINTLN(F(" found"));
                
                if (advertisedDevice.haveManufacturerData()) {
                  //save manufacturer data to use for following functions
                  for (int j = 0; j < sizeof(BLETrackedDevices[i].manufacturerData) - 1 ; j++){
                     BLETrackedDevices[i].manufacturerData[j] = advertisedDevice.getManufacturerData().c_str()[j];
                     if (j == sizeof(BLETrackedDevices[i].manufacturerData) - 1){
                       BLETrackedDevices[i].manufacturerData[sizeof(BLETrackedDevices[i].manufacturerData)] = 0; //Null terminating the string
                     }
                  }  
                  DEBUG_PRINT(F("INFO: BLE  "));
                  DEBUG_PRINT(BLETrackedDevices[i].location);
                  DEBUG_PRINT(F(" Size of manufacturer data: "));
                  DEBUG_PRINTLN(sizeof(BLETrackedDevices[i].manufacturerData)-1); //-1 because of 0-termination
                  DEBUG_PRINT(F("INFO: BLE  "));
                  DEBUG_PRINT(BLETrackedDevices[i].location);
                  DEBUG_PRINT(F(" manufacturer data: "));
                  for (int j = 0; j < sizeof(BLETrackedDevices[i].manufacturerData); j++){
                    DEBUG_PRINT(BLETrackedDevices[i].manufacturerData[j]);
                  }
                  DEBUG_PRINTLN();
  
                  //save battery voltage (in manufacturerData[4-7])
                  for (int j=0; j<sizeof(BLETrackedDevices[i].battery) - 1 ; j++){
                     BLETrackedDevices[i].battery[j] = BLETrackedDevices[i].manufacturerData[j+4];
                     if (j == sizeof(BLETrackedDevices[i].battery) - 1){
                        BLETrackedDevices[i].battery[sizeof(BLETrackedDevices[i].battery)] = 0; //Null terminating the string
                     }
                  }
                  DEBUG_PRINT(F("INFO: BLE  "));
                  DEBUG_PRINT(BLETrackedDevices[i].location);
                  DEBUG_PRINT(F(" battery voltage: "));
                  DEBUG_PRINTLN(BLETrackedDevices[i].battery);
  
                  //save magnet state
                    //manufacturerData looks either like this : (periodic) ⸮w⸮3.27"mag/p":1    
                    //Or like this: (status changed) ⸮w⸮3.27"mag":1     
                    //Depends if the Advertising of the Bluetooth-Device is a periodic callback or a status change
                  if (BLETrackedDevices[i].manufacturerData[19] == '0' || BLETrackedDevices[i].manufacturerData[21] == '0'){
                    DEBUG_PRINT(F("INFO: BLE  "));
                    DEBUG_PRINT(BLETrackedDevices[i].location);
                    DEBUG_PRINTLN(F(" magnet state OPEN"));
                    BLETrackedDevices[i].magnet = 0;
                  }
                  else if (BLETrackedDevices[i].manufacturerData[19] == '1' || BLETrackedDevices[i].manufacturerData[21] == '1'){
                    DEBUG_PRINT(F("INFO: BLE  "));
                    DEBUG_PRINT(BLETrackedDevices[i].location);
                    DEBUG_PRINTLN(F(" magnet state CLOSED"));
                    BLETrackedDevices[i].magnet = 1;
                  }
                  else {
                    DEBUG_PRINT(F("ERROR: BLE  "));
                    DEBUG_PRINT(BLETrackedDevices[i].location);
                    DEBUG_PRINTLN(F(" MAGNET ERROR"));
                    BLETrackedDevices[i].magnet = -1;
                  }
                }
              }
              else {
                 DEBUG_PRINT(F("INFO: BLE  "));
                 DEBUG_PRINT(BLETrackedDevices[i].location);
                 DEBUG_PRINTLN(F(" already found in this Scanning Period"));
              }
          }
        }
      }
    
};
/*
 * Function to check if tracked Devices are discovered and publish state to MQTT
 */
void handleDiscoveredBLEDevices(){
  for (int i = 0; i < NB_OF_BLE_TRACKED_DEVICES ; i++){
          if (BLETrackedDevices[i].isDiscovered == true && BLETrackedDevices[i].sensorType == BLE_SENSOR_REED){
            //make mqtt topics
            //get friendly name aka location of listed device
            char MQTT_BLUETOOTH_DEVICE[sizeof(BLETrackedDevices[i].location)] = {0};
            sprintf(MQTT_BLUETOOTH_DEVICE, BLETrackedDevices[i].location.c_str());
            DEBUG_PRINTLN(F("INFO: MQTT --------------------------------"));
            DEBUG_PRINT(F("INFO: MQTT "));
            DEBUG_PRINTLN(BLETrackedDevices[i].location);
                      
            //make magnet state topic
            char MQTT_MAGNET_STATE_TOPIC[sizeof(MQTT_MAGNET_STATE_TOPIC_TEMPLATE)-1 + sizeof(MQTT_LOCATION)-1 + sizeof(MQTT_BLUETOOTH_DEVICE)-1] = {0};     
            sprintf(MQTT_MAGNET_STATE_TOPIC, MQTT_MAGNET_STATE_TOPIC_TEMPLATE,MQTT_LOCATION, MQTT_BLUETOOTH_DEVICE);

            //make battery voltage topic
            char MQTT_BAT_VOLT_TOPIC[sizeof(MQTT_BAT_VOLT_TOPIC_TEMPLATE)-1 + sizeof(MQTT_LOCATION)-1 + sizeof(MQTT_BLUETOOTH_DEVICE)-1] = {0};     
            sprintf(MQTT_BAT_VOLT_TOPIC, MQTT_BAT_VOLT_TOPIC_TEMPLATE, MQTT_LOCATION, MQTT_BLUETOOTH_DEVICE);

            //publish MQTT magnet state
            switch(BLETrackedDevices[i].magnet){
               case 0:
                  publishToMQTT(MQTT_MAGNET_STATE_TOPIC, MQTT_PAYLOAD_OFF);
                  break;
               case 1:
                  publishToMQTT(MQTT_MAGNET_STATE_TOPIC, MQTT_PAYLOAD_ON);
                  break;
               default:
                  publishToMQTT(MQTT_MAGNET_STATE_TOPIC, MQTT_PAYLOAD_ERROR);
                  break;
            }

            //publish MQTT battery voltage
            publishToMQTT(MQTT_BAT_VOLT_TOPIC, BLETrackedDevices[i].battery);

          }
          //reset isDiscovered
          BLETrackedDevices[i].isDiscovered = false;
  }
}
/*
 * Function to setup Bluetooth
 */
void setupBluetooth(){
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}
/*
 * Function to scan Bluetooth
 */
void scanBluetooth(){
  if ((millis() - lastScan)/1000  > BLE_SCANNING_INTERVAL){ 
    DEBUG_PRINTLN(F("INFO: BLE  --------------------------------"));
    DEBUG_PRINT(F("INFO: BLE  Scan-cycle started with Scanning period: "));
    DEBUG_PRINT(BLE_SCANNING_PERIOD);
    DEBUG_PRINTLN(F(" seconds"));
    float timer = millis();
    pBLEScan->start(BLE_SCANNING_PERIOD);
    timer = millis() - timer;
    DEBUG_PRINTLN(F("INFO: BLE  --------------------------------"));
    DEBUG_PRINT(F("INFO: BLE  Scan-cycle finished after "));
    DEBUG_PRINT(timer);
    DEBUG_PRINTLN(F(" ms"));
    DEBUG_PRINTLN(F("INFO: BLE  --------------------------------"));
    lastScan = millis();
    printed = false;
  }
  else {
    if (printed == false){
      DEBUG_PRINTLN(F("_________________________________________________________"));
      DEBUG_PRINT(F("INFO: BLE  waiting "));
      DEBUG_PRINT(BLE_SCANNING_INTERVAL);
      DEBUG_PRINTLN(F(" s for next scanning period")); 
      printed = true;
    }
  }
}
///////////////////////////////////////////////////////////////////////////
//   SETUP
///////////////////////////////////////////////////////////////////////////
void setup() {
#if defined(DEBUG_SERIAL)
  Serial.begin(115200); 
#endif
  DEBUG_PRINTLN();
  DEBUG_PRINTLN(F(".....................SETUP.........................."));
  setupWiFi();
  setupMQTT();
  setupOTA();
  setupBluetooth();
  DEBUG_PRINTLN(F(".....................LOOP.........................."));
}
///////////////////////////////////////////////////////////////////////////
//   LOOP
///////////////////////////////////////////////////////////////////////////
void loop() {
  ArduinoOTA.handle();
  scanBluetooth();
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
  handleDiscoveredBLEDevices();  
  delay(2000);    // had to add because got watchdog triggered with ERROR below:
                  // " Task watchdog got triggered. The following tasks did not reset the watchdog in time: - IDLE (CPU 0) "
                  // also in forums they suggested adding "vTaskDelay(10);" Don't know what pretends error and what's better
}
