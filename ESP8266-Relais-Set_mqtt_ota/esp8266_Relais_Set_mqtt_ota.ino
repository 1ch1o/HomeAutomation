#include "config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//checks if DEBUG_SERIAL is defined in config. If not, Debug lines won't be compiled
#if defined(DEBUG_SERIAL)
#define     DEBUG_PRINT(x)    Serial.print(x)
#define     DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define     DEBUG_PRINT(x)
#define     DEBUG_PRINTLN(x)
#endif

byte relON[] = {0xA0, 0x01, 0x01, 0xA2}; //Hex command to send to serial to open relay 
byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1}; //Hex command to send to serial to close relay 

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////
/*
 * Function called do setup WiFi
 */
void setupWiFi() {
  // init the WiFi connection
  DEBUG_PRINTLN();
  DEBUG_PRINT(F("INFO: WiFi Connecting to: "));
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
  DEBUG_PRINT(F("INFO: WiFi connected, IP: "));
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
char MQTT_RELAY_COMMAND_TOPIC[sizeof(MQTT_RELAY_COMMAND_TOPIC_TEMPLATE)-1 + sizeof(MQTT_LOCATION)-1] = {0};
char MQTT_RELAY_STATE_TOPIC[sizeof(MQTT_RELAY_STATE_TOPIC_TEMPLATE)-1 + sizeof(MQTT_LOCATION)-1] = {0};
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

  DEBUG_PRINT(F("INFO: MQTT MQTT_LOCATION: "));
  DEBUG_PRINTLN(MQTT_LOCATION);
  
  ///Availability-Topic  
  sprintf(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABILITY_TOPIC_TEMPLATE, MQTT_LOCATION);

  DEBUG_PRINT(F("INFO: MQTT availability topic: "));
  DEBUG_PRINTLN(MQTT_AVAILABILITY_TOPIC);

  ///Command-Topic
  sprintf(MQTT_RELAY_COMMAND_TOPIC, MQTT_RELAY_COMMAND_TOPIC_TEMPLATE, MQTT_LOCATION);

  DEBUG_PRINT(F("INFO: MQTT relay_command topic: "));
  DEBUG_PRINTLN(MQTT_RELAY_COMMAND_TOPIC); 

  ////State-Topic
  sprintf(MQTT_RELAY_STATE_TOPIC, MQTT_RELAY_STATE_TOPIC_TEMPLATE, MQTT_LOCATION);

  DEBUG_PRINT(F("INFO: MQTT relay_state topic: "));
  DEBUG_PRINTLN(MQTT_RELAY_STATE_TOPIC); 
  
//// init the MQTT connection
  DEBUG_PRINT(F("INFO: MQTT set Server: "));
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

  //check Relay-Topic
  if (strcmp(p_topic, MQTT_RELAY_COMMAND_TOPIC) == 0) {
  //If p_topic = Command_Topic then switch relay to on and publish new state
    if (payload == MQTT_PAYLOAD_ON) {
      DEBUG_PRINT(F("INFO: SERIAL following message wrote to serial: "));
      Serial.write(relON, sizeof(relON)); // turns the relay on
      DEBUG_PRINTLN();
      DEBUG_PRINTLN(F("INFO: RELAY switched to ON"));
      publishToMQTT(MQTT_RELAY_STATE_TOPIC, MQTT_PAYLOAD_ON);
    }
    
    //If p_topic = Command_Topic then switch relay to off and publish new state
    else if (payload == MQTT_PAYLOAD_OFF) {
      DEBUG_PRINT(F("INFO: SERIAL following message wrote to serial: "));
      Serial.write(relOFF, sizeof(relOFF)); // turns the relay off
      DEBUG_PRINTLN();
      DEBUG_PRINTLN(F("INFO: RELAY switched to OFF"));
      publishToMQTT(MQTT_RELAY_STATE_TOPIC, MQTT_PAYLOAD_OFF);
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
    DEBUG_PRINTLN();
    DEBUG_PRINTLN(F("ERROR: MQTT message not published, either connection lost, or message too large. Topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(" , payload: "));
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
        subscribeTopic(MQTT_RELAY_COMMAND_TOPIC);
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
  DEBUG_PRINTLN(F("INFO: OTA setting up"));  
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  DEBUG_PRINT(F("INFO: OTA HOSTNAME: "));
  DEBUG_PRINTLN(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  DEBUG_PRINT(F("INFO: OTA PORT: "));
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
    DEBUG_PRINT(F("INFO: OTA start updating "));
    DEBUG_PRINTLN(type);
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN(F("INFO: OTA update successfull"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINT(F("INFO: Progress: "));
    DEBUG_PRINTLN(progress / (total / 100));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      DEBUG_PRINTLN(F("ERROR: OTA End Failed"));
    }
  });
  ArduinoOTA.begin();
}  
///////////////////////////////////////////////////////////////////////////
//   SETUP() & LOOP()
///////////////////////////////////////////////////////////////////////////
void setup() {
  // init the serial
  Serial.begin(9600);
  
  setupWiFi();
  setupMQTT();
  setupOTA();
  
}

void loop() {
  ArduinoOTA.handle();
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
}
