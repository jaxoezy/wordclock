#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <RGBConverter.h>

extern const uint8_t gamma8[];

// Weather forecast Wunderground
const char weather_host[] = "api.wunderground.com";
const char COUNTRY[] = "Germany";
const char CITY[] = "Munich";
const char APIKEY[] = ""; // Wunderground API key

// Set Pins
#define PIN D1 // LED data pin
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

RGBConverter rgb_conv;
byte clock_rgb[3];
byte ambilight_rgb[3];
double hsv_value_inc = 0.1;

// Initialize LEDs
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(256, PIN, NEO_GRB + NEO_KHZ800);

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

int ambimode = 0;
int led = 0;

// German
byte es_ist[7] =      {1, 2, 4, 5, 6, 0, 0};
byte fuenf_min[7] =   {8, 9, 10, 11, 0, 0, 0};
byte zehn_min[7] =    {12, 13, 14, 15, 0, 0, 0};
byte viertel_min[7] = {27, 28, 29, 30, 31, 32, 33};
byte zwanzig_min[7] = {22, 21, 20, 19, 18, 17, 16};
byte nach[7] =        {34, 35, 36, 37, 0, 0, 0};
byte vor[7] =         {40, 41, 42, 0, 0, 0, 0};
byte halb[7] =        {45, 46, 47, 48, 0, 0, 0};
byte ein_std[7] =     {64, 63, 62, 0, 0, 0, 0};
byte eins_std[7] =    {64, 63, 62, 61, 0, 0, 0};
byte zwei_std[7] =    {86, 85, 84, 83, 0, 0, 0};
byte drei_std[7] =    {66, 65, 64, 63, 0, 0, 0};
byte vier_std[7] =    {67, 68, 69, 70, 0, 0, 0};
byte fuenf_std[7] =   {74, 75, 76, 77, 0, 0, 0};
byte sechs_std[7] =   {82, 81, 80, 79, 78, 0, 0};
byte sieben_std[7] =  {56, 57, 58, 59, 60, 61, 0};
byte acht_std[7] =    {90, 91, 92, 93, 0, 0, 0};
byte neun_std[7] =    {107, 106, 105, 104, 0, 0, 0};
byte zehn_std[7] =    {110, 109, 108, 107, 0, 0, 0};
byte elf_std[7] =     {72, 73, 74, 0, 0, 0, 0};
byte zwoelf_std[7] =  {95, 96, 97, 98, 99, 0, 0};
byte null_std[7] =    {50, 51, 52, 53, 0, 0, 0};
byte uhr[7] =         {102, 101, 100, 0, 0, 0, 0};
byte* hours_GER[13] = {null_std, eins_std, zwei_std, drei_std, vier_std, fuenf_std, sechs_std, sieben_std, acht_std, neun_std, zehn_std, elf_std, zwoelf_std};
byte* full_hours_GER[13] = {null_std, ein_std, zwei_std, drei_std, vier_std, fuenf_std, sechs_std, sieben_std, acht_std, neun_std, zehn_std, elf_std, zwoelf_std};

// English
byte it_is[7] =       {  1,   2,   4,   5,   0,   0,   0};
byte five_min[7] =    { 29,  30,  31,  32,   0,   0,   0};
byte ten_min[7] =     { 37,  38,  39,   0,   0,   0,   0};
byte quarter_min[7] = { 14,  15,  16,  17,  18,  19,  20};
byte twenty_min[7] =  { 23,  24,  25,  26,  27,  28,   0};
byte past[7] =        { 45,  46,  47,  48,   0,   0,   0};
byte to[7] =          { 34,  35,   0,   0,   0,   0,   0};
byte half[7] =        { 41,  42,  43,  44,   0,   0,   0};
byte one_std[7] =     { 64,  65,  66,   0,   0,   0,   0};
byte two_std[7] =     { 75,  76,  77,   0,   0,   0,   0};
byte three_std[7] =   { 56,  57,  58,  59,  60,   0,   0};
byte four_std[7] =    { 67,  68,  69,  70,   0,   0,   0};
byte five_std[7] =    { 71,  72,  73,  74,   0,   0,   0};
byte six_std[7] =     { 61,  62,  63,   0,   0,   0,   0};
byte seven_std[7] =   { 89,  90,  91,  92,  93,   0,   0};
byte eight_std[7] =   { 84,  85,  86,  87,  88,   0,   0};
byte nine_std[7] =    { 52,  53,  54,  55,   0,   0,   0};
byte ten_std[7] =     {108, 109, 110,   0,   0,   0,   0};
byte eleven_std[7] =  { 78,  79,  80,  81,  82,  83,   0};
byte twelve_std[7] =  { 94,  95,  96,  97,  98,  99,   0};
//byte null_std[7] =    { 50,  51,  52,  53,   0,   0,   0};
byte oclock[7] =      {100, 101, 102, 103, 104, 105,   0};
byte am[7] =          {  8,   9,   0,   0,   0,   0,   0};
byte pm[7] =          { 10,  11,   0,   0,   0,   0,   0};
byte* hours_EN[13] = {twelve_std, one_std, two_std, three_std, four_std, five_std, six_std, seven_std, eight_std, nine_std, ten_std, eleven_std, twelve_std};

