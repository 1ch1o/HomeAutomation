# ESP8266 Relais-Set MQTT

A simple example how to use a Relais-Set with the ESP8266.\
The code mainly uses code from [mertenats' Open-Home-Automation repository](https://github.com/mertenats/Open-Home-Automation)\
check it out, there are a lot more mqtt-setups for HomeAssistant.

![alt text](https://ae01.alicdn.com/kf/HTB1I8deby6guuRjy0Fmq6y0DXXaR/ESP8266-5-v-WiFi-relais-modul-Dinge-smart-home-fernbedienung-schalter-telefon-APP.jpg_640x640.jpg)
#### You Need:
  * ESP8266 RELAY as shown
  * ##### Either:
    * FTDI232 (for connection between PC-USB and UART)
    * [FTDI-Breadboard Adapter](https://www.tindie.com/products/FemtoCow/esp8266-ftdi-and-breadboard-adapter-with-33v-reg/)\
    NOTE: you have to connect GPIO-0 and GND, and after that press the RST-Button or reattach USB-cable to get into Flash-Mode\
    (I soldered an adapter myself and don`t use this, but i think this will do the job)\
  * ##### Or:
    * [This Adapter](https://www.amazon.de/gp/product/B077Z4L8DD/ref=crt_ewc_title_dp_3?ie=UTF8&psc=1&smid=AL7OCEVQ2O38D)
  * Arduino IDE setup for ESP8266:\
    add a board-manager-URL to your IDE as shown [here](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/).\
    I suggest to look at this [instructable](https://www.instructables.com/id/ESP8266-Programming-Using-FTDI-and-Arduino-IDE/).\
    Befor Flashing check if FlashMode is set to DOUT.
    
    
 My Upload Settings in the Arduino IDE are these:
 NOTE: You maybe have to change the COM-Port.
 ![alt_text](https://github.com/1ch1o/HomeAutomation/blob/master/ESP8266_Relais-Set_mqtt/UploadSettings.png)
