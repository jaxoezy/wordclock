// Todo: Englisch/Deutsch
// Temperatur aus Internet lesen und für bestimmte Zeit anzeigen
// Standard-Farbe einstellen (RGB?)
// Einstellungen in EEProm abspeichern
// WifiManager sollte hinzugefügt werden
// IP von Webserver ist nicht konstant -> mit Felix besprechen
// Nachgucken, wie deep sleep funktioniert
// Uhr nachts zu einer bestimmten Zeit ausschalten und morgens wieder einschalten (ESP.deepSleep(10s*1000000))
// -> Zeiten sollten manuell einstellbar sein
// Sollte man den ESP auch für die einzelnen Minuten ausschalten, um maximal Strom zu sparen?
// Er müsste sich jede Minute neu zu dem Netzwerk verbinden ... hmmm ...

// LEDs: 10 hoch, 11 breit
// Wetter: Temperatur
// Sonnig, leicht bewölkt, bewölkt, Regen
// Optional:
// 1. Geo-Koordinaten bestimmen (https://askgeo.com)
// 2. Zeitzone bestimmen (timezonedb.com)
// 3. Wetter abfragen (http://openweathermap.org/)

#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// Network
const char ssid[] = "";  //  your network SSID (name)
const char pass[] = "";       // your network password

// Weather forecast Openweathermap
//const char weather_host[] = "api.openweathermap.org";
//const char city_id[] = "3220838"; // Munich
//const char API_key[] = "";
//const char units[] = "metric"; // °C
//const char no_3h_forecast[] = "1"; // No. consecutive 3h forecast

// Weather forecast Wunderground
const char weather_host[] = "api.wunderground.com";
const char COUNTRY[] = "Germany";
const char CITY[] = "Munich";
const char APIKEY[] = ""; // Wunderground

// Set Pins
#define PIN D1 // LED data pin
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

byte brightness = 30;
byte min_brightness = 5;
byte max_brightness = 255;
byte brightness_inc = 10;

// Initialize LEDs
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(256, PIN, NEO_GRB + NEO_KHZ800);

// Stuff for static IP
//IPAddress ip(192,168,0,128);  //Node static IP
//IPAddress gateway(192,168,0,1);
//IPAddress subnet(255,255,255,0);

MDNSResponder mdns;
ESP8266WebServer server(80);
String webPage = "";

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
static const char ntpServerName[] = "de.pool.ntp.org"; // NTP Server
void serial_clock_display();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);
const int timeZone = 1;     // Central European Time
int summertime = 0;

// German
byte es_ist[7] =      {195, 196, 198, 199, 200,   0,   0};
byte fuenf_min[7] =   {202, 203, 204, 205,   0,   0,   0};
byte zehn_min[7] =    {180, 181, 182, 183,   0,   0,   0};
byte viertel_min[7] = {167, 168, 169, 170, 171, 172, 173};
byte zwanzig_min[7] = {190, 189, 188, 187, 186, 185, 184};
byte nach[7] =        {148, 149, 150, 151,   0,   0,   0};
byte vor[7] =         {154, 155, 156,   0,   0,   0,   0};
byte halb[7] =        {131, 132, 133, 134,   0,   0,   0};
byte ein_std[7] =     {124, 123, 122,   0,   0,   0,   0};
byte eins_std[7] =    {124, 123, 122, 121,   0,   0,   0};
byte zwei_std[7] =    { 92,  91,  90,  89,   0,   0,   0};
byte drei_std[7] =    {126, 125, 124, 123,   0,   0,   0};
byte vier_std[7] =    { 99, 100, 101, 102,   0,   0,   0};
byte fuenf_std[7] =   {106, 107, 108, 109,   0,   0,   0};
byte sechs_std[7] =   { 88,  87,  86,  85,  84,   0,   0};
byte sieben_std[7] =  {116, 117, 118, 119, 120, 121,   0};
byte acht_std[7] =    { 68,  69,  70,  71,   0,   0,   0};
byte neun_std[7] =    { 59,  58,  57,  56,   0,   0,   0};
byte zehn_std[7] =    { 62,  61,  60,  59,   0,   0,   0};
byte elf_std[7] =     {104, 105, 106,   0,   0,   0,   0};
byte zwoelf_std[7] =  { 73,  74,  75,  76,  77,   0,   0};
byte null_std[7] =    {136, 137, 138, 139,   0,   0,   0};
byte uhr[7] =         { 54,  53,  52,   0,   0,   0,   0};
byte* hours_disp[13] = {null_std, eins_std, zwei_std, drei_std, vier_std, fuenf_std, sechs_std, sieben_std, acht_std, neun_std, zehn_std, elf_std, zwoelf_std};
byte* full_hours_disp[13] = {null_std, ein_std, zwei_std, drei_std, vier_std, fuenf_std, sechs_std, sieben_std, acht_std, neun_std, zehn_std, elf_std, zwoelf_std};