// Single minutes
byte min_1[7] = {114,   0,   0,  0, 0, 0, 0};
byte min_2[7] = {114, 113,   0,  0, 0, 0, 0};
byte min_3[7] = {114, 113, 112,  0, 0, 0, 0};
byte min_4[7] = {114, 113, 112, 111, 0, 0, 0};
byte* single_mins[4] = {min_1, min_2, min_3, min_4};

// Numbers
byte left_1[17] =  {42, 26, 18, 27, 40, 49, 62, 71, 84, 0, 0, 0, 0, 0, 0, 0, 0};
byte right_1[17] = {36, 32, 12, 33, 34, 55, 56, 77, 78, 0, 0, 0, 0, 0, 0, 0, 0};
byte left_2[17] =  {23, 21, 20, 19, 27, 40, 48, 64, 68, 88, 87, 86, 85, 84, 0, 0, 0};
byte right_2[17] = {29, 15, 14, 13, 33, 34, 54, 58, 74, 82, 81, 80, 79, 78, 0, 0, 0};
byte left_3[17] =  {23, 21, 20, 19, 27, 40, 48, 47, 62, 71, 85, 86, 87, 67, 0, 0, 0};
byte right_3[17] = {29, 15, 14, 13, 33, 34, 54, 53, 56, 77, 79, 80, 81, 73, 0, 0, 0};
byte left_4[17] =  {19, 26, 41, 48, 63, 70, 85, 62, 64, 65, 66, 45, 43, 25, 0, 0, 0};
byte right_4[17] = {13, 32, 35, 54, 57, 76, 79, 56, 58, 59, 60, 51, 37, 31, 0, 0, 0};
byte left_5[17] =  {18, 19, 20, 21, 22, 23, 44, 43, 42, 41, 49, 62, 71, 85, 86, 87, 67};
byte right_5[17] = {12, 13, 14, 15, 16, 29, 38, 37, 36, 35, 55, 56, 77, 79, 80, 81, 73};
byte left_6[17] =  {27, 19, 20, 21, 23, 44, 45, 66, 67, 87, 86, 85, 71, 62, 48, 47, 46};
byte right_6[17] = {33, 13, 14, 15, 29, 38, 51, 60, 73, 81, 80, 79, 77, 56, 54, 53, 52};
byte left_7[17] =  {87, 68, 65, 47, 41, 27, 18, 19, 20, 21, 22, 0, 0, 0, 0, 0, 0};
byte right_7[17] = {81, 74, 59, 53, 35, 33, 12, 13, 14, 15, 16, 0, 0, 0, 0, 0, 0};
byte left_8[17] =  {19, 20, 21, 46, 47, 48, 85, 86, 87, 71, 62, 40, 27, 67, 66, 44, 23};
byte right_8[17] = {13, 14, 15, 52, 53, 54, 79, 80, 81, 77, 56, 34, 33, 29, 38, 60, 73};
byte left_9[17] =  {19, 20, 21, 46, 47, 48, 85, 86, 87, 71, 62, 40, 27, 67, 44, 23, 0};
byte right_9[17] = {13, 14, 15, 52, 53, 54, 79, 80, 81, 77, 56, 34, 33, 29, 38, 73, 0};
byte left_0[17] =  {19, 20, 21, 85, 86, 87, 23, 44, 45, 66, 67, 71, 62, 49, 40, 27, 0};
byte right_0[17] = {13, 14, 15, 29, 38, 51, 60, 73, 81, 80, 79, 77, 56, 55, 34, 33, 0};

