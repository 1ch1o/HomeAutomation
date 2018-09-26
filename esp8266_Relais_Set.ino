#include <ESP8266WiFi.h>
#include <PubSubClient.h>

 byte relON[] = {0xA0, 0x01, 0x01, 0xA2}; //Hex command to send to serial to open relay 
 byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1}; //Hex command to send to serial to close relay 

#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wifi: SSID and password
const char* WIFI_SSID = "YourWIFI";
const char* WIFI_PASSWORD = "YourWIFIpassword";

// MQTT: ID, server IP, port, username and password
const /*PROGMEM*/ char* MQTT_CLIENT_ID = "giveUniqueNameForEachModule";
const /*PROGMEM*/ char* MQTT_SERVER_IP = "xxx.xxx.x.xxx";
const /*PROGMEM*/ uint16_t MQTT_SERVER_PORT = 1883;
const /*PROGMEM*/ char* MQTT_USER = "mqttuser";
const /*PROGMEM*/ char* MQTT_PASSWORD = "mqttpassword";

// MQTT: topic
const /*PROGMEM*/ char* MQTT_RELAY_TOPIC = "Edit here";

// MQTT: topic
const /*PROGMEM*/ char* MQTT_RELAY_STATUS_TOPIC = "Edit here";

// default payload
const /*PROGMEM*/ char* RELAY_ON = "ON";
const /*PROGMEM*/ char* RELAY_OFF = "OFF";

String s_RELAY_ON = "ON";
String s_RELAY_OFF = "OFF";


WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  Serial.print("Message arrived [");  
  Serial.print(p_topic);
  Serial.print("} ");
  String payload = "";
  for (int i=0;i<p_length;i++) {
    Serial.print((char)p_payload[i]);
    payload = payload + (char)p_payload[i];
  }
  Serial.println();
  Serial.println("Payload: " + payload);
  Serial.println();
  if ((strcmp(p_topic, MQTT_RELAY_TOPIC) == 0)&&(payload == s_RELAY_ON )) {
    Serial.write(relON, sizeof(relON)); // turns the relay on
    client.publish(MQTT_RELAY_STATUS_TOPIC, RELAY_ON);
    Serial.println("Relay switched ON");
  }
  if ((strcmp(p_topic, MQTT_RELAY_TOPIC) == 0)&&(payload == s_RELAY_OFF)) {
    Serial.write(relOFF, sizeof(relOFF)); // turns the relay off
    client.publish(MQTT_RELAY_STATUS_TOPIC, RELAY_OFF);
    Serial.println("Relay switched OFF");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
      client.subscribe(MQTT_RELAY_TOPIC);
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // init the serial
  //Serial.begin(115200);
  Serial.begin(9600);

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.println("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
