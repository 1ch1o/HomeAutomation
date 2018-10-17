# ESP32 BLE MQTT Bridge with OTA

![alt text](https://github.com/1ch1o/HomeAutomation/blob/master/ESP32_ble_mqtt_bridge/ESP32-mqtt-ble-bridge.png)

I made this script to replace the gateway for the Window-Reed BLE Modules in this [instructable](https://www.instructables.com/id/LoRa-Tooth-Small-Wireless-Sensors/)

To use this you either have to comment out the Encryption-Part from this [MBED Sensor](https://os.mbed.com/users/electronichamsters/code/BLE_Sensor/) (line 677 to 680 in the main.cpp) or you use the already compiled .hex file I uploaded to flash your nrf51822.\
If you flash your nrf51822 with a raspi 3b+ have a look at my comment in the instructable.


NOTE: There is no safety precaution made in this script, so don't use this for security tasks. This is just to send the state of the reed switch to your mqtt broker.
-> The way this Gateway works your Sensor could easy be copied. So wrong BLE-messages could be send to your Gateway.



