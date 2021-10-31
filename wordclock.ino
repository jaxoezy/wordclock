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
#include <Wire.h>

// Set Pins
#define PIN D5 // LED data pin
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

// Initialize LEDs
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(256, PIN, NEO_RGB + NEO_KHZ800);
const int MAX_NUM_LEDS = 114;
RGBConverter rgb_conv;
byte clock_rgb[3];
double hsv_value_inc = 0.05;

MDNSResponder mdns;
ESP8266WebServer server(80);
String webPage = "WordClock";
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
unsigned long OldTimeSinds1900 = 0;
int SyncTime;
static const char ntpServerName[] = "nl.pool.ntp.org"; // NTP Server
const int timeZone = 1;     // Central European Time
int summertime = 0;

// Dutch time
byte nl_het_is[7] =      {  2,   3,   4,  6,    7,   0,   0};
byte nl_vijf_min[7] =    {  9,  10,  11,  12,   0,   0,   0};
byte nl_tien_min[7] =    { 24,  23,  22,  21,   0,   0,   0};
byte nl_kwart_min[7] =   { 31,  32,  33,  34,  35,   0,   0};
byte nl_voor[7] =        { 17,  16,  15,  14,   0,   0,   0};
byte nl_over[7] =        { 25,  26,  27,  28,   0,   0,   0};
byte nl_voor1[7] =       { 47,  48,  49,  50,   0,   0,   0};
byte nl_over1[7] =       { 39,  38,  37,  36,   0,   0,   0};
byte nl_half[7] =        { 46,  45,  44,  43,   0,   0,   0};
byte nl_een_std[7] =     { 54,  55,  56,   0,   0,   0,   0};
byte nl_twee_std[7] =    { 68,  67,  66,  65,   0,   0,   0};
byte nl_drie_std[7] =    { 61,  60,  59,  58,   0,   0,   0};
byte nl_vier_std[7] =    { 69,  70,  71,  72,   0,   0,   0};
byte nl_vijf_std[7] =    { 73,  74,  75,  76,   0,   0,   0};
byte nl_zes_std[7] =     { 77,  78,  79,   0,   0,   0,   0};
byte nl_zeven_std[7] =   { 90,  89,  88,  87,  86,   0,   0};
byte nl_acht_std[7] =    { 91,  92,  93,  94,   0,   0,   0};
byte nl_negen_std[7] =   { 84,  83,  82,  81,  80,   0,   0};
byte nl_tien_std[7] =    { 95,  96,  97,  98,   0,   0,   0};
byte nl_elf_std[7] =     { 99,  100, 101,   0,   0,   0,   0};
byte nl_twaalf_std[7] =  {113, 112, 111, 110, 109, 108,   0};
//byte null_std[7] =     {  0,   0,   0,   0,   0,   0,   0};
byte nl_uur[7] =         {105, 104, 103,   0,   0,   0,   0};
byte nl_am[7] =          {  0,   0,   0,   0,   0,   0,   0};
byte nl_pm[7] =          {  0,   0,   0,   0,   0,   0,   0};
byte* hours_nl[13] = {nl_twaalf_std, nl_een_std, nl_twee_std, nl_drie_std, nl_vier_std, nl_vijf_std, nl_zes_std, nl_zeven_std, nl_acht_std, nl_negen_std, nl_tien_std, nl_elf_std, nl_twaalf_std};

// Single minutes
byte min_1[7] =       {1,   0,   0,  0, 0, 0, 0};
byte min_2[7] =       {1,  13,   0,  0, 0, 0, 0};
byte min_3[7] =       {1,  13, 102,  0, 0, 0, 0};
byte min_4[7] =       {1,  13, 102, 114, 0, 0, 0};
byte* single_mins[4] = {min_1, min_2, min_3, min_4};

// Numbers
byte left_1[17] =  {44, 28, 20, 29, 42, 51, 64, 73, 86, 0, 0, 0, 0, 0, 0, 0, 0};
byte right_1[17] = {38, 34, 14, 35, 36, 57, 58, 79, 80, 0, 0, 0, 0, 0, 0, 0, 0};
byte left_2[17] =  {25, 23, 22, 21, 29, 42, 50, 66, 70, 90, 89, 88, 87, 86, 0, 0, 0};
byte right_2[17] = {31, 17, 16, 15, 35, 36, 56, 60, 76, 84, 83, 82, 81, 80, 0, 0, 0};
byte left_3[17] =  {25, 23, 22, 21, 29, 42, 50, 49, 64, 73, 87, 88, 89, 69, 0, 0, 0};
byte right_3[17] = {31, 17, 16, 15, 35, 36, 56, 55, 58, 79, 81, 82, 83, 75, 0, 0, 0};
byte left_4[17] =  {21, 28, 43, 50, 65, 72, 87, 64, 64, 67, 68, 47, 45, 27, 0, 0, 0};
byte right_4[17] = {15, 34, 37, 56, 59, 78, 81, 58, 60, 61, 62, 53, 39, 33, 0, 0, 0};
byte left_5[17] =  {20, 21, 22, 23, 24, 25, 46, 45, 44, 43, 51, 64, 73, 87, 88, 89, 69};
byte right_5[17] = {14, 15, 16, 17, 18, 31, 40, 39, 38, 37, 57, 58, 79, 81, 82, 83, 75};
byte left_6[17] =  {29, 21, 22, 23, 25, 46, 47, 68, 69, 89, 88, 87, 73, 64, 50, 49, 48};
byte right_6[17] = {35, 15, 16, 17, 31, 40, 53, 62, 75, 83, 82, 81, 79, 58, 56, 55, 54};
byte left_7[17] =  {89, 70, 67, 49, 43, 29, 20, 21, 22, 23, 24, 0, 0, 0, 0, 0, 0};
byte right_7[17] = {83, 76, 61, 55, 37, 35, 14, 15, 16, 17, 18, 0, 0, 0, 0, 0, 0};
byte left_8[17] =  {21, 22, 23, 48, 49, 50, 87, 88, 89, 73, 64, 42, 29, 69, 68, 46, 25};
byte right_8[17] = {15, 16, 17, 54, 55, 56, 81, 82, 83, 79, 58, 36, 35, 31, 40, 62, 75};
byte left_9[17] =  {21, 22, 23, 48, 49, 50, 87, 88, 89, 73, 64, 42, 29, 69, 46, 25, 0};
byte right_9[17] = {15, 16, 17, 54, 55, 56, 81, 82, 83, 79, 58, 36, 35, 31, 40, 75, 0};
byte left_0[17] =  {21, 22, 23, 87, 88, 89, 25, 46, 47, 68, 69, 73, 64, 51, 42, 29, 0};
byte right_0[17] = {15, 16, 17, 31, 40, 53, 62, 75, 83, 82, 81, 79, 58, 57, 36, 35, 0};