byte* numbers_left[10] = {left_0, left_1, left_2, left_3, left_4, left_5, left_6, left_7, left_8, left_9};
byte* numbers_right[10] = {right_0, right_1, right_2, right_3, right_4, right_5, right_6, right_7, right_8, right_9};

// LED matrix indices
byte LED_matrix[10][11];
byte row_offset[10] = {1, 22, 23, 44, 45, 66, 67, 88, 89, 110};

byte hours = 0;
byte minutes = 0;
byte min_five = 0;
byte single_min = 0;

double min_user_brightness;
double max_user_brightness;
byte min_LDR_value;
byte max_LDR_value;
byte en_es_ist;
byte en_uhr;
byte en_single_min;
byte en_ambilight;
boolean settings_changed = false;
int language;
byte en_nighttime = 1;
byte t_night_1;
byte t_night_2;
double h_clock;
double s_clock;
double v_clock;
double h_ambilight;
double s_ambilight;
double v_ambilight;

// EEPROM address assignment
const int EEPROM_addr_min_brightness = 0;
const int EEPROM_addr_LDR_min = 1;
const int EEPROM_addr_max_brightness = 2;
const int EEPROM_addr_LDR_max = 3;
const int EEPROM_addr_es_ist = 4;
const int EEPROM_addr_uhr = 5;
const int EEPROM_addr_single_min = 6;
const int EEPROM_addr_ambilight = 7;
const int EEPROM_addr_language = 8;
const int EEPROM_addr_t_night_1 = 9;
const int EEPROM_addr_t_night_2 = 10;
const int EEPROM_addr_h_clock = 11;
const int EEPROM_addr_s_clock = 12;
const int EEPROM_addr_v_clock = 13;
const int EEPROM_addr_h_ambilight = 14;
const int EEPROM_addr_s_ambilight = 15;
const int EEPROM_addr_v_ambilight = 16;

