# Wiegand_ESP8266_MQTT_WebUI
This is a simple stand alone Wiegand keypad/RFID relay controller....let's add some features

This is a simple device that uses a Wemos D1 Mini and a Wiegand RFID reader with keypad.  Using a simple web based user interface you can connect to your network and set up user codes.


Commands:

9 9 9 9 1 ENT - Will turn on the Wed user interface to configure the device

9 9 9 9 0 ENT - Will turn off the Web user interface

ESC - will clear the code buffer


Defaults:

1 2 3 4 ENT - default admin code

RELAY GPIO14
D0    GPIO12 - White wire
D1    GPIO13 - Green wire
LED   GPIO5  - Blue wire
BEEP  GPIO4  - Yellow wire
** Please note these are GPIO pports and not the Wemos pin labels

I have used a library from GerLech for the web UI.
https://github.com/GerLech/WebConfig
