///////////////////////////////////////////////////////////////////////////
//  CONFIGURATION - SOFTWARE
///////////////////////////////////////////////////////////////////////////

// Debug output
#define DEBUG_SERIAL

// Location of Module
////-> out of this the MQTT_TOPIC will be generated: = <ZONE>/<FLOOR>/<LOCATION>/<MQTT_CLIENT_ID>/....
////-> for example: MQTT_MAGNET_STATE_TOPIC = home/floor_0/bedroom/TESTID/magnet
#define ZONE "home"
#define FLOOR "floor_0"
#define LOCATION "bedroom"

// Wi-Fi credentials
#define WIFI_SSID     "YOURWIFI"
#define WIFI_PASSWORD "YOURWIFIPSWD"

// MQTT
#define MQTT_USERNAME     "mqttuser"
#define MQTT_PASSWORD     "mqttpswd"
#define MQTT_SERVER_IP    "xxx.xxx.x.x"
#define MQTT_SERVER_PORT  1883

////-> if you want to set CLIENT_ID manually define it here.
////-> otherwise comment it and ESP's MAC-Adress will be used.
#define MQTT_CLIENT_ID "TESTID"

#define MQTT_CONNECTION_TIMEOUT 5000 // [ms]

// Templates, don't change
#define MQTT_LOCATION_TEMPLATE "%s/%s/%s/%s"
#define MQTT_AVAILABILITY_TOPIC_TEMPLATE  "%s/availability" 
#define MQTT_COMMAND_TOPIC_TEMPLATE "%s/command"

//Here the second %s will be replaced by the friendly name of your BLETrackedDevices
#define MQTT_MAGNET_STATE_TOPIC_TEMPLATE "%s/%s/magnet"
#define MQTT_BAT_VOLT_TOPIC_TEMPLATE "%s/%s/batvolt"

//Paploads
#define MQTT_PAYLOAD_ON   "ON"
#define MQTT_PAYLOAD_OFF  "OFF"
#define MQTT_PAYLOAD_ERROR "ERROR"

#define MQTT_PAYLOAD_AVAILABLE    "online"
#define MQTT_PAYLOAD_UNAVAILABLE  "offline"

//Bluetooth
#define BLE_SCANNING_PERIOD 30 //in s
#define BLE_SCANNING_INTERVAL 10 //in s

#define BLE_SENSOR_REED 1
#define BLE_SENSOR_OTHR 0 

//Change NB_OF_BLE_TRACKED_DEVICES depending on how much Devices you have got in BLETrackedDevices
#define NB_OF_BLE_TRACKED_DEVICES 3

//typestructure of BLETrackedDevices:
/*  String  address;
    bool    isDiscovered;
    long    lastDiscovery;
    int     magnet;
    char    battery[4 + 1];
    char    manufacturerData[26 + 1]; // last 1 is 0-termination
    int     sensorType;
    String  location;
*/
//Change this depending on your BLE-Devices
BLETrackedDevice BLETrackedDevices[NB_OF_BLE_TRACKED_DEVICES] = {
  {"00:1a:22:0b:46:a8", false, 0, 0, {0}, {0}, BLE_SENSOR_OTHR, "Unknown Device1"},
  {"ee:23:a4:1f:77:77", false, 0, 0, {0}, {0}, BLE_SENSOR_REED, "Window1"},
  {"00:1a:22:0b:05:0a", false, 0, 0, {0}, {0}, BLE_SENSOR_OTHR, "Unknown Device2"}
};

//OTA
#define OTA_HOSTNAME      MQTT_CLIENT_ID
#define OTA_PASSWORD      ""
#define OTA_PORT          8266