////////////////////////////////////////////////////
// Setup routine
////////////////////////////////////////////////////
void setup() {
  Serial.begin (9600);

  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  // Fetches ssid and pass and tries to connect
  // If it does not connect it starts an access point with the specified name
  // here "Wordclock"
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Wordclock")) {
    Serial.println("failed to connect and hit timeout");
    // Reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  EEPROM.begin(512); // There are 512 bytes of EEPROM, from 0 to 511

  // Execute this code only once to initialize default values
  //    EEPROM.write(EEPROM_addr_min_brightness, 10); // Min brightness set by user
  //    EEPROM.write(EEPROM_addr_LDR_min, 0); // Correspondig LDR value
  //    EEPROM.write(EEPROM_addr_max_brightness, 80); // Max brightness set by user
  //    EEPROM.write(EEPROM_addr_LDR_max, 255); // Correspondig LDR value
  //    EEPROM.write(EEPROM_addr_es_ist, 1); // "Es ist", default: on
  //    EEPROM.write(EEPROM_addr_uhr, 1); // "Uhr", default: on
  //    EEPROM.write(EEPROM_addr_single_min, 1); // Display singles minutes, default: on
  //    EEPROM.write(EEPROM_addr_ambilight, 0); // Ambilight, default: off
  //    EEPROM.write(EEPROM_addr_language, 1); // Language: 0: German, 1: English, default: German
  //    EEPROM.write(EEPROM_addr_t_night_1, 1); // Starting hour of nighttime, default: 1 am
  //    EEPROM.write(EEPROM_addr_t_night_2, 7); // Ending hour of nighttime, default: 7 am
  //    EEPROM.write(EEPROM_addr_h_clock, 8); // Hue LED clock, default: 0
  //    EEPROM.write(EEPROM_addr_s_clock, 100); // Saturation LED clock, default: 100
  //    EEPROM.write(EEPROM_addr_v_clock, 75); // Value LED clock, default: 0
  //    EEPROM.write(EEPROM_addr_h_ambilight, 8); // Hue LED ambilight, default: 0
  //    EEPROM.write(EEPROM_addr_s_ambilight, 100); // Saturaion LED ambilight, default: 100
  //    EEPROM.write(EEPROM_addr_v_ambilight, 75); // Value LED ambilight, default: 0
  //    EEPROM.commit();

  // Read settings from EEPROM
  min_user_brightness = EEPROM.read(EEPROM_addr_min_brightness) / 100;
  max_user_brightness = EEPROM.read(EEPROM_addr_max_brightness) / 100;
  min_user_brightness = constrain(min_user_brightness, 0, 1);
  max_user_brightness = constrain(max_user_brightness, 0, 1);
  min_LDR_value = EEPROM.read(EEPROM_addr_LDR_min);
  max_LDR_value = EEPROM.read(EEPROM_addr_LDR_max);
  en_es_ist = EEPROM.read(EEPROM_addr_es_ist);
  en_uhr = EEPROM.read(EEPROM_addr_uhr);
  en_single_min = EEPROM.read(EEPROM_addr_single_min);
  en_ambilight = EEPROM.read(EEPROM_addr_ambilight);
  language = EEPROM.read(EEPROM_addr_language);
  t_night_1 = EEPROM.read(EEPROM_addr_t_night_1);
  t_night_2 = EEPROM.read(EEPROM_addr_t_night_2);
  h_clock = (double) EEPROM.read(EEPROM_addr_h_clock) / 100; // Must be in range [0..1]
  s_clock = (double) EEPROM.read(EEPROM_addr_s_clock) / 100; // Must be in range [0..1]
  v_clock = (double) EEPROM.read(EEPROM_addr_v_clock) / 100; // Must be in range [0..1]
  h_clock = constrain(h_clock, 0, 1);
  s_clock = constrain(s_clock, 0, 1);
  v_clock = constrain(v_clock, 0, 1);
  h_ambilight = (double) EEPROM.read(EEPROM_addr_h_ambilight) / 100; // Must be in range [0..1]
  s_ambilight = (double) EEPROM.read(EEPROM_addr_s_ambilight) / 100; // Must be in range [0..1]
  v_ambilight = (double) EEPROM.read(EEPROM_addr_v_ambilight) / 100; // Must be in range [0..1]
  h_ambilight = constrain(h_ambilight, 0, 1);
  s_ambilight = constrain(s_ambilight, 0, 1);
  v_ambilight = constrain(v_ambilight, 0, 1);

  // Convert HSV to RGB
  rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
  rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);

  // Initialize maxtrix indices (cannot be done before setup)
  for (byte i = 0; i <= 9; i++) { // rows
    for (byte j = 0; j <= 10; j++) { // columns
      if (i % 2 == 0) { // even
        LED_matrix[i][j] = row_offset[i] + j;
      }
      else {
        LED_matrix[i][j] = row_offset[i] - j;
      }
    }
  }

  // HTML website for server
  webPage += "<font size=""7""><h1>Wordclock Web Server</h1></font>";
  webPage += "<font size=""7""><p>Datum (Tag): <a href=\"day_of_month\"><button>Anzeigen</button></a></p></font>";
  webPage += "<font size=""7""><p>Sprache <a href=\"language_ger\"><button>Deutsch</button></a>&nbsp;<a href=\"language_en\"><button>Englisch</button></a></p></font>";
  webPage += "<font size=""7""><p>Aktuelle Temperatur: <a href=\"disp_temp\"><button>Anzeigen</button></a></p></font>";
  webPage += "<font size=""7""><p>'Uhr' anzeigen: <a href=\"uhr_on\"><button>Ein</button></a>&nbsp;<a href=\"uhr_off\"><button>Aus</button></a></p></font>";
  webPage += "<font size=""7""><p>'Es ist' anzeigen: <a href=\"es_ist_on\"><button>Ein</button></a>&nbsp;<a href=\"es_ist_off\"><button>Aus</button></a></p></font>";
  webPage += "<font size=""7""><p>Einzelne Minuten anzeigen : <a href=\"single_min_on\"><button>Ein</button></a>&nbsp;<a href=\"single_min_off\"><button>Aus</button></a></p></font>";
  webPage += "<font size=""7""><p>LED Test: <a href=\"led_test\"><button>LED Test</button></a></p></font>";
  webPage += "<font size=""7""><p>Helligkeit: <a href=\"inc_brightness\"><button>Erhoehen</button></a>&nbsp;<a href=\"dec_brightness\"><button>Reduzieren</button></a></p></font>";
  webPage += "<font size=""7""><p>Kalibrieren: <a href=\"calib_bright\"><button>Heller Raum</button></a>&nbsp;<a href=\"calib_dark\"><button>Dunkler Raum</button></a></p></font>";
  webPage += "<font size=""7""><p>Ambilight: <a href=\"ambilight_on\"><button>Ein</button></a>&nbsp;<a href=\"ambilight_off\"><button>Aus</button></a></p></font>";
  webPage += "<font size=""7""><p>Nachtzeit: <a href=\"nighttime_on\"><button>Ein</button></a>&nbsp;<a href=\"nighttime_off\"><button>Aus</button></a></p></font>";

  pixels.begin(); // This initializes the NeoPixel library.

  Serial.print("IP for web server is ");
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

  // Connect to wordlock through wordclock.local
  if (mdns.begin("wordclock", WiFi.localIP())) {
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

  server.on("/language_ger", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Language: German");
    if (language) {
      EEPROM.write(EEPROM_addr_language, 0);
      EEPROM.commit();
      language = false;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/language_en", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Language: English");
    if (!language) {
      EEPROM.write(EEPROM_addr_language, 1);
      EEPROM.commit();
      language = true;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/uhr_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Uhr': on");
    if (!en_uhr) {
      EEPROM.write(EEPROM_addr_uhr, 1);
      EEPROM.commit();
      en_uhr = true;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/uhr_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Uhr': on");
    if (en_uhr) {
      EEPROM.write(EEPROM_addr_uhr, 0);
      EEPROM.commit();
      en_uhr = false;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/es_ist_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Es ist': on");
    if (!en_es_ist) {
      EEPROM.write(EEPROM_addr_es_ist, 1);
      EEPROM.commit();
      en_es_ist = true;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/es_ist_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display 'Es ist': off");
    if (en_es_ist) {
      EEPROM.write(EEPROM_addr_es_ist, 0);
      EEPROM.commit();
      en_es_ist = false;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/single_min_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display single minute LEDs: on");
    if (!en_single_min) {
      EEPROM.write(EEPROM_addr_single_min, 1);
      EEPROM.commit();
      en_single_min = true;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/single_min_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Display single minute LEDs: off");
    if (en_single_min) {
      EEPROM.write(EEPROM_addr_single_min, 0);
      EEPROM.commit();
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
    v_clock += hsv_value_inc;
    v_clock = constrain(v_clock, min_user_brightness, max_user_brightness);
    v_ambilight = v_clock;
    // Convert to RGB
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);
    Serial.print(v_clock);
    Serial.println("");
    settings_changed = true;
    delay(1000);
  });

  server.on("/dec_brightness", []() {
    server.send(200, "text/html", webPage);
    Serial.print("Decreasing LED brightness to ");
    v_clock -= hsv_value_inc;
    v_clock = constrain(v_clock, min_user_brightness, max_user_brightness);
    v_ambilight = v_clock;
    // Convert to RGB
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);
    Serial.print(v_clock);
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

  server.on("/ambilight_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Enable ambilight");
    if (!en_ambilight) {
      EEPROM.write(EEPROM_addr_ambilight, 1);
      EEPROM.commit();
      en_ambilight = true;
      ambilight();
    }
    delay(1000);
  });

  server.on("/ambilight_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Disable ambilight");
    if (en_ambilight) {
      EEPROM.write(EEPROM_addr_ambilight, 0);
      EEPROM.commit();
      en_ambilight = false;
      disable_ambilight();
    }
    delay(1000);
  });

  server.on("/nighttime_on", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Enable nighttime");
    if (!en_nighttime) {
      en_nighttime = 1;
      settings_changed = true;
    }
    delay(1000);
  });

  server.on("/nighttime_off", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Disable nighttime");
    if (en_nighttime) {
      en_nighttime = 0;
      settings_changed = true;
    }
    delay(1000);
  });

  server.begin();
  Serial.println("HTTP server started");

  // getWeatherData();
  //  get_brightness();

}