// Single minutes
byte min_1[7] = {1,   0,   0,  0, 0, 0, 0};
byte min_2[7] = {1, 255,   0,  0, 0, 0, 0};
byte min_3[7] = {1, 255, 241,  0, 0, 0, 0};
byte min_4[7] = {1, 255, 241, 16, 0, 0, 0};
byte* single_mins[4] = {min_1, min_2, min_3, min_4};

// Numbers
// Todo: Datentyp anpassen
byte left_1[17] =  {156, 166, 186, 167, 154, 135, 122, 103,  90,   0,   0,   0,   0,   0,   0,   0,   0};
byte right_1[17] = {150, 172, 180, 173, 148, 141, 116, 109,  84,   0,   0,   0,   0,   0,   0,   0,   0};
byte left_2[17] =  {163, 189, 188, 187, 167, 154, 134, 124, 100,  94,  93,  92,  91,  90,   0,   0,   0};
byte right_2[17] = {169, 183, 182, 181, 173, 148, 140, 118, 106,  88,  87,  86,  85,  84,   0,   0,   0};
byte left_3[17] =  {163, 189, 188, 187, 167, 154, 134, 133, 122, 103,  91,  92,  93,  99,   0,   0,   0};
byte right_3[17] = {169, 183, 182, 181, 173, 148, 140, 139, 116, 109,  85,  86,  87, 105,   0,   0,   0};
byte left_4[17] =  {187, 166, 155, 134, 123, 102,  91, 122, 124, 125, 126, 131, 157, 165,   0,   0,   0};
byte right_4[17] = {181, 172, 149, 140, 117, 108,  85, 116, 118, 119, 120, 137, 151, 171,   0,   0,   0};
byte left_5[17] =  {186, 187, 188, 189, 190, 163, 158, 157, 156, 155, 135, 122, 103,  91,  92,  93,  99};
byte right_5[17] = {180, 181, 182, 183, 184, 169, 152, 151, 150, 149, 141, 116, 109,  85,  86,  87, 105};
byte left_6[17] =  {167, 187, 188, 189, 163, 158, 131, 126,  99,  93,  92,  91, 103, 122, 134, 133, 132};
byte right_6[17] = {173, 181, 182, 183, 169, 152, 137, 120, 105,  87,  86,  85, 109, 116, 140, 139, 138};
byte left_7[17] =  {93,  100, 125, 133, 155, 167, 186, 187, 188, 189, 190,   0,   0,   0,   0,   0,   0};
byte right_7[17] = {87,  106, 119, 139, 149, 173, 180, 181, 182, 183, 184,   0,   0,   0,   0,   0,   0};
byte left_8[17] =  {187, 188, 189, 132, 133, 134,  91,  92,  93, 103, 122, 154, 167,  99, 126, 158, 163};
byte right_8[17] = {181, 182, 183, 138, 139, 140,  85,  86,  87, 109, 116, 148, 173, 169, 152, 120, 105};
byte left_9[17] =  {187, 188, 189, 132, 133, 134,  91,  92,  93, 103, 122, 154, 167,  99, 158, 163,   0};
byte right_9[17] = {181, 182, 183, 138, 139, 140,  85,  86,  87, 109, 116, 148, 173, 169, 152, 105,   0};
byte left_0[17] =  {187, 188, 189,  91,  92,  93, 163, 158, 131, 126,  99, 103, 122, 135, 154, 167,   0};
byte right_0[17] = {181, 182, 183, 169, 152, 137, 120, 105,  87,  86,  85, 109, 116, 141, 148, 173,   0};

