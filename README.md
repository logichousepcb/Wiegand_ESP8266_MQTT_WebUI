# Wiegand_ESP8266_MQTT_WebUI
This is a simple stand alone Wiegand keypad/RFID relay controller....let's add some features

This is a simple device that uses a Wemos D1 Mini and a Wiegand RFID reader with keypad.  Using a simple web based user interface you can connect to your network and set up user codes.


Command:

9 9 9 9 1 ENT - Will turn on the Wed user interface to configure the device

9 9 9 9 0 ENT - Will turn off the Web user interface

ESC - will clear the code buffer

ENT - sends code after pressing keys (the max number of digits in a code is 11)


# Defaults:

1 2 3 4 ENT - default admin code **this can be changed by the web ui but the admin code is always active


# Connections:

RELAY GPIO14 - D5 on Womos

D0    GPIO12 - White wire - D7 on Womos 

D1    GPIO13 - Green wire - D6 on Womos

LED   GPIO5  - Blue wire - D1 on Womos

BEEP  GPIO4  - Yellow wire - D2 on Womos

** Please note these are GPIO pports and not the Wemos pin labels


# LED Indications:
 
4 flashes = relay triggered

5 flashes = WEB UI activated

2 flashes = WEB UI deactivated

fast flashing while connecting to network

# Serial data:

Output:

SSID-############ - returns name of network SSID trying to connect to 

WEBUI-ON  - returns is the web user interface is active

WEBUI-OFF - returns if the web user interface is turned off

UIIP-__.__.__.__ returs the IP for the UI

RECV-__________ returns what device recieved via serial communication 

CODE-###########-nn-v  ## is the code entered (up to 11 digits)
                       nn is the user number
                       v  G (granted if the code is code and user has access on) 
                          D (denied due to user access turned off) 
                          B (bad code entered and will use user 99)
                          C (command code entered)

ADD-usr#-%%%%%% reply when user # is added with code %%%%%%%

ERROR-1 Cannot read configuration

ERROR-2 Cannot write configuration

ERROR There was an error with the config web ui interface

Input:

r - trigger dealy to open door

G - green light on

g - green light off

b - beep

P-##-code  where ## is the user 01-10 and code is the access code up to 11 digits

![GitHub Logo](https://github.com/logichousepcb/Wiegand_ESP8266_MQTT_WebUI/blob/main/Wiegand_ESP8266_MQTT_WebUI.JPG)

I have used a library from GerLech for the web UI.
https://github.com/GerLech/WebConfig