time_t prevDisplay = 0; // when the digital clock was displayed

byte temp = 0;
byte temp_max_index = 0;

////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////
void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    // Handle Webserver
    server.handleClient();

    // Execute everything only if minutes or settings have changed
    if (timeStatus() != timeNotSet) {
      if ((minute() != prevDisplay || settings_changed)) { // Alternative: now()

        // Check whether nighttime is active
        if (!nighttime()) {

          // Determine LED brightness based on LDR measurement
          //get_brightness();

          if (en_ambilight)
            ambilight();
          else
            disable_ambilight();

          // Determine and display time
          clock_display(); // Real clock
          serial_clock_display(); // Serial port
        }
        else
          disable_all_led();

        prevDisplay = minute();
        settings_changed = false;

      }
    }
  }
  else
    ESP.reset();
}


////////////////////////////////////////////////////
// Determine current time and send data to LEDs
////////////////////////////////////////////////////
void clock_display() {

  // All clock LED off
  disable_clock_led();

  // Get NTP time
  minutes = minute();
  hours = hour();

  // Single minutes
  single_min = minutes % 5;
  if (single_min > 0 && en_single_min)
    send_time_2_LED(single_mins[single_min - 1]);

  // Five minutes
  min_five = minutes - single_min;

  // Hours
  if (hours == 12)
    hours = 12;
  else
    hours = hours % 12; // hours modulo 12

  // Hier zwischen Sprachen wechseln
  switch (language) {

    // German
    case 0:

      // Display "es ist"
      if (en_es_ist)
        send_time_2_LED(es_ist);

      switch (min_five) {
        case 0:
          send_time_2_LED(full_hours_GER[hours]);
          if (en_uhr) send_time_2_LED(uhr);
          break;
        case 5:
          send_time_2_LED(fuenf_min);
          send_time_2_LED(nach);
          send_time_2_LED(hours_GER[hours]);
          break;
        case 10:
          send_time_2_LED(zehn_min);
          send_time_2_LED(nach);
          send_time_2_LED(hours_GER[hours]);
          break;
        case 15:
          send_time_2_LED(viertel_min);
          send_time_2_LED(nach);
          send_time_2_LED(hours_GER[hours]);
          break;
        case 20:
          send_time_2_LED(zwanzig_min);
          send_time_2_LED(nach);
          send_time_2_LED(hours_GER[hours]);
          break;
        case 25:
          send_time_2_LED(fuenf_min);
          send_time_2_LED(vor);
          send_time_2_LED(halb);
          hours++;
          send_time_2_LED(hours_GER[hours]);
          break;
        case 30:
          send_time_2_LED(halb);
          hours++;
          send_time_2_LED(hours_GER[hours]);
          break;
        case 35:
          send_time_2_LED(fuenf_min);
          send_time_2_LED(nach);
          send_time_2_LED(halb);
          hours++;
          send_time_2_LED(hours_GER[hours]);
          break;
        case 40:
          send_time_2_LED(zwanzig_min);
          send_time_2_LED(vor);
          hours++;
          send_time_2_LED(hours_GER[hours]);
          break;
        case 45:
          send_time_2_LED(viertel_min);
          send_time_2_LED(vor);
          hours++;
          send_time_2_LED(hours_GER[hours]);
          break;
        case 50:
          send_time_2_LED(zehn_min);
          send_time_2_LED(vor);
          hours++;
          send_time_2_LED(hours_GER[hours]);
          break;
        case 55:
          send_time_2_LED(fuenf_min);
          send_time_2_LED(vor);
          hours++;
          send_time_2_LED(hours_GER[hours]);
          break;
      }

      break;

    // English
    case 1:

      // Display "it is"
      if (en_es_ist)
        send_time_2_LED(it_is);

      switch (min_five) {
        case 0:
          send_time_2_LED(hours_EN[hours]);
          if (en_uhr) send_time_2_LED(oclock);
          break;
        case 5:
          send_time_2_LED(five_min);
          send_time_2_LED(past);
          send_time_2_LED(hours_EN[hours]);
          break;
        case 10:
          send_time_2_LED(ten_min);
          send_time_2_LED(past);
          send_time_2_LED(hours_EN[hours]);
          break;
        case 15:
          send_time_2_LED(quarter_min);
          send_time_2_LED(past);
          send_time_2_LED(hours_EN[hours]);
          break;
        case 20:
          send_time_2_LED(twenty_min);
          send_time_2_LED(past);
          send_time_2_LED(hours_EN[hours]);
          break;
        case 25:
          send_time_2_LED(twenty_min);
          send_time_2_LED(five_min);
          send_time_2_LED(past);
          send_time_2_LED(hours_EN[hours]);
          break;
        case 30:
          send_time_2_LED(half);
          send_time_2_LED(past);
          send_time_2_LED(hours_EN[hours]);
          break;
        case 35:
          send_time_2_LED(twenty_min);
          send_time_2_LED(five_min);
          send_time_2_LED(to);
          hours++;
          send_time_2_LED(hours_EN[hours]);
          break;
        case 40:
          send_time_2_LED(twenty_min);
          send_time_2_LED(to);
          hours++;
          send_time_2_LED(hours_EN[hours]);
          break;
        case 45:
          send_time_2_LED(quarter_min);
          send_time_2_LED(to);
          hours++;
          send_time_2_LED(hours_EN[hours]);
          break;
        case 50:
          send_time_2_LED(ten_min);
          send_time_2_LED(to);
          hours++;
          send_time_2_LED(hours_EN[hours]);
          break;
        case 55:
          send_time_2_LED(five_min);
          send_time_2_LED(to);
          hours++;
          send_time_2_LED(hours_EN[hours]);
          break;
      }
      break;
  }

  pixels.show(); // This sends the updated pixel color to the hardware.
}

