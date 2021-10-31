Word Clock based on Wemos D1 mini / ESP8266
===========================================

This is my first project / fork on github.
I'm adjusting the original project to my needs. Will update the code soon.
Note: by no means am i a c+ / arduino expert. This is just an hobby project for me. I created an PCB design trough easyeda to work with the code.
Things that i've changed from the original:
  - Dutch hours / readout
  - Removed ambilight futures
  - removed english & german language for readability
  - added nighttime monday till sunday
  - added option 'summertime' in the HTTP site, and removed automaticly by date
  - print IP adres for HTTP site when startingup -> for when MDNS doesn't work on router
  - nighttime and other settings will display the current value in the hmi.
    - Issue: When a new value is entered the page needs a manual refresh before the new value is displayed
   - Getting the clock to count offline
    - This kinda works, when we don't get a NTP response, i add the sync interval time to the old time
    - When we don't have wifi at all, i skip trying to get a random server by the pool, because this takes to long (+-8 sec for each request) 
 
 Wishlist:
  - Something fun on 12-31 00:00 like a small fireworks show
  - clock to continue working offline
  - update website after a new value is entered
  - Nicer looking page | or maybe create an android app for control

----------------------
Required board support
----------------------
add http://arduino.esp8266.com/versions/2.4.0/package_esp8266com_index.json as additional board manager URL

Required libraries
------------------
- Adafruid NeoPixel
- Time
- ArduinoJson
- WiFiManager
- https://github.com/ratkins/RGBConverter

Features
--------

- LEDs
  - Control of WS2812B LED strip
  - Ambilight
  - Color in HSV
  - Brightness adjusted by LDR

- Time
  - Obtained by NTP
  - Wifimanager used to connect to Wifi

- Webserver:
  - All parameters can be set with simple web server
  - Access to web server: Type wordclock.local in your browser

- Night-time:
  - Word clock LEDs can be switched off at night
  - Start and end time can be set manually

- Others
  - Language: German/English
  - Display of current date

Wiring
------

- LED IDs:
  - Rows from top to bottom:
  - 2-12
  - 24-14
  - 25-35
  - 46-36
  - 47-57
  - 68-58
  - 69-79
  - 90-80
  - 91-101
  - 113-103
  - LEDs for single minutes: 1 - 13 - 102 - 114
  - All remaining LEDs belong to ambilight

- LDR:
  - 3.3V —— LDR —— A0 —— 10k —— GND