byte* numbers_left[10] = {left_0, left_1, left_2, left_3, left_4, left_5, left_6, left_7, left_8, left_9};
byte* numbers_right[10] = {right_0, right_1, right_2, right_3, right_4, right_5, right_6, right_7, right_8, right_9};

// LED matrix indices
byte LED_matrix[10][11];
byte row_offset[10] = {2, 24, 25, 46, 47, 68, 69, 90, 91, 113};
byte Corners[4] = {1, 13, 102, 114};

byte hours = 0;
byte hoursNighttime = 0;
byte minutes = 0;
byte min_five = 0;
byte single_min = 0;

byte en_it_is;
byte en_am_pm;
byte en_oclock;
byte en_single_min;
boolean settings_changed = false;
byte en_clock = 1;
byte en_nighttime = 1;
int weekdays;
//temps to display value on screen
byte t_night_1;
byte t_night_2;
byte t_mo_night_start;
byte t_mo_night_end;
byte t_tu_night_start;
byte t_tu_night_end;
byte t_we_night_start;
byte t_we_night_end;
byte t_th_night_start;
byte t_th_night_end;
byte t_fr_night_start;
byte t_fr_night_end;
byte t_sa_night_start;
byte t_sa_night_end;
byte t_su_night_start;
byte t_su_night_end;
double t_h_clock;
double t_s_clock;
double t_v_clock;

double h_clock;
double s_clock;
double v_clock;
bool httpChange;

// EEPROM address assignment
const int EEPROM_addr_it_is = 6;
const int EEPROM_addr_oclock = 7;
const int EEPROM_addr_am_pm = 8;
const int EEPROM_addr_single_min = 9;
const int EEPROM_addr_t_mo_night_start = 10;
const int EEPROM_addr_t_mo_night_end = 11;
const int EEPROM_addr_t_tu_night_start = 12;
const int EEPROM_addr_t_tu_night_end = 13;
const int EEPROM_addr_t_we_night_start = 14;
const int EEPROM_addr_t_we_night_end = 15;
const int EEPROM_addr_t_th_night_start = 16;
const int EEPROM_addr_t_th_night_end = 17;
const int EEPROM_addr_t_fr_night_start = 18;
const int EEPROM_addr_t_fr_night_end = 19;
const int EEPROM_addr_t_sa_night_start = 20;
const int EEPROM_addr_t_sa_night_end = 21;
const int EEPROM_addr_t_su_night_start = 22;
const int EEPROM_addr_t_su_night_end = 23;
const int EEPROM_addr_h_clock = 24;
const int EEPROM_addr_s_clock = 25;
const int EEPROM_addr_v_clock = 26;
//const int EEPROM_addr_LDR_corr_type = 27;
const int EEPROM_SummerTime = 28;