byte* numbers_left[10] = {left_0, left_1, left_2, left_3, left_4, left_5, left_6, left_7, left_8, left_9};
byte* numbers_right[10] = {right_0, right_1, right_2, right_3, right_4, right_5, right_6, right_7, right_8, right_9};

// LED matrix indices
byte LED_matrix[10][11];
byte row_offset[10] = {62, 67, 94, 99, 126, 131, 158, 163, 190, 195};

// Weather types:
//   0: sunny
//   1: partly clouded
//   2: clouded
//   3: rain
//   4: heavy rain/thunderstorm

// Associated colors
//byte weather_color[5][3] = {
//  {100, 100, 100},
//  {100, 100, 100},
//  {100, 100, 100},
//  {100, 100, 100},
//  {100, 100, 100}
//};

byte hours = 0;
byte minutes = 0;
byte five_min = 0;
byte single_min = 0;

boolean en_es_ist = true;
boolean en_uhr = true;
boolean en_single_min = true;
boolean settings_changed = false;

////////////////////////////////////////////////////
// Setup routine
////////////////////////////////////////////////////
void setup() {
  Serial.begin (9600);
  EEPROM.begin(512); // There are 512 bytes of EEPROM, from 0 to 511
  
  // Execute this code only once to initialize default brightness
//  EEPROM.write(0, 10); // Min brightness set by user
//  EEPROM.write(1, 0); // Correspondig LDR value
//  EEPROM.write(2, 50); // Max brightness set by user
//  EEPROM.write(3, 255); // Correspondig LDR value
//  EEPROM.commit();

  // Initialize maxtrix indices (cannot be done before setup)
  for (byte i = 0; i <= 9; i++) { // rows
    for (byte j = 0; j <= 10; j++) { // columns
      if (i % 2 == 0) { // even
        LED_matrix[i][j] = row_offset[i] - j;
      }
      else {
        LED_matrix[i][j] = row_offset[i] + j;
      }
    }
  }

  webPage += "<font size=""7""><h1>Wordclock Web Server</h1></font>";
  webPage += "<font size=""7""><p>Datum (Tag): <a href=\"day_of_month\"><button>Anzeigent</button></a></p></font>";
  webPage += "<font size=""7""><p>Sprache <a href=\"language_ger\"><button>Deutsch</button></a>&nbsp;<a href=\"language_en\"><button>Englisch</button></a></p></font>";
  webPage += "<font size=""7""><p>Aktuelle Temperatur: <a href=\"disp_temp\"><button>Anzeigen</button></a></p></font>";
  webPage += "<font size=""7""><p>'Uhr' anzeigen: <a href=\"uhr_on\"><button>Ein</button></a>&nbsp;<a href=\"uhr_off\"><button>Aus</button></a></p></font>";
  webPage += "<font size=""7""><p>'Es ist' anzeigen: <a href=\"es_ist_on\"><button>Ein</button></a>&nbsp;<a href=\"es_ist_off\"><button>Aus</button></a></p></font>";
  webPage += "<font size=""7""><p>Einzelne Minuten anzeigen : <a href=\"single_min_on\"><button>Ein</button></a>&nbsp;<a href=\"single_min_off\"><button>Aus</button></a></p></font>";
  webPage += "<font size=""7""><p>LED Test: <a href=\"led_test\"><button>LED Test</button></a></p></font>";
  webPage += "<font size=""7""><p>Helligkeit: <a href=\"inc_brightness\"><button>Erhoehen</button></a>&nbsp;<a href=\"dec_brightness\"><button>Reduzieren</button></a></p></font>";
  webPage += "<font size=""7""><p>Kalibrieren: <a href=\"calib_bright\"><button>Heller Raum</button></a>&nbsp;<a href=\"calib_dark\"><button>Dunkler Raum</button></a></p></font>";

  pixels.begin(); // This initializes the NeoPixel library.

  delay(250);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  //WiFi.config(ip, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number for web server is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSummerTime();
  setSyncProvider(getNtpTime);
  // Todo: Test different sync intervals
  setSyncInterval(300);

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  server.on("/", []() {
    server.send(200, "text/html", webPage);
  });

  server.on("/day_of_month", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Displaying day of month");
    show_day();
    settings_changed = true;
    delay(1000);
  });

  server.on("/disp_temp", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Current temperature: XX");
    //    settings_changed = true;
    delay(1000);
  });

  // Todo
  server.on("/language_ger", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Language: German");
    //    settings_changed = true;
    delay(1000);
  });

  // Todo
  server.on("/language_en", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Language: English");
    //    settings_changed = true;
    delay(1000);
  });

  server.on("/uhr_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Uhr': on");
    if (!en_uhr) {
      en_uhr = true;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/uhr_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Uhr': on");
    if (en_uhr) {
      en_uhr = false;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/es_ist_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Es ist': on");
    if (!en_es_ist) {
      en_es_ist = true;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/es_ist_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Es ist': off");
    if (en_es_ist) {
      en_es_ist = false;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/single_min_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display single minute LEDs: on");
    if (!en_single_min) {
      en_single_min = true;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/single_min_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display single minute LEDs: off");
    if (en_single_min) {
      en_single_min = false;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/led_test", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Testing all LEDs");
    LED_test();
    settings_changed = true;
    delay(1000);
  });

  server.on("/inc_brightness", []() {
    server.send(200, "text/html", webPage);
    Serial.print("Increasing LED brightness to ");
    brightness += brightness_inc;
    brightness = constrain(brightness, min_brightness, max_brightness);
    Serial.print(brightness);
    Serial.println("");
    settings_changed = true;
    delay(1000);
  });

  server.on("/dec_brightness", []() {
    server.send(200, "text/html", webPage);
    Serial.print("Decreasing LED brightness to ");
    brightness -= brightness_inc;
    brightness = constrain(brightness, min_brightness, max_brightness);
    Serial.print(brightness);
    Serial.println("");
    settings_changed = true;
    delay(1000);
  });

  server.on("/calib_bright", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Brightness for bright room calibrated");
    // set_max_brightness();
    // settings_changed = true;
    delay(1000);
  });

  server.on("/calib_dark", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Brightness for dark room calibrated");
    // set_min_brightness();
    // settings_changed = true;
    delay(1000);
  });

  server.begin();
  Serial.println("HTTP server started");

  getWeatherData();
//  get_brightness();

}

