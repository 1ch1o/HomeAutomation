///////////////////////////////////////////////////////////////////////////
//  CONFIGURATION - SOFTWARE
///////////////////////////////////////////////////////////////////////////

// Debug output
#define DEBUG_SERIAL

// Location of Module
////-> out of this the MQTT_TOPIC will be generated: = <ZONE>/<FLOOR>/<LOCATION>/<MQTT_CLIENT_ID>/....
////-> for example: MQTT_RELAY_STATE_TOPIC = home/floor_0/bedroom/TESTID/relay/state
#define ZONE "home"
#define FLOOR "floor_0"
#define LOCATION "bedroom"

// Wi-Fi credentials
#define WIFI_SSID     "Example-WiFi"
#define WIFI_PASSWORD "Example-Pswd"

// MQTT
#define MQTT_USERNAME     "Example-Usr"
#define MQTT_PASSWORD     "Example-pswd"
#define MQTT_SERVER_IP    "xxx.xxx.x.x"
#define MQTT_SERVER_PORT  1883

////-> if you want to set CLIENT_ID manually define it here.
////-> otherwise comment it and ESP's MAC-Adress will be used.
#define MQTT_CLIENT_ID "TESTID"

//OTA
#define OTA_HOSTNAME      MQTT_CLIENT_ID
#define OTA_PASSWORD      ""
#define OTA_PORT          8266



#define MQTT_CONNECTION_TIMEOUT 5000 // [ms]

// Templates, don't change
#define MQTT_LOCATION_TEMPLATE "%s/%s/%s/%s"
#define MQTT_AVAILABILITY_TOPIC_TEMPLATE  "%s/availability" 
#define MQTT_RELAY_COMMAND_TOPIC_TEMPLATE "%s/relay/command"
#define MQTT_RELAY_STATE_TOPIC_TEMPLATE "%s/relay/state"

//Paploads
#define MQTT_PAYLOAD_ON   "ON"
#define MQTT_PAYLOAD_OFF  "OFF"

#define MQTT_PAYLOAD_AVAILABLE    "online"
#define MQTT_PAYLOAD_UNAVAILABLE  "offline"