////////////////////////////////////////////////////
// Setup routine
////////////////////////////////////////////////////
void setup() {
  Serial.begin (9600);

  // Start the I2C interface
  //Wire.begin();

  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  //wifiManager.setAPCallback(configModeCallback);
  // hotspot time out - will retry to connect to the last network
  wifiManager.setConfigPortalTimeout(300);
  // Fetches ssid and pass and tries to connect
  // If it does not connect it starts an access point with the specified name "Wordclock"
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Wordclock")) {
    Serial.println("failed to connect and hit timeout");
    // Reset and try again, or maybe put it to deep sleep
    ESP.restart(); // originally ESP.reset()
    delay(1000);
  }

  EEPROM.begin(512); // There are 512 bytes of EEPROM, from 0 to 511

  // Execute this code only once to initialize default values
  //          EEPROM.write(EEPROM_addr_it_is, 1); // "Es ist", default: on
  //          EEPROM.write(EEPROM_addr_am_pm, 0); // "am/pm", default: off
  //          EEPROM.write(EEPROM_addr_oclock, 1); // "Uhr", default: on
  //          EEPROM.write(EEPROM_addr_single_min, 1); // Display singles minutes, default: on
  //          EEPROM.write(EEPROM_addr_t_mo_night_start, 1); // Starting hour of nighttime, default: 1 am
  //          EEPROM.write(EEPROM_addr_t_mo_night_end, 7); // Ending hour of nighttime, default: 7 am
  //          EEPROM.write(EEPROM_addr_t_tu_night_start, 1); // Starting hour of nighttime, default: 1 am
  //          EEPROM.write(EEPROM_addr_t_tu_night_end, 7); // Ending hour of nighttime, default: 7 am
  //          EEPROM.write(EEPROM_addr_t_we_night_start, 1); // Starting hour of nighttime, default: 1 am
  //          EEPROM.write(EEPROM_addr_t_we_night_end, 7); // Ending hour of nighttime, default: 7 am
  //          EEPROM.write(EEPROM_addr_t_th_night_start, 1); // Starting hour of nighttime, default: 1 am
  //          EEPROM.write(EEPROM_addr_t_th_night_end, 7); // Ending hour of nighttime, default: 7 am
  //          EEPROM.write(EEPROM_addr_t_fr_night_start, 1); // Starting hour of nighttime, default: 1 am
  //          EEPROM.write(EEPROM_addr_t_fr_night_end, 7); // Ending hour of nighttime, default: 7 am
  //          EEPROM.write(EEPROM_addr_t_sa_night_start, 1); // Starting hour of nighttime, default: 1 am
  //          EEPROM.write(EEPROM_addr_t_sa_night_end, 7); // Ending hour of nighttime, default: 7 am
  //          EEPROM.write(EEPROM_addr_t_su_night_start, 1); // Starting hour of nighttime, default: 1 am
  //          EEPROM.write(EEPROM_addr_t_su_night_end, 7); // Ending hour of nighttime, default: 7 am
  //          EEPROM.write(EEPROM_addr_h_clock, 8); // Hue LED clock, default: 0
  //          EEPROM.write(EEPROM_addr_s_clock, 0); // Saturation LED clock, default: 0
  //          EEPROM.write(EEPROM_addr_v_clock, 50); // Value LED clock, default: 0
  //          EEPROM.write(EEPROM_SummerTime, 0); // ll SummerTime on/off, default 0
  //          EEPROM.commit();

  // Read settings from EEPROM
  en_it_is = EEPROM.read(EEPROM_addr_it_is);
  en_am_pm = EEPROM.read(EEPROM_addr_am_pm);
  en_oclock = EEPROM.read(EEPROM_addr_oclock);
  en_single_min = EEPROM.read(EEPROM_addr_single_min);
  t_mo_night_start = EEPROM.read(EEPROM_addr_t_mo_night_start);
  t_mo_night_end = EEPROM.read(EEPROM_addr_t_mo_night_end);
  t_tu_night_start = EEPROM.read(EEPROM_addr_t_tu_night_start);
  t_tu_night_end = EEPROM.read(EEPROM_addr_t_tu_night_end);
  t_we_night_start = EEPROM.read(EEPROM_addr_t_we_night_start);
  t_we_night_end = EEPROM.read(EEPROM_addr_t_we_night_end);
  t_th_night_start = EEPROM.read(EEPROM_addr_t_th_night_start);
  t_th_night_end = EEPROM.read(EEPROM_addr_t_th_night_end);
  t_fr_night_start = EEPROM.read(EEPROM_addr_t_fr_night_start);
  t_fr_night_end = EEPROM.read(EEPROM_addr_t_fr_night_end);
  t_sa_night_start = EEPROM.read(EEPROM_addr_t_sa_night_start);
  t_sa_night_end = EEPROM.read(EEPROM_addr_t_sa_night_end);
  t_su_night_start = EEPROM.read(EEPROM_addr_t_su_night_start);
  t_su_night_end = EEPROM.read(EEPROM_addr_t_su_night_end);
  h_clock = (double) EEPROM.read(EEPROM_addr_h_clock) / 100; // Must be in range [0..1]
  s_clock = (double) EEPROM.read(EEPROM_addr_s_clock) / 100; // Must be in range [0..1]
  v_clock = (double) EEPROM.read(EEPROM_addr_v_clock) / 100; // Must be in range [0..1]
  h_clock = constrain(h_clock, 0, 1);
  s_clock = constrain(s_clock, 0, 1);
  v_clock = constrain(v_clock, 0, 1);
  t_h_clock = (double)EEPROM.read(EEPROM_addr_h_clock);
  t_s_clock = (double)EEPROM.read(EEPROM_addr_s_clock);
  t_v_clock = (double)EEPROM.read(EEPROM_addr_v_clock);
  summertime = EEPROM.read(EEPROM_SummerTime);

  // Convert HSV to RGB
  rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);

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

  // load http site for the first time
  httpChange = true;
  LoadHttp();

  pixels.begin(); // This initializes the NeoPixel library.
  disableClockLED();
  Serial.print("IP for web server is ");
  Serial.println(WiFi.localIP());
  ShowIp();
  Serial.println("Starting UDP");
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Udp.begin(localPort);
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  //setSummerTime();
  setSyncProvider(getNtpTime);
  // Todo: Test different sync intervals
  // SyncTime also used with Time_T GetNtpTime, when offline add this sync value
  SyncTime = 300;
  setSyncInterval(SyncTime);

  //   Connect to wordlock via wordclock.local
  if (mdns.begin("wordclock", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  server.on("/", []() {
    server.send(200, "text/html", webPage);
  });

  /////////////////////////////////////
  // Server: General
  /////////////////////////////////////

  // Reset module
  server.on("/reset", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Resetting word clock");
    disableClockLED();
    ESP.reset();
  });

  // Reset wifi
  server.on("/resetWifi", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Resetting wifi module");
    disableClockLED();
    WiFi.disconnect();
    delay(2000);
    ESP.reset();
  });
  /////////////////////////////////////
  // Server: Clock
  /////////////////////////////////////

  // Enable/Disable clock LEDs temporarily
  server.on("/clock_leds", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Enable clock LEDs");
      en_clock = true;
    }
    else {
      Serial.println("Disable clock LEDs");
      en_clock = false;
    }
    settings_changed = true;
    delay(1000);
  });

  // Select clock hue
  server.on("/hue_clock", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    state = map(state, 0, 360, 0, 100);
    h_clock = double(state) / 100;
    t_h_clock = double(state);
    // Convert to RGB
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    Serial.print("Clock LED hue is now: ");
    Serial.print(h_clock);
    Serial.println(" / 1");
    settings_changed = true;
    httpChange = true;
    delay(1000);
  });

  // Select clock saturation
  server.on("/sat_clock", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    s_clock = (double) state / 100;
    t_s_clock = double(state);
    // Convert to RGB
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    Serial.print("Clock LED saturation is now: ");
    Serial.print(s_clock);
    Serial.println(" / 1");
    settings_changed = true;
    httpChange = true;
    delay(1000);
  });

  // Select clock brightness
  server.on("/clock_brightness", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    v_clock = (double) state / 100;
    t_v_clock = double(state);
    // Convert to RGB
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    Serial.print("Clock LED brightness is now: ");
    Serial.print(v_clock);
    Serial.println(" / 1");
    settings_changed = true;
    httpChange = true;
    delay(1000);
  });

  // Store current clock color to EEPROM
  server.on("/store_color_clock", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Saving clock hue and saturation values in EEPROM");
    byte temp = (int) (h_clock * 100);
    EEPROM.write(EEPROM_addr_h_clock, temp);
    Serial.print("Hue: ");
    Serial.println(temp);
    temp = (int) (s_clock * 100);
    EEPROM.write(EEPROM_addr_s_clock, temp);
    Serial.print("Saturation: ");
    Serial.println(temp);
    temp = (int) (v_clock * 100);
    EEPROM.write(EEPROM_addr_v_clock, temp);
    Serial.print("Brightness: ");
    Serial.println(temp);
    EEPROM.commit();
    delay(1000);
  });

  // Enable/Disable "o'clock"
  server.on("/disp_oclock", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Display 'o'clock' enabled");
      if (!en_oclock) {
        EEPROM.write(EEPROM_addr_oclock, 1);
        EEPROM.commit();
        en_oclock = true;
        settings_changed = true;
      }
    }
    else {
      Serial.println("Display 'o'clock' disabled");
      if (en_oclock) {
        EEPROM.write(EEPROM_addr_oclock, 0);
        EEPROM.commit();
        en_oclock = false;
        settings_changed = true;
      }
    }
    delay(1000);
  });

  // Enable/Disable "It is"
  server.on("/disp_it_is", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Display 'it is' enabled");
      if (!en_it_is) {
        EEPROM.write(EEPROM_addr_it_is, 1);
        EEPROM.commit();
        en_it_is = true;
        settings_changed = true;
      }
    }
    else {
      Serial.println("Display 'it is' disabled");
      if (en_it_is) {
        EEPROM.write(EEPROM_addr_it_is, 0);
        EEPROM.commit();
        en_it_is = false;
        settings_changed = true;
      }
    }
    delay(1000);
  });

  // Enable/Disable "am/pm"
  server.on("/disp_am_pm", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Display 'am/pm' enabled");
      if (!en_am_pm) {
        EEPROM.write(EEPROM_addr_am_pm, 1);
        EEPROM.commit();
        en_am_pm = true;
        settings_changed = true;
      }
    }
    else {
      Serial.println("Display 'am/pm' disabled");
      if (en_am_pm) {
        EEPROM.write(EEPROM_addr_am_pm, 0);
        EEPROM.commit();
        en_am_pm = false;
        settings_changed = true;
      }
    }
    delay(1000);
  });

  // Enable/Disable single minute LEDs
  server.on("/disp_single_min", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Display single minute LEDs: on");
      if (!en_single_min) {
        EEPROM.write(EEPROM_addr_single_min, 1);
        EEPROM.commit();
        en_single_min = true;
        settings_changed = true;
      }
    }
    else {
      Serial.println("Display single minute LEDs: off");
      if (en_single_min) {
        EEPROM.write(EEPROM_addr_single_min, 0);
        EEPROM.commit();
        en_single_min = false;
        settings_changed = true;
      }
    }
    delay(1000);
  });

  /////////////////////////////////////
  // Server: Night-time
  /////////////////////////////////////
  //Monday-----------------------------------------------------------------
  // Start of night-time
  server.on("/mo_night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_mo_night_start = state;
    EEPROM.write(EEPROM_addr_t_mo_night_start, t_mo_night_start);
    EEPROM.commit();
    Serial.print("Starting monday night-time at: ");
    Serial.print(t_mo_night_start);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  // End of night-time
  server.on("/mo_night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_mo_night_end = state;
    EEPROM.write(EEPROM_addr_t_mo_night_end, t_mo_night_end);
    EEPROM.commit();
    Serial.print("Ending monday night-time at: ");
    Serial.print(t_mo_night_end);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  //Tuesday-----------------------------------------------------------------
  // Start of night-time
  server.on("/tu_night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_tu_night_start = state;
    EEPROM.write(EEPROM_addr_t_tu_night_start, t_tu_night_start);
    EEPROM.commit();
    Serial.print("Starting tuesday night-time at: ");
    Serial.print(t_tu_night_start);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  // End of night-time
  server.on("/tu_night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_tu_night_end = state;
    EEPROM.write(EEPROM_addr_t_tu_night_end, t_tu_night_end);
    EEPROM.commit();
    Serial.print("Ending tuesday night-time at: ");
    Serial.print(t_tu_night_end);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  //Wednesday-----------------------------------------------------------------
  // Start of night-time
  server.on("/we_night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_we_night_start = state;
    EEPROM.write(EEPROM_addr_t_we_night_start, t_we_night_start);
    EEPROM.commit();
    Serial.print("Starting wenday night-time at: ");
    Serial.print(t_we_night_start);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  // End of night-time
  server.on("/we_night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_we_night_end = state;
    EEPROM.write(EEPROM_addr_t_we_night_end, t_we_night_end);
    EEPROM.commit();
    Serial.print("Ending wenday night-time at: ");
    Serial.print(t_we_night_end);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  //Thursday-----------------------------------------------------------------
  // Start of night-time
  server.on("/th_night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_th_night_start = state;
    EEPROM.write(EEPROM_addr_t_th_night_start, t_th_night_start);
    EEPROM.commit();
    Serial.print("Starting thnday night-time at: ");
    Serial.print(t_th_night_start);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  // End of night-time
  server.on("/th_night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_th_night_end = state;
    EEPROM.write(EEPROM_addr_t_th_night_end, t_th_night_end);
    EEPROM.commit();
    Serial.print("Ending thnday night-time at: ");
    Serial.print(t_th_night_end);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  //Friday-----------------------------------------------------------------
  // Start of night-time
  server.on("/fr_night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_fr_night_start = state;
    EEPROM.write(EEPROM_addr_t_fr_night_start, t_fr_night_start);
    EEPROM.commit();
    Serial.print("Starting frnday night-time at: ");
    Serial.print(t_fr_night_start);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  // End of night-time
  server.on("/fr_night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_fr_night_end = state;
    EEPROM.write(EEPROM_addr_t_fr_night_end, t_fr_night_end);
    EEPROM.commit();
    Serial.print("Ending frnday night-time at: ");
    Serial.print(t_fr_night_end);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  //Saterday-----------------------------------------------------------------
  // Start of night-time
  server.on("/sa_night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_sa_night_start = state;
    EEPROM.write(EEPROM_addr_t_sa_night_start, t_sa_night_start);
    EEPROM.commit();
    Serial.print("Starting sanday night-time at: ");
    Serial.print(t_sa_night_start);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  // End of night-time
  server.on("/sa_night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_sa_night_end = state;
    EEPROM.write(EEPROM_addr_t_sa_night_end, t_sa_night_end);
    EEPROM.commit();
    Serial.print("Ending sanday night-time at: ");
    Serial.print(t_sa_night_end);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  //Sunday-----------------------------------------------------------------
  // Start of night-time
  server.on("/su_night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_su_night_start = state;
    EEPROM.write(EEPROM_addr_t_su_night_start, t_su_night_start);
    EEPROM.commit();
    Serial.print("Starting sunday night-time at: ");
    Serial.print(t_su_night_start);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });
  // End of night-time
  server.on("/su_night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_su_night_end = state;
    EEPROM.write(EEPROM_addr_t_su_night_end, t_su_night_end);
    EEPROM.commit();
    Serial.print("Ending sunday night-time at: ");
    Serial.print(t_su_night_end);
    Serial.println(" h");
    httpChange = true;
    delay(1000);
  });

  // Enable/Disable night-time temporarily
  server.on("/nighttime_temp", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 0) {
      Serial.println("Disable night-time temporarily");
      if (en_nighttime) {
        en_nighttime = 0;
        settings_changed = true;
      }
    }
    else {
      Serial.println("Enable night-time");
      if (!en_nighttime) {
        en_nighttime = 1;
        settings_changed = true;
      }
    }
    delay(1000);
  });

  // Select SummerTime
  server.on("/summertime", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("enable SummerTime");
      summertime = 1;
    }
    else {
      Serial.println("Disable SummerTime");
      summertime = 0;
    }
    settings_changed = true;
    EEPROM.write(EEPROM_SummerTime, summertime);
    EEPROM.commit();
    delay(1000);
  });

  /////////////////////////////////////
  // Server: Other settings
  /////////////////////////////////////

  // LED test
  server.on("/LEDTest", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Testing all LEDs");
    LEDTest();
    settings_changed = true;
    delay(1000);
  });

  // Show current day of month
  server.on("/day_of_month", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Displaying day of month");
    showDay();
    settings_changed = true;
    delay(1000);
  });

  server.begin();
  Serial.println("HTTP server started");

}

time_t prevDisplay = 0; // when the digital clock was displayed

////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////
void loop() {
  LoadHttp();
  // removed, so it will keep counting offline and waits till it regains connection
  // if (WiFi.status() == WL_CONNECTED) {
  // Handle Webserver
  server.handleClient();
  // Execute everything only if minutes or settings have changed
  if (timeStatus() != timeNotSet) {
    if ((minute() != prevDisplay || settings_changed)) { // Alternative: now()

      // Check whether nighttime is active
      if (!nighttime() && en_clock) {

        // Determine and display time
        clockDisplay(); // Real clock
        serialClockDisplay(); // Serial port
      }
      else
        disableAllLED();
      prevDisplay = minute();
      settings_changed = false;
    }
  }
  else {
    Serial.println("Time not set");
    disableClockLED();
  }
  //   }
  //  else
  // maybe give a sign that wifi is lost
  //   ESP.restart();
}

////////////////////////////////////////////////////
// Determine current time and send data to LEDs
////////////////////////////////////////////////////
void clockDisplay() {

  // All clock LED off
  disableClockLED();

  // Get NTP time
  minutes = minute();
  hours = hour();

  // if summertime, do hour +1
  if (summertime == 1) hours++;

  // Single minutes
  single_min = minutes % 5;
  if (single_min > 0 && en_single_min)
    sendTime2LED(single_mins[single_min - 1]);

  // Five minutes
  min_five = minutes - single_min;

  // Hours
  if (hours >= 12) hours = hours % 12;


  // Dutch time
  // Display "het is"
  if (en_it_is)
    sendTime2LED(nl_het_is);

  switch (min_five) {
    case 0:
      sendTime2LED(hours_nl[hours]);
      if (en_oclock) sendTime2LED(nl_uur);
      break;
    case 5:
      sendTime2LED(nl_vijf_min);
      sendTime2LED(nl_over);
      sendTime2LED(hours_nl[hours]);
      break;
    case 10:
      sendTime2LED(nl_tien_min);
      sendTime2LED(nl_over);
      sendTime2LED(hours_nl[hours]);
      break;
    case 15:
      sendTime2LED(nl_kwart_min);
      sendTime2LED(nl_over1);
      sendTime2LED(hours_nl[hours]);
      break;
    case 20:
      sendTime2LED(nl_tien_min);
      sendTime2LED(nl_voor);
      sendTime2LED(nl_half);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
    case 25:
      sendTime2LED(nl_vijf_min);
      sendTime2LED(nl_voor);
      sendTime2LED(nl_half);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
    case 30:
      sendTime2LED(nl_half);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
    case 35:
      sendTime2LED(nl_vijf_min);
      sendTime2LED(nl_over);
      sendTime2LED(nl_half);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
    case 40:
      sendTime2LED(nl_tien_min);
      sendTime2LED(nl_over);
      sendTime2LED(nl_half);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
    case 45:
      sendTime2LED(nl_kwart_min);
      sendTime2LED(nl_voor1);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
    case 50:
      sendTime2LED(nl_tien_min);
      sendTime2LED(nl_voor);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
    case 55:
      sendTime2LED(nl_vijf_min);
      sendTime2LED(nl_voor);
      hours++;
      sendTime2LED(hours_nl[hours]);
      break;
  }

  pixels.show(); // This sends the updated pixel color to the hardware.
}

////////////////////////////////////////////////////
// Send data to LEDs
////////////////////////////////////////////////////
void sendTime2LED(byte x[]) {

  for (byte i = 0; i <= 6; i++) {
    if (x[i] != 0)
      pixels.setPixelColor(x[i] - 1, clock_rgb[0], clock_rgb[1], clock_rgb[2]);
  }
}

////////////////////////////////////////////////////
// Display numbers
////////////////////////////////////////////////////
void sendNum2LED(byte x[]) {
  for (byte i = 0; i <= 16; i++) {
    if (x[i] != 0)
      pixels.setPixelColor(x[i] - 1, clock_rgb[0], clock_rgb[1], clock_rgb[2]);
  }
}

////////////////////////////////////////////////////
// Disable clock LEDs
////////////////////////////////////////////////////
void disableClockLED() {
  for (byte i = 0; i <= 114; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}


////////////////////////////////////////////////////
// Disable all LEDs
////////////////////////////////////////////////////
void disableAllLED() {
  for (byte i = 0; i < 255; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

////////////////////////////////////////////////////
// Display current time (serial monitor)
////////////////////////////////////////////////////
void serialClockDisplay()
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
  Serial.print(summertime);
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
// If t_night_1 = t_night_2, then "false" is always returned
// Nighttime can only be disabled locally, i.e. during the current night
// By default it is always enabled
////////////////////////////////////////////////////
boolean nighttime() {
  // If nighttime is enabled
  if (en_nighttime) {
    weekdays = weekday();

    switch (weekdays) {
      case 1: //sunday
        t_night_1 = t_su_night_start;
        t_night_2 = t_su_night_end;
        Serial.print("today is sunday");
        break;
      case 2: //Monday
        t_night_1 = t_mo_night_start;
        t_night_2 = t_mo_night_end;
        Serial.print("today is monday");
        break;
      case 3: //Tuesday
        t_night_1 = t_tu_night_start;
        t_night_2 = t_tu_night_end;
        Serial.print("today is tuesday");
        break;
      case 4: //Wednesday
        t_night_1 = t_we_night_start;
        t_night_2 = t_we_night_end;
        Serial.print("today is wednesday");
        break;
      case 5: //Thursday
        t_night_1 = t_th_night_start;
        t_night_2 = t_th_night_end;
        Serial.print("today is thursday");
        break;
      case 6: //Friday
        t_night_1 = t_fr_night_start;
        t_night_2 = t_fr_night_end;
        Serial.print("today is friday");
        break;
      case 7: //Saterday
        t_night_1 = t_sa_night_start;
        t_night_2 = t_sa_night_end;
        Serial.print("today is saterday");
        break;
    }
    Serial.print("\n");
    // If nighttime is enabled
    if (en_nighttime) {
      // do hours +1 if summertime enabled
      hoursNighttime = hour();
      if (summertime == 1) hoursNighttime++;

      if (t_night_1 == t_night_2)
        return false;
      // t_1 before mignight, t_2 after midnight
      else if (t_night_1 > t_night_2) {
        if (hoursNighttime >= t_night_1 || hoursNighttime < t_night_2)
          return true;
        else {
          en_nighttime = 1; // Enable nighttime again during the day
          return false;
        }
      }
      // t_1 and t_2 after midnight
      else if (t_night_1 < t_night_2) {
        if (hoursNighttime >= t_night_1 && hoursNighttime < t_night_2)
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
}

////////////////////////////////////////////////////
// Wifimanager
////////////////////////////////////////////////////
void configModeCallback (WiFiManager * myWiFiManager) {
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
  // No connection -> don't try
  if (WiFi.status() == WL_CONNECTED) {
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
        OldTimeSinds1900 = secsSince1900 - 2208988800UL + (timeZone) * SECS_PER_HOUR; // update old time for offline use
        return OldTimeSinds1900;
      }
    }
    Serial.println("No NTP Response :-(");
    OldTimeSinds1900 = OldTimeSinds1900 + SyncTime; // add sync time when we are not online
    return OldTimeSinds1900;
  }
  else {
    Serial.println("Not tried NTP response, were offline :-(");
    // if no connection, don't try -> add sync time
    OldTimeSinds1900 = OldTimeSinds1900 + SyncTime; // add sync time when we are not online
    return OldTimeSinds1900; // return 0 if unable to get the time
  }
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
void LEDTest() {

  byte test_brightness = 150;

  // Clock LEDs
  for (byte n = 0; n <= 3; n++) { // 3 times
    for (byte i = 0; i <= 9; i++) { // rows
      disableAllLED();
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
      delay(250);
    }
  }
  // All remaining LED
  for (byte n = 0; n <= 3; n++) { // 3 times
    disableAllLED();
    for (byte j = 0; j <= 3; j++) { // columns
      switch (n) {
        case 0: // Red
          pixels.setPixelColor(Corners[j] - 1, pixels.Color(test_brightness, 0, 0));
          break;
        case 1: // Green
          pixels.setPixelColor(Corners[j] - 1, pixels.Color(0, test_brightness, 0));
          break;
        case 2: // Blue
          pixels.setPixelColor(Corners[j] - 1, pixels.Color(0, 0, test_brightness));
          break;
        case 3: // White
          pixels.setPixelColor(Corners[j] - 1, pixels.Color(test_brightness, test_brightness, test_brightness));
          break;
      }
    }
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(2000);
  }
  disableAllLED();

}

////////////////////////////////////////////////////
// Display current weekday
////////////////////////////////////////////////////
void showDay() {

  disableClockLED();

  byte day_left = (day() - day() % 10) / 10;
  byte day_right = day() % 10;
  sendNum2LED(numbers_left[day_left]);
  sendNum2LED(numbers_right[day_right]);

  pixels.show();
  delay(2000);

}
////////////////////////////////////////////////////
// Display Ip adres
////////////////////////////////////////////////////
void ShowIp() {
  int FlDelay = 3000;
  int len = 4;
  for (int i = 0; i < len; i++) {
    int ip = WiFi.localIP()[i];
    // Serial.println(ip);
    int x = (ip / 100U) % 10;
    int y = (ip / 10U) % 10;
    int z = (ip / 1U) % 10;
    // when all are 0, or x & y are 0 -> only show last digit
    if ((x == 0 && y==0 && z==0) || ( x == 0 & y == 0)) {
      disableClockLED();
      sendNum2LED(numbers_left[z]);
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(FlDelay);
    }
    // show 3 digits
    else if (x == 0) {
      disableClockLED();
      sendNum2LED(numbers_left[y]);
      sendNum2LED(numbers_right[z]);
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(FlDelay);
    }
    // When there are only 2 digits, skip first one
    else {
      disableClockLED();
      sendNum2LED(numbers_left[x]);
      sendNum2LED(numbers_right[y]);
      pixels.show();
      delay(FlDelay);
      disableClockLED();
      sendNum2LED(numbers_left[z]);
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(FlDelay);
    }
  }
}
////////////////////////////////////////////////////
// Test numbers
////////////////////////////////////////////////////
void testNumbers() {
  for (byte i = 0; i <= 5; i++) {
    for (byte j = 0; j <= 9; j++) {
      disableClockLED();
      sendNum2LED(numbers_left[j]);
      sendNum2LED(numbers_right[j]);
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(500);
    }
  }
}

////////////////////////////////////////////////////
// This function writes a 2 byte integer to the eeprom at
// the specified address and address + 1
////////////////////////////////////////////////////
void EEPROMWriteInt(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}

////////////////////////////////////////////////////
// This function reads a 2 byte integer from the eeprom at
// the specified address and address + 1
////////////////////////////////////////////////////
unsigned int EEPROMReadInt(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

////////////////////////////////////////////////////
// Http site
////////////////////////////////////////////////////
void LoadHttp() {
  if (httpChange) {
    webPage = "";
    // Html website for web server
    webPage += "<h1>Wordclock Web Server</h1>";
    webPage += "<ul><li>Unless stated otherwise, all settings are stored permanently.</li>";
    webPage += "<li>HSV format is used for LEDs colors. Check <a href='http://colorizer.org' target='_blank'>Colorizer</a> (HSV/HSB).</li>";
    webPage += "<li>For brightness calibration, first adjust brightness manually and then select according lightning conditions. One has time for manual brightness adjustment until the clock updates the time, i.e. max. 60 s.</li></ul>";

    // General
    webPage += "<h2>General</h2>";
    webPage += "<p>Restart clock: <a href=\"reset\"><button>Submit</button></a></p>";

    // Clock
    webPage += "<h2>Clock</h2>";
    webPage += "<form action='clock_leds'>Clock LEDs (temporary): <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
    webPage += "<form action='hue_clock'><form> Hue (temporary, 0-360 deg): <input type='number' name='value' min='0' max='360' step='1' value='";
    webPage += t_h_clock;
    webPage += "'><input type='submit' value='Submit'></form></form>";
    webPage += "<form action='sat_clock'><form> Saturation (temporary, 0-100 %): <input type='number' name='value' min='0' max='100' step='1' value='";
    webPage += t_s_clock;
    webPage += "'><input type='submit' value='Submit'></form></form>";
    webPage += "<form action='clock_brightness'><form> clock brightness (temporary, 0-100 %): <input type='number' name='value' min='0' max='100' step='1' value='";
    webPage += t_v_clock;
    webPage += "'><input type='submit' value='Submit'></form></form>";
    webPage += "<p>Save current color permanently: <a href=\"store_color_clock\"><button>Submit</button></a></p>";

    webPage += "<form action='disp_oclock'>Display 'o'clock': <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
    webPage += "<form action='disp_it_is'>Display 'It is': <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
    webPage += "<form action='disp_am_pm'>Display am/pm (English only): <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
    webPage += "<form action='disp_single_min'>Corner LEDs for single minutes: <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";

    // Nighttime
    webPage += "<h2>Night-time</h2>";
    webPage += "<ul><li>All LEDs are switched off during the specified night-time.</li>";
    webPage += "<li>Selecting the same start and end time disables night-time mode.</li></ul>";
    webPage += "<table>";
    webPage += "  <tr>";
    webPage += "    <th>Day</th>";
    webPage += "    <th>Start of night-time (0-23H)</th>";
    webPage += "    <th>End of night-time (0-23H)</th>";
    webPage += "  </tr><tr>";
    webPage += "    <td>Monday</td>";
    webPage += "    <td><form action='mo_night_start'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_mo_night_start;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "    <td><form action='mo_night_end'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_mo_night_end;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "  </tr><tr>";
    webPage += "    <td>Tuesday</td>";
    webPage += "    <td><form action='tu_night_start'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_tu_night_start;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "    <td><form action='tu_night_end'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_tu_night_end;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "  </tr><tr>";
    webPage += "    <td>Wednesday</td>";
    webPage += "    <td><form action='we_night_start'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_we_night_start;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "    <td><form action='we_night_end'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_we_night_end;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "  </tr><tr>";
    webPage += "    <td>Thursday</td>";
    webPage += "    <td><form action='th_night_start'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_th_night_start;
    webPage +=       "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "    <td><form action='th_night_end'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_th_night_end;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "  </tr><tr>";
    webPage += "    <td>Friday</td>";
    webPage += "    <td><form action='fr_night_start'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=       t_fr_night_start;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "    <td><form action='fr_night_end'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_fr_night_end;
    webPage +=       "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "  </tr><tr>";
    webPage += "    <td>Saterday</td>";
    webPage += "    <td><form action='sa_night_start'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_sa_night_start;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "    <td><form action='sa_night_end'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_sa_night_end;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "  </tr><tr>";
    webPage += "    <td>Sunday</td>";
    webPage += "    <td><form action='su_night_start'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_su_night_start;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "    <td><form action='su_night_end'><form><input type='number' name='value' min='0' max='23' step='1' value='";
    webPage +=      t_su_night_end;
    webPage +=      "'><input type='submit' value='Submit'></form></form></td>";
    webPage += "  </tr></table>";

    webPage += "<form action='nighttime_temp'>Night-time (temporary): <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form></p>";
    webPage += "<form action='summertime'><form> Summertime: <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";

    // Other settings
    webPage += "<h2>Other settings</h2>";
    webPage += "<p>LED test: <a href=\"LEDTest\"><button>Submit</button></a></p>";
    webPage += "Show current date (day): <a href=\"day_of_month\"><button>Submit</button></a>";
    webPage += "<p>Reset wifi, Warning password will be lost!: <a href=\"resetWifi\"><button>Submit</button></a></p>";
    Serial.println("reloaded HTTP site");
    Serial.println(settings_changed);
    httpChange = false;
  }
}