time_t prevDisplay = 0; // when the digital clock was displayed

byte temp = 0;
byte temp_max_index = 0;

////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////
void loop() {

  server.handleClient();

  // Update everything only if minutes or settings have changed
  if (timeStatus() != timeNotSet) {
    if (minute() != prevDisplay || settings_changed) { // Alternative: now()

      // Determine LED brightness based on LDR measurement
      get_brightness();

      // Determine and display time
      clock_display(); // Real clock
      serial_clock_display(); // Serial port

      prevDisplay = minute();
      settings_changed = false;
    }
  }
}

////////////////////////////////////////////////////
// Determine current time and send data to LEDs
////////////////////////////////////////////////////
void clock_display() {

  // All LED off
  disable_all_led();

  // NTP time
  minutes = minute();
  hours = hour();

  // Single minutes
  single_min = minutes % 5;
  if (single_min > 0 && en_single_min) {
    send_time_2_LED(single_mins[single_min - 1]);
  }

  // Five minutes
  five_min = minutes - single_min;

  // Hours
  if (hours == 12) {
    hours = 12;
  }
  else {
    hours = hours % 12;
  }

  // Display time
  if (en_es_ist) {
    send_time_2_LED(es_ist);
  }

  switch (five_min) {
    case 0:
      send_time_2_LED(full_hours_disp[hours]);
      if (en_uhr) send_time_2_LED(uhr);
      break;
    case 5:
      send_time_2_LED(fuenf_min);
      send_time_2_LED(nach);
      send_time_2_LED(hours_disp[hours]);
      break;
    case 10:
      send_time_2_LED(zehn_min);
      send_time_2_LED(nach);
      send_time_2_LED(hours_disp[hours]);
      break;
    case 15:
      send_time_2_LED(viertel_min);
      send_time_2_LED(nach);
      send_time_2_LED(hours_disp[hours]);
      break;
    case 20:
      send_time_2_LED(zwanzig_min);
      send_time_2_LED(nach);
      send_time_2_LED(hours_disp[hours]);
      break;
    case 25:
      send_time_2_LED(fuenf_min);
      send_time_2_LED(vor);
      send_time_2_LED(halb);
      hours++;
      send_time_2_LED(hours_disp[hours]);
      break;
    case 30:
      send_time_2_LED(halb);
      hours++;
      send_time_2_LED(hours_disp[hours]);
      break;
    case 35:
      send_time_2_LED(fuenf_min);
      send_time_2_LED(nach);
      send_time_2_LED(halb);
      hours++;
      send_time_2_LED(hours_disp[hours]);
      break;
    case 40:
      send_time_2_LED(zwanzig_min);
      send_time_2_LED(vor);
      hours++;
      send_time_2_LED(hours_disp[hours]);
      break;
    case 45:
      send_time_2_LED(viertel_min);
      send_time_2_LED(vor);
      hours++;
      send_time_2_LED(hours_disp[hours]);
      break;
    case 50:
      send_time_2_LED(zehn_min);
      send_time_2_LED(vor);
      hours++;
      send_time_2_LED(hours_disp[hours]);
      break;
    case 55:
      send_time_2_LED(fuenf_min);
      send_time_2_LED(vor);
      hours++;
      send_time_2_LED(hours_disp[hours]);
      break;
  }

  pixels.show(); // This sends the updated pixel color to the hardware.
}

