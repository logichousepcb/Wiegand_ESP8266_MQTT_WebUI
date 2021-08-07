# Wiegand_ESP8266_MQTT_WebUI
This is a simple stand alone Wiegand keypad/RFID relay controller....let's add some features

This is a simple device that uses a Wemos D1 Mini and a Wiegand RFID reader with keypad.  Using a simple web based user interface you can connect to your network and set up user codes.


Commands:

9 9 9 9 1 ENT - Will turn on the Wed user interface to configure the device

9 9 9 9 0 ENT - Will turn off the Web user interface

ESC - will clear the code buffer

ENT - sends code after pressing keys (the max number of digits in a code is 11)


Defaults:

1 2 3 4 ENT - default admin code **this can be changed by the web ui but the admin code is always active


RELAY GPIO14 - D5 on Womos
D0    GPIO12 - White wire - D7 on Womos
D1    GPIO13 - Green wire - D6 on Womos
LED   GPIO5  - Blue wire - D1 on Womos
BEEP  GPIO4  - Yellow wire - D2 on Womos
** Please note these are GPIO pports and not the Wemos pin labels


LED Indications:

4 flashes = relay triggered
5 flashes = WEB UI activated
2 flashes = WEB UI deactivated

I have used a library from GerLech for the web UI.
https://github.com/GerLech/WebConfig