////////////////////////////////////////////////////
// Send data to LEDs
////////////////////////////////////////////////////
void send_time_2_LED(byte x[]) {
  //Serial.print("LED color: R: ");
  //Serial.print(pgm_read_byte(&gamma8[clock_rgb[0]]));
  //Serial.print(", G: ");
  //Serial.print(pgm_read_byte(&gamma8[clock_rgb[1]]));
  //Serial.print(", B: ");
  //Serial.println(pgm_read_byte(&gamma8[clock_rgb[2]]));
  //
  //Serial.print("LED color: H: ");
  //Serial.print(h_clock);
  //Serial.print(", S: ");
  //Serial.print(s_clock);
  //Serial.print(", V: ");
  //Serial.println(v_clock);

  for (byte i = 0; i <= 6; i++) {
    if (x[i] != 0)
      // pixels.setPixelColor(x[i] - 1, pixels.Color(R_clock, G_clock, B_clock));
      pixels.setPixelColor(x[i] - 1, pgm_read_byte(&gamma8[clock_rgb[0]]), pgm_read_byte(&gamma8[clock_rgb[1]]), pgm_read_byte(&gamma8[clock_rgb[2]]));
  }
}

////////////////////////////////////////////////////
// Display numbers
////////////////////////////////////////////////////
void send_num_2_LED(byte x[]) {
  for (byte i = 0; i <= 16; i++) {
    if (x[i] != 0)
      // pixels.setPixelColor(x[i] - 1, pixels.Color(R_clock, G_clock, B_clock));
      pixels.setPixelColor(x[i] - 1, pgm_read_byte(&gamma8[clock_rgb[0]]), pgm_read_byte(&gamma8[clock_rgb[1]]), pgm_read_byte(&gamma8[clock_rgb[2]]));
  }
}