////////////////////////////////////////////////////
// Send data to LEDs
////////////////////////////////////////////////////
// Todo: Set Color & brightness
// Use only one display function with variable array sizes if possible
void send_time_2_LED(byte x[]) {
  for (byte i = 0; i <= 6; i++) {
    if (x[i] != 0) {
      pixels.setPixelColor(x[i] - 1, pixels.Color(brightness, brightness, brightness));
    }
  }
}

////////////////////////////////////////////////////
// Display numbers
////////////////////////////////////////////////////
// Todo: Set Color & brightness
void send_num_2_LED(byte x[]) {
  for (byte i = 0; i <= 16; i++) {
    if (x[i] != 0) {
      pixels.setPixelColor(x[i] - 1, pixels.Color(brightness, brightness, brightness));
    }
  }
}

////////////////////////////////////////////////////
// Disable all LEDs
////////////////////////////////////////////////////
// Todo: Not very elegant
void disable_all_led() {
  for (byte i = 0; i < 255; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}

////////////////////////////////////////////////////
// Display current time (serial monitor)
////////////////////////////////////////////////////
void serial_clock_display()
{
  // digital clock display of the time
  Serial.print("Current time and date: ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(", ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.print(" ");
  //Serial.print(" Summertime: ");
  //Serial.print(summertime);
  Serial.println();
}

////////////////////////////////////////////////////
// utility for digital clock display: prints preceding colon and leading 0
////////////////////////////////////////////////////
void printDigits(int digits)
{
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

////////////////////////////////////////////////////
// Determine offset due to German summertime
////////////////////////////////////////////////////
// Switch to summertime at last Sunday of March
// Switch to wintertime at last Sunday of October
void setSummerTime() {
  if (month() >= 3 && month() <= 10) {
    switch (month()) {
      // March
      case 3:
        if (day() > 31 - (7 - weekday())) {
          summertime = 1;
        }
        else {
          summertime = 0;
        }
        break;
      // October
      case 10:
        if (day() > 31 - (7 - weekday())) {
          summertime = 0;
        }
        else {
          summertime = 1;
        }
        break;
      default:
        summertime = 1;
    }
  }
  else {
    summertime = 0;
  }
}

////////////////////////////////////////////////////
// NTP code
////////////////////////////////////////////////////
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + (timeZone + summertime) * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

////////////////////////////////////////////////////
// Send an NTP request to the time server at the given address
////////////////////////////////////////////////////
void sendNTPpacket(IPAddress & address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

////////////////////////////////////////////////////
// Check if all LEDs are working properly
////////////////////////////////////////////////////
void LED_test() {

  byte test_brightness = 100;

  for (byte n = 0; n <= 3; n++) { // 3 times
    for (byte i = 0; i <= 9; i++) { // rows

      disable_all_led();

      for (byte j = 0; j <= 10; j++) { // columns
        switch (n) {
          case 0: // Red
            pixels.setPixelColor(LED_matrix[i][j] - 1, pixels.Color(test_brightness, 0, 0));
            break;
          case 1: // Green
            pixels.setPixelColor(LED_matrix[i][j] - 1, pixels.Color(0, test_brightness, 0));
            break;
          case 2: // Blue
            pixels.setPixelColor(LED_matrix[i][j] - 1, pixels.Color(0, 0, test_brightness));
            break;
          case 3: // White
            pixels.setPixelColor(LED_matrix[i][j] - 1, pixels.Color(test_brightness, test_brightness, test_brightness));
            break;
        }
      }
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(400);
    }
  }

}

////////////////////////////////////////////////////
// Display current weekday
////////////////////////////////////////////////////
void show_day() {

  disable_all_led();

  byte day_left = (day() - day() % 10) / 10;
  byte day_right = day() % 10;
  send_num_2_LED(numbers_left[day_left]);
  send_num_2_LED(numbers_right[day_right]);

  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(2000);

}

////////////////////////////////////////////////////
// Increase brightness locally
////////////////////////////////////////////////////
void get_brightness() {

  // Read LDR
  int sensorValue = analogRead(analogInPin)/4; // Value between 0 and 1023

  int min_user_brightness = EEPROM.read(0);
  int max_user_brightness = EEPROM.read(2);
  if(min_user_brightness > max_user_brightness) {
      max_user_brightness = min_user_brightness;
  }
  int min_sensor_value = EEPROM.read(1);
  int max_sensor_value = EEPROM.read(3);
  brightness = map(sensorValue, min_sensor_value, max_sensor_value, min_user_brightness, max_user_brightness);
  brightness = constrain(brightness, min_brightness, max_brightness);

  // Print sensor value to serial monitor
  Serial.print("Sensor value: ");
  Serial.print(sensorValue);
  Serial.print("/255, Brightness: ");
  Serial.print(brightness);
  Serial.println("/255");
}

////////////////////////////////////////////////////
// Set minimum brightness
////////////////////////////////////////////////////
void set_min_brightness() {

  // Read the analog in value
  byte sensorValue = analogRead(analogInPin)/4; // Value between 0 and 1023/4
  EEPROM.write(0, brightness); // Min brightness set by user
  EEPROM.write(2, sensorValue); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Set maximum brightness
////////////////////////////////////////////////////
void set_max_brightness() {

  // Read the analog in value
  byte sensorValue = analogRead(analogInPin)/4; // Value between 0 and 1023
  EEPROM.write(1, brightness); // Max brightness set by user
  EEPROM.write(3, sensorValue); // Correspondig LDR value
  EEPROM.commit();

}

//  // Test numbers
//  for(byte i = 0; i <= 5; i++) {
//    for(byte j = 0; j <= 9; j++) {
//      disable_all_led();
//      send_num_2_LED(numbers_left[j]);
//      send_num_2_LED(numbers_right[j]);
//      pixels.show(); // This sends the updated pixel color to the hardware.
//      delay(5000);
//    }
//  }

//  //  read temperature from somewhere
//  //  % Determine conservative temperature, quantisation: 3°C
//  byte temp_array[11] = {18, 20, 25, 19, 10, 12, 14, 16, 18, 20, 22};
//  byte weather_type[11] = {1, 2, 4, 3, 10, 3, 14, 16, 18, 20, 22};
//
//  // Display temperature and weather
//  for (byte i = 0; i <= 10; i++) {
//    temp = temp_array[i];
//    temp = constrain(temp, -3, 24); // Temperatur range: -3...25 °C
//    temp = temp + 3; // shift such that -3 °C has index 0
//    temp = temp - temp % 3;
//    temp_max_index = temp / 3; // LED with largest index
//    // Display temperature and weather type
//    for (byte j = 0; j <= temp_max_index; j++) {
//      pixels.setPixelColor(LED_matrix[i][j], pixels.Color(weather_color[weather_type[i]][1], weather_color[weather_type[i]][2], weather_color[weather_type[i]][3]));
//    }
//  }


//// Get data from Openweathermap
//void getWeatherData() //client function to send/receive GET request data.
//{
//
//  WiFiClient client;
//  char servername[] = "api.openweathermap.org"; // remote server we will connect to
//  String result;
//
//  String CityID = "3220838"; // Munich
//  String APIKEY = "";
//
//  if (client.connect(servername, 80)) {  //starts client connection, checks for connection
//    client.println("GET /data/2.5/weather?id=" + CityID + "&units=metric&APPID=" + APIKEY);
//    client.println("Host: api.openweathermap.org");
//    client.println("User-Agent: ArduinoWiFi/1.1");
//    client.println("Connection: close");
//    client.println();
//  }
//  else {
//    Serial.println("connection failed"); //error message if no client connect
//    Serial.println();
//  }
//
//  while (client.connected() && !client.available()) delay(1); //waits for data
//  while (client.connected() || client.available()) { //connected or data available
//    char c = client.read(); //gets byte from ethernet buffer
//    result = result + c;
//  }
//
//  client.stop(); //stop client
//  result.replace('[', ' ');
//  result.replace(']', ' ');
//  Serial.println(result);
//
//  char jsonArray [result.length() + 1];
//  result.toCharArray(jsonArray, sizeof(jsonArray));
//  jsonArray[result.length() + 1] = '\0';
//
//  StaticJsonBuffer<1024> json_buf;
//  JsonObject &root = json_buf.parseObject(jsonArray);
//  if (!root.success())
//  {
//    Serial.println("parseObject() failed");
//  }
//
//  // Example:
//  // {"coord":{"lon":11.64,"lat":48.05},
//  // "weather":{"id":800,"main":"Clear","description":"clear sky","icon":"01d"} ,
//  // "base":"cmc stations",
//  // "main":{"temp":29.38,"pressure":1018,"humidity":39,"temp_min":27.22,"temp_max":32.22},
//  // "wind":{"speed":2.6,"deg":120},"clouds":{"all":0},"dt":1469020873,
//  // "sys":{"type":3,"id":4887,"message":0.0034,"country":"DE","sunrise":1468985746,"sunset":1469041395},
//  // "id":3220838,
//  // "name":"Landkreis München",
//  // "cod":200}
//
//  String location = root["name"];
//  String country = root["sys"]["country"];
//  float temperature = root["main"]["temp"];
//  float humidity = root["main"]["humidity"];
//  String weather = root["weather"]["main"];
//  String description = root["weather"]["description"];
//  float pressure = root["main"]["pressure"];
//
//  Serial.println("Weather:");
//  Serial.println(description);
//  Serial.println(location);
//  Serial.println(country);
//  Serial.println(temperature);
//  Serial.println(humidity);
//  Serial.println(pressure);
//
//}

////////////////////////////////////////////////////
// Get historic weather data for certain date from Wunderground
////////////////////////////////////////////////////
void getWeatherData()
{
  WiFiClient client;
  String result;

  if (client.connect(weather_host, 80)) {  //starts client connection, checks for connection

    // Convert day to format DD
    String day_str;
    if (day() < 10) {
      day_str = "0";
      day_str += String(day());
    }
    else {
      day_str = String(day());
    }

    // Convert day to format MM
    String month_str;
    if (month() < 10) {
      month_str = "0";
      month_str += String(month());
    }
    else {
      month_str = String(month());
    }

    // Request history current day
    //    String request;
    //    request += "GET /api/";
    //    request += APIKEY;
    //    request += "/history_";
    //    request += String(year());
    //    request += month_str;
    //    request += day_str;
    //    request += "/q/";
    //    request += COUNTRY;
    //    request += "/";
    //    request += CITY;
    //    request += ".json HTTP/1.1";

    // Request current conditions
    String request;
    request += "GET /api/";
    request += APIKEY;
    request += "/conditions";
    request += "/q/";
    request += COUNTRY;
    request += "/";
    request += CITY;
    request += ".json HTTP/1.1";

    Serial.println(request);

    client.println(request);
    client.println("Host: api.wunderground.com");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("connection failed"); //error message if no client connect
    Serial.println();
  }

  while (client.connected() && !client.available()) delay(1); //waits for data
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    result = result + c;
  }

  client.stop(); //stop client
  //  result.replace('[', ' ');
  //  result.replace(']', ' ');
//  Serial.println(result); // Computationally expensive

  Serial.print("Result length: ");
  Serial.println(result.length() + 1);

  char jsonArray [result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
//
//  StaticJsonBuffer<4096> json_buf;
//  JsonObject &root = json_buf.parseObject(jsonArray);
//  if (!root.success())
//  {
//    Serial.println("parseObject() failed");
//  }
//
//  byte mday_res = root["history"]["observations"][0]["date"]["mday"];
//  byte mon_res = root["history"]["observations"][0]["date"]["mon"];
//  int year_res = root["history"]["observations"][0]["date"]["year"];
//  byte hour_res = root["history"]["observations"][0]["date"]["hour"];
//  byte min_res = root["history"]["observations"][0]["date"]["min"];
//  float temp_res = root["history"]["observations"][0]["tempm"];

//  Serial.println("Observation hour 0:");
//  Serial.print(mday_res);
//  Serial.print("-");
//  Serial.print(mon_res);
//  Serial.print("-");
//  Serial.print(year_res);
//  Serial.println("");
//
//  Serial.print(hour_res);
//  Serial.print(":");
//  Serial.print(min_res);
//  Serial.println("");
//
//  Serial.print("Temperature: ");
//  Serial.println(temp_res);

  //  // Select 11*2h temperature forecast
  //  for (byte i = 0; i <= 10; i++) {
  //    temp_forecast[i] = root["hourly"]["data"][2 * i]["temperature"];
  //  }

//  String timeZone = root["timezone"];

  //display_temp_forecast();
  //Serial.println("Weather:");

}