////////////////////////////////////////////////////
// Disable clock LEDs
////////////////////////////////////////////////////
void disable_clock_led() {
  for (byte i = 0; i <= 114; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}

////////////////////////////////////////////////////
// Enable ambilight LEDs
////////////////////////////////////////////////////
void ambilight() {
  // Fade colors
  //if(led == 49) { led = 0; ambimode++; } else { led++; }
  //if(ambimode == 25) { ambimode = 0; }
  //pixels.setPixelColor(led + 114, pixels.Color(255-(10 * ambimode), 10 * ambimode, 255-(10 * ambimode)));

  // Static color
  for (byte i = 114; i <= 170; i++) {
    // pixels.setPixelColor(i - 1, pixels.Color(R_ambilight, G_ambilight, B_ambilight));
    pixels.setPixelColor(i - 1, pgm_read_byte(&gamma8[ambilight_rgb[0]]), pgm_read_byte(&gamma8[ambilight_rgb[1]]), pgm_read_byte(&gamma8[ambilight_rgb[2]]));
  }
  pixels.show();

}

////////////////////////////////////////////////////
// Disable ambilight LEDs
////////////////////////////////////////////////////
void disable_ambilight() {
  for (byte i = 115; i < 255; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();

}

////////////////////////////////////////////////////
// Disable all LEDs
////////////////////////////////////////////////////
void disable_all_led() {
  for (byte i = 0; i < 255; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

////////////////////////////////////////////////////
// Display current time (serial monitor)
////////////////////////////////////////////////////
void serial_clock_display()
{
  // IP for webserver access
  // Serial.print("IP for web server is ");
  // Serial.println(WiFi.localIP());

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
// Utility for digital clock display: prints preceding colon and leading 0
////////////////////////////////////////////////////
void printDigits(int digits)
{
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

////////////////////////////////////////////////////
// Check whether the specfied nighttime is active
// If t_night_1 = t_night_2, then false is always returned
// Nighttime can only be disabled locally, i.e. during the current night
// By default it is always enabled
////////////////////////////////////////////////////
boolean nighttime() {

  // If nighttime is enabled
  if (en_nighttime) {
    if (t_night_1 == t_night_2)
      return false;
    // t_1 before mignight, t_2 after midnight
    else if (t_night_1 > t_night_2) {
      if (hour() >= t_night_1 || hour() < t_night_2)
        return true;
      else {
        en_nighttime = 1; // Enable nighttime again during the day
        return false;
      }
    }
    // t_1 and t_2 after midnight
    else if (t_night_1 < t_night_2) {
      if (hour() >= t_night_1 && hour() < t_night_2)
        return true;
      else {
        en_nighttime = 1; // Enable nighttime again during the day
        return false;
      }
    }
  }
  else
    return false;

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
        if (day() > 31 - (7 - weekday()))
          summertime = 1;
        else
          summertime = 0;
        break;
      // October
      case 10:
        if (day() > 31 - (7 - weekday()))
          summertime = 0;
        else
          summertime = 1;
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
// Wifimanager
////////////////////////////////////////////////////
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
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
// Sweep through rows, each time with different color
////////////////////////////////////////////////////
void LED_test() {

  byte test_brightness = 150;

  for (byte n = 0; n <= 3; n++) { // 3 times
    for (byte i = 0; i <= 9; i++) { // rows

      disable_clock_led();

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

  disable_clock_led();

  byte day_left = (day() - day() % 10) / 10;
  byte day_right = day() % 10;
  send_num_2_LED(numbers_left[day_left]);
  send_num_2_LED(numbers_right[day_right]);

  pixels.show();
  delay(2000);

}

////////////////////////////////////////////////////
// Determine brightness based on sensor data and limits
////////////////////////////////////////////////////
// TODO: Helligkeit ist noch nicht mit eingebaut
// Bisher nur default-Farbe, die Helligkeitsskalierung ist nicht enthalten
void get_brightness() {

  // Read LDR
  int sensorValue = analogRead(analogInPin) / 4; // Value between 0 and 1023

  if (min_user_brightness > max_user_brightness)
    max_user_brightness = min_user_brightness;

  v_clock = map(sensorValue, min_LDR_value, max_LDR_value, min_user_brightness, max_user_brightness);
  v_clock = constrain(v_clock, min_user_brightness, max_user_brightness);
  v_ambilight = v_clock;

  // Print sensor value to serial monitor
  Serial.print("Sensor value: ");
  Serial.print(sensorValue);
  Serial.print("/255, Brightness: ");
  Serial.print(v_clock);
  Serial.println("/1");
}

////////////////////////////////////////////////////
// Set minimum brightness
////////////////////////////////////////////////////
void set_min_brightness() {

  // Read the analog in value
  byte sensorValue = analogRead(analogInPin) / 4; // Value between 0 and 1023/4
  EEPROM.write(EEPROM_addr_min_brightness, v_clock * 100); // Min brightness set by user
  EEPROM.write(EEPROM_addr_LDR_min, sensorValue); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Set maximum brightness
////////////////////////////////////////////////////
void set_max_brightness() {

  // Read the analog in value
  byte sensorValue = analogRead(analogInPin) / 4; // Value between 0 and 1023
  EEPROM.write(EEPROM_addr_max_brightness, v_clock * 100); // Max brightness set by user
  EEPROM.write(EEPROM_addr_LDR_max, sensorValue); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Test numbers
////////////////////////////////////////////////////
void test_numbers() {
  for (byte i = 0; i <= 5; i++) {
    for (byte j = 0; j <= 9; j++) {
      disable_clock_led();
      send_num_2_LED(numbers_left[j]);
      send_num_2_LED(numbers_right[j]);
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(500);
    }
  }
}

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

const uint8_t PROGMEM gamma8[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};
