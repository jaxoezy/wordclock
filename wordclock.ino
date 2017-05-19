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
extern const uint16_t LDR_corr_log[];
extern const uint16_t LDR_corr_sqrt[];

// Set Pins
#define PIN D1 // LED data pin
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

RGBConverter rgb_conv;
byte clock_rgb[3];
byte ambilight_rgb[3];
double hsv_value_inc = 0.05;

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
int min_LDR_value;
int max_LDR_value;
byte LDR_corr_type;
byte en_it_is;
byte en_oclock;
byte en_single_min;
byte en_ambilight;
boolean settings_changed = false;
int language;
byte en_clock = 1;
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
const int EEPROM_addr_min_user_brightness = 0;
const int EEPROM_addr_LDR_min = 1; // needs two bytes
const int EEPROM_addr_max_user_brightness = 3;
const int EEPROM_addr_LDR_max = 4; // needs two bytes
const int EEPROM_addr_es_ist = 6;
const int EEPROM_addr_uhr = 7;
const int EEPROM_addr_single_min = 8;
const int EEPROM_addr_ambilight = 9;
const int EEPROM_addr_language = 10;
const int EEPROM_addr_t_night_1 = 11;
const int EEPROM_addr_t_night_2 = 12;
const int EEPROM_addr_h_clock = 13;
const int EEPROM_addr_s_clock = 14;
const int EEPROM_addr_v_clock = 15;
const int EEPROM_addr_h_ambilight = 16;
const int EEPROM_addr_s_ambilight = 17;
const int EEPROM_addr_v_ambilight = 18;
const int EEPROM_addr_LDR_corr_type = 19;

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
  //  EEPROM.write(EEPROM_addr_min_user_brightness, 45); // Min brightness set by user
  //  EEPROMWriteInt(EEPROM_addr_LDR_min, 124); // Correspondig LDR value
  //  EEPROM.write(EEPROM_addr_max_user_brightness, 100); // Max brightness set by user
  //  EEPROMWriteInt(EEPROM_addr_LDR_max, 912); // Correspondig LDR value
  //  EEPROM.write(EEPROM_addr_es_ist, 1); // "Es ist", default: on
  //  EEPROM.write(EEPROM_addr_uhr, 1); // "Uhr", default: on
  //  EEPROM.write(EEPROM_addr_single_min, 1); // Display singles minutes, default: on
  //  EEPROM.write(EEPROM_addr_ambilight, 0); // Ambilight, default: off
  //  EEPROM.write(EEPROM_addr_language, 1); // Language: 0: German, 1: English, default: German
  //  EEPROM.write(EEPROM_addr_t_night_1, 1); // Starting hour of nighttime, default: 1 am
  //  EEPROM.write(EEPROM_addr_t_night_2, 7); // Ending hour of nighttime, default: 7 am
  //  EEPROM.write(EEPROM_addr_h_clock, 8); // Hue LED clock, default: 0
  //  EEPROM.write(EEPROM_addr_s_clock, 100); // Saturation LED clock, default: 100
  //  EEPROM.write(EEPROM_addr_v_clock, 75); // Value LED clock, default: 0
  //  EEPROM.write(EEPROM_addr_h_ambilight, 8); // Hue LED ambilight, default: 0
  //  EEPROM.write(EEPROM_addr_s_ambilight, 100); // Saturaion LED ambilight, default: 100
  //  EEPROM.write(EEPROM_addr_v_ambilight, 75); // Value LED ambilight, default: 0
  //  EEPROM.write(EEPROM_addr_LDR_corr_type, 0); // LDR correction type, default: 0
  //  EEPROM.commit();

  // Read settings from EEPROM
  min_user_brightness = EEPROM.read(EEPROM_addr_min_user_brightness);
  min_user_brightness = min_user_brightness / 100;
  max_user_brightness = EEPROM.read(EEPROM_addr_max_user_brightness);
  min_user_brightness = max_user_brightness / 100;
  min_user_brightness = constrain(min_user_brightness, 0, 1);
  max_user_brightness = constrain(max_user_brightness, 0, 1);
  min_LDR_value = EEPROMReadInt(EEPROM_addr_LDR_min);
  max_LDR_value = EEPROMReadInt(EEPROM_addr_LDR_max);
  en_it_is = EEPROM.read(EEPROM_addr_es_ist);
  en_oclock = EEPROM.read(EEPROM_addr_uhr);
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
  LDR_corr_type = EEPROM.read(EEPROM_addr_LDR_corr_type);

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

  // html website for web server
  webPage += "<h1>Wordclock web server</h1>";
  webPage += "<p>Unless stated otherwise, all settings are stored permanently.</p>";

  webPage += "<h2>General</h2>";
  webPage += "<p>Reset: <a href=\"reset\"><button>Submit</button></a></p>";
  webPage += "<form action='leds_on_off'>Clock LEDs (temporary): <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
  webPage += "<form action='ambilight'>Ambilight: <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";

  // LED color & brightness
  webPage += "<h2>LED color & brightness</h2>";
  webPage += "<p>HSV format is used for LEDs colors. Check <a href='http://colorizer.org' target='_blank'>Colorizer</a> (HSV/HSB).<br>";
  webPage += "For brightness calibration, first adjust brightness manually and then select according lightning conditions.</p>";
  webPage += "<form action='hue'><form> Hue (temporary, 0-360 deg): <input type='number' name='value' min='0' max='360' step='1' value='0'><input type='submit' value='Submit'></form></form>";
  webPage += "<form action='sat'><form> Saturation (temporary, 0-100 %): <input type='number' name='value' min='0' max='100' step='1' value='100'><input type='submit' value='Submit'></form></form>";
  webPage += "<p>Save current color permanently: <a href=\"store_color\"><button>Submit</button></a></p>";
  webPage += "<form action='brightness'>Brightness (temporary, 5 % step): <input type='radio' name='state' value='1'>Increase <input type='radio' name='state' value='0'>Decrease <input type='submit' value='Submit'></form>";
  webPage += "<form action='calib_brightness'>Calibrate brightness: <input type='radio' name='state' value='1'>Bright room <input type='radio' name='state' value='0'>Dark room <input type='submit' value='Submit'></form>";
  webPage += "<p>LED test: <a href=\"led_test\"><button>Submit</button></a></p>";
  webPage += "<form action='LDR_corr_type'>Correction light measurement: <select name='LDR_corr_type'><option value='0'>None (linear)</option><option value='1'>Square root</option><option value='2'>Logarithmic</option></select><input type='submit' value='Submit'></form>";

  // Nighttime
  webPage += "<h2>Night-time</h2>";
  webPage += "<p>The LEDs are switched off during the specified night-time.<br>";
  webPage += "Selecting the same start and end time disables night-time mode.</p>";
  webPage += "<form action='night_start'><form> Start of night-time (0-23h): <input type='number' name='value' min='0' max='23' step='1' value='1'><input type='submit' value='Submit'></form></form>";
  webPage += "<form action='night_end'><form> End of night-time (0-23h): <input type='number' name='value' min='0' max='23' step='1' value='7'><input type='submit' value='Submit'></form></form>";
  webPage += "<form action='disable_nighttime_temp'>Night-time (temporary): <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form></p>";

  // Other settings
  webPage += "<h2>Other settings</h2>";
  webPage += "<form action='language'>Language: <select name='language'><option value='0'>German</option><option value='1'>English</option></select><input type='submit' value='Submit'></form>";
  webPage += "<form action='disp_oclock'>Display 'o'clock': <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
  webPage += "<form action='disp_it_is'>Display 'It is': <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
  webPage += "<form action='disp_single_min'>Corner LEDs for single minutes: <input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0'>Off <input type='submit' value='Submit'></form>";
  webPage += "Show current date (day): <a href=\"day_of_month\"><button>Submit</button></a>";

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

  // Connect to wordlock via wordclock.local
  if (mdns.begin("wordclock", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  server.on("/", []() {
    server.send(200, "text/html", webPage);
  });

  /////////////////////////////////////
  // Server: General
  /////////////////////////////////////

  // Reset
  server.on("/reset", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Resetting word clock");
    ESP.reset();
  });

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

  // Enable/Disable ambilight
  server.on("/ambilight", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Enable ambilight");
      if (!en_ambilight) {
        EEPROM.write(EEPROM_addr_ambilight, 1);
        EEPROM.commit();
        en_ambilight = true;
        ambilight();
      }
    }
    else {
      Serial.println("Disable ambilight");
      if (en_ambilight) {
        EEPROM.write(EEPROM_addr_ambilight, 0);
        EEPROM.commit();
        en_ambilight = false;
        disable_ambilight();
      }
    }
    delay(1000);
  });

  /////////////////////////////////////
  // Server: LED color & brightness
  /////////////////////////////////////

  // Select hue
  server.on("/hue", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    state = map(state, 0, 360, 0, 100);
    h_clock = double(state) / 100;
    h_ambilight = h_clock;
    // Convert to RGB
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);
    settings_changed = true;
    Serial.print("LED hue is now: ");
    Serial.print(h_clock);
    Serial.println(" / 1");
    delay(1000);
  });

  // Select saturation
  server.on("/sat", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    s_clock = (double) state / 100;
    s_ambilight = s_clock;
    // Convert to RGB
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);
    settings_changed = true;
    Serial.print("LED saturation is now: ");
    Serial.print(s_clock);
    Serial.println(" / 1");
    delay(1000);
  });

  // Store current color to EEPROM
  server.on("/store_color", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Saving hue and saturation values in EEPROM");
    byte temp = (int) (h_clock * 100);
    EEPROM.write(EEPROM_addr_h_clock, temp);
    Serial.print("Hue: ");
    Serial.println(temp);
    temp = (int) (s_clock * 100);
    EEPROM.write(EEPROM_addr_s_clock, temp);
    Serial.print("Saturation: ");
    Serial.println(temp);
    EEPROM.commit();
    delay(1000);
  });

  // Set brightness
  server.on("/brightness", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();

    if (state == 1) {
      Serial.print("Increasing LED brightness to ");
      v_clock += hsv_value_inc;
    }
    else {
      Serial.print("Decreasing LED brightness to ");
      v_clock -= hsv_value_inc;
    }

    v_clock = constrain(v_clock, 0, 1);
    v_ambilight = v_clock;
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);
    Serial.print(v_clock);
    Serial.println("");
    settings_changed = true;
    delay(1000);
  });

  // Calibrate bright/dark room
  server.on("/calib_brightness", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Calibrate brightness for bright room");
      set_max_brightness();
    }
    else {
      Serial.println("Calibrate brightness for dark room");
      set_min_brightness();
    }
    settings_changed = true;
    delay(1000);
  });

  // LED test
  server.on("/led_test", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Testing all LEDs");
    LED_test();
    settings_changed = true;
    delay(1000);
  });

  // Set language
  server.on("/LDR_corr_type", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("LDR_corr_type").toInt();
    LDR_corr_type = state;
    EEPROM.write(EEPROM_addr_LDR_corr_type, LDR_corr_type);
    EEPROM.commit();
    settings_changed = true;
    delay(1000);
  });
  
  /////////////////////////////////////
  // Server: Night-time
  /////////////////////////////////////

  // Start of night-time
  server.on("/night_start", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_night_1 = state;
    EEPROM.write(EEPROM_addr_t_night_1, t_night_1);
    EEPROM.commit();
    Serial.print("Starting night-time at: ");
    Serial.print(t_night_1);
    Serial.println(" h");
    delay(1000);
  });

  // End of night-time
  server.on("/night_end", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("value").toInt();
    t_night_2 = state;
    EEPROM.write(EEPROM_addr_t_night_2, t_night_2);
    EEPROM.commit();
    Serial.print("Ending night-time at: ");
    Serial.print(t_night_2);
    Serial.println(" h");
    delay(1000);
  });

  // Enable/Disable night-time temporarily
  server.on("/disable_nighttime_temp", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
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

  /////////////////////////////////////
  // Server: Other settings
  /////////////////////////////////////

  // Set language
  server.on("/language", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("language").toInt();
    if (state == 0) {
      Serial.println("Language: German");
      if (language) {
        EEPROM.write(EEPROM_addr_language, 0);
        EEPROM.commit();
        language = false;
        settings_changed = true;
      }
    }
    else {
      if (!language) {
        EEPROM.write(EEPROM_addr_language, 1);
        EEPROM.commit();
        language = true;
        settings_changed = true;
      }
    }
    delay(1000);
  });

  // Enable/Disable "o'clock"
  server.on("/disp_oclock", []() {
    server.send(200, "text/html", webPage);
    int state = server.arg("state").toInt();
    if (state == 1) {
      Serial.println("Display 'o'clock' enabled");
      if (!en_oclock) {
        EEPROM.write(EEPROM_addr_uhr, 1);
        EEPROM.commit();
        en_oclock = true;
        settings_changed = true;
      }
    }
    else {
      Serial.println("Display 'o'clock' disabled");
      if (en_oclock) {
        EEPROM.write(EEPROM_addr_uhr, 0);
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
        EEPROM.write(EEPROM_addr_es_ist, 1);
        EEPROM.commit();
        en_it_is = true;
        settings_changed = true;
      }
    }
    else {
      Serial.println("Display 'it is' disabled");
      if (en_it_is) {
        EEPROM.write(EEPROM_addr_es_ist, 0);
        EEPROM.commit();
        en_it_is = false;
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

  // Show current day of month
  server.on("/day_of_month", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Displaying day of month");
    show_day();
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

  if (WiFi.status() == WL_CONNECTED) {

    // Handle Webserver
    server.handleClient();

    // Execute everything only if minutes or settings have changed
    if (timeStatus() != timeNotSet) {
      if ((minute() != prevDisplay || settings_changed)) { // Alternative: now()

        // Check whether nighttime is active
        if (!nighttime() && en_clock) {

          // Determine LED brightness based on LDR measurement
          get_brightness();

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
    else {
      Serial.println("Time not set");
    }
  }
  else
    ESP.restart();
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

  // Output time depending on language
  switch (language) {

    // German
    case 0:

      // Display "es ist"
      if (en_it_is)
        send_time_2_LED(es_ist);

      switch (min_five) {
        case 0:
          send_time_2_LED(full_hours_GER[hours]);
          if (en_oclock) send_time_2_LED(uhr);
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
      if (en_it_is)
        send_time_2_LED(it_is);

      switch (min_five) {
        case 0:
          send_time_2_LED(hours_EN[hours]);
          if (en_oclock) send_time_2_LED(oclock);
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

  for (byte i = 0; i <= 6; i++) {
    if (x[i] != 0)
      // pixels.setPixelColor(x[i] - 1, pixels.Color(R_clock, G_clock, B_clock));
      pixels.setPixelColor(x[i] - 1, pgm_read_byte(&gamma8[clock_rgb[0]]), pgm_read_byte(&gamma8[clock_rgb[1]]), pgm_read_byte(&gamma8[clock_rgb[2]]));
  }

  //Serial.print("LED color: H: ");
  //Serial.print(h_clock);
  //Serial.print(", S: ");
  //Serial.print(s_clock);
  //Serial.print(", V: ");
  //Serial.println(v_clock);
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
// If t_night_1 = t_night_2, then "false" is always returned
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
void get_brightness() {

  // Read LDR
  int sensorValue = analogRead(analogInPin); // Value between 0 and 1023
  sensorValue = constrain(sensorValue, 0, 1023);
  sensorValue = LDR_correction(sensorValue);
  sensorValue = constrain(sensorValue, min_LDR_value, max_LDR_value);

  // Map sensor value
  v_clock = map(sensorValue, min_LDR_value, max_LDR_value, min_user_brightness * 100, max_user_brightness * 100);
  v_clock = v_clock / 100;
  v_ambilight = v_clock;

  // Convert to RGB
  rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
  rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);

  // Print sensor value to serial monitor
  Serial.print("Sensor value: ");
  Serial.print(sensorValue);
  Serial.print("/1024, Brightness: ");
  Serial.print(", Brightness: ");
  Serial.print(v_clock);
  Serial.println("/1");
  //  Serial.print("Min LDR: ");
  //  Serial.print(min_LDR_value);
  //  Serial.print(", Max LDR: ");
  //  Serial.println(max_LDR_value);
  //  Serial.print("Min User: ");
  //  Serial.print(min_user_brightness);
  //  Serial.print(", Max User: ");
  //  Serial.print(max_user_brightness);

}

////////////////////////////////////////////////////
// Set minimum brightness
////////////////////////////////////////////////////
void set_min_brightness() {

  // Read the analog in value
  int sensorValue = analogRead(analogInPin); // Value between 0 and 1023
  sensorValue = constrain(sensorValue, 0, 1023);
  sensorValue = LDR_correction(sensorValue);

  byte temp = (int) (v_clock * 100);
  EEPROM.write(EEPROM_addr_min_user_brightness, temp); // Min brightness set by user
  EEPROMWriteInt(EEPROM_addr_LDR_min, sensorValue); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Set maximum brightness
////////////////////////////////////////////////////
void set_max_brightness() {
  
  // Read the analog in value
  int sensorValue = analogRead(analogInPin); // Value between 0 and 1023
  sensorValue = constrain(sensorValue, 0, 1023);
  sensorValue = LDR_correction(sensorValue);

  byte temp = (int) (v_clock * 100);
  EEPROM.write(EEPROM_addr_max_user_brightness, temp); // Max brightness set by user
  EEPROMWriteInt(EEPROM_addr_LDR_max, sensorValue); // Correspondig LDR value
  EEPROM.commit();
  
}

////////////////////////////////////////////////////
// LDR correction
////////////////////////////////////////////////////
int LDR_correction(int sensorValue) {
  
  switch (LDR_corr_type) {
  
  case 0: // None
    return sensorValue;
    break;  
  
  case 1: // Square root
    return pgm_read_word(&LDR_corr_sqrt[sensorValue]);
    break;

  case 2: // Logarithmic
    return pgm_read_word(&LDR_corr_log[sensorValue]);
    break;
  }
  
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

// LED gamma correction
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

// LDR correction (logarithm)
// Function: y = log10(x+1)*1023/log10(1023+1);
const uint16_t PROGMEM LDR_corr_log[] = {
0, 102, 162, 204, 237, 264, 287, 306, 324, 339, 353, 366, 378, 389, 399, 409,
418, 426, 434, 442, 449, 456, 462, 469, 475, 480, 486, 491, 496, 501, 506, 511,
516, 520, 524, 528, 532, 536, 540, 544, 548, 551, 555, 558, 561, 565, 568, 571,
574, 577, 580, 583, 585, 588, 591, 594, 596, 599, 601, 604, 606, 609, 611, 613,
616, 618, 620, 622, 624, 627, 629, 631, 633, 635, 637, 639, 641, 642, 644, 646,
648, 650, 652, 653, 655, 657, 659, 660, 662, 664, 665, 667, 668, 670, 672, 673,
675, 676, 678, 679, 681, 682, 684, 685, 686, 688, 689, 691, 692, 693, 695, 696,
697, 699, 700, 701, 702, 704, 705, 706, 707, 709, 710, 711, 712, 713, 714, 716,
717, 718, 719, 720, 721, 722, 723, 725, 726, 727, 728, 729, 730, 731, 732, 733,
734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749,
749, 750, 751, 752, 753, 754, 755, 756, 757, 757, 758, 759, 760, 761, 762, 763,
763, 764, 765, 766, 767, 768, 768, 769, 770, 771, 772, 772, 773, 774, 775, 775,
776, 777, 778, 778, 779, 780, 781, 781, 782, 783, 784, 784, 785, 786, 787, 787,
788, 789, 789, 790, 791, 791, 792, 793, 794, 794, 795, 796, 796, 797, 798, 798,
799, 800, 800, 801, 801, 802, 803, 803, 804, 805, 805, 806, 807, 807, 808, 808,
809, 810, 810, 811, 811, 812, 813, 813, 814, 814, 815, 816, 816, 817, 817, 818,
818, 819, 820, 820, 821, 821, 822, 822, 823, 824, 824, 825, 825, 826, 826, 827,
827, 828, 828, 829, 830, 830, 831, 831, 832, 832, 833, 833, 834, 834, 835, 835,
836, 836, 837, 837, 838, 838, 839, 839, 840, 840, 841, 841, 842, 842, 843, 843,
844, 844, 845, 845, 846, 846, 847, 847, 848, 848, 849, 849, 849, 850, 850, 851,
851, 852, 852, 853, 853, 854, 854, 854, 855, 855, 856, 856, 857, 857, 858, 858,
858, 859, 859, 860, 860, 861, 861, 862, 862, 862, 863, 863, 864, 864, 864, 865,
865, 866, 866, 867, 867, 867, 868, 868, 869, 869, 869, 870, 870, 871, 871, 871,
872, 872, 873, 873, 873, 874, 874, 875, 875, 875, 876, 876, 877, 877, 877, 878,
878, 879, 879, 879, 880, 880, 880, 881, 881, 882, 882, 882, 883, 883, 883, 884,
884, 885, 885, 885, 886, 886, 886, 887, 887, 887, 888, 888, 888, 889, 889, 890,
890, 890, 891, 891, 891, 892, 892, 892, 893, 893, 893, 894, 894, 894, 895, 895,
895, 896, 896, 896, 897, 897, 897, 898, 898, 899, 899, 899, 900, 900, 900, 900,
901, 901, 901, 902, 902, 902, 903, 903, 903, 904, 904, 904, 905, 905, 905, 906,
906, 906, 907, 907, 907, 908, 908, 908, 909, 909, 909, 909, 910, 910, 910, 911,
911, 911, 912, 912, 912, 913, 913, 913, 913, 914, 914, 914, 915, 915, 915, 916,
916, 916, 916, 917, 917, 917, 918, 918, 918, 918, 919, 919, 919, 920, 920, 920,
920, 921, 921, 921, 922, 922, 922, 922, 923, 923, 923, 924, 924, 924, 924, 925,
925, 925, 926, 926, 926, 926, 927, 927, 927, 928, 928, 928, 928, 929, 929, 929,
929, 930, 930, 930, 930, 931, 931, 931, 932, 932, 932, 932, 933, 933, 933, 933,
934, 934, 934, 934, 935, 935, 935, 936, 936, 936, 936, 937, 937, 937, 937, 938,
938, 938, 938, 939, 939, 939, 939, 940, 940, 940, 940, 941, 941, 941, 941, 942,
942, 942, 942, 943, 943, 943, 943, 944, 944, 944, 944, 945, 945, 945, 945, 946,
946, 946, 946, 947, 947, 947, 947, 947, 948, 948, 948, 948, 949, 949, 949, 949,
950, 950, 950, 950, 951, 951, 951, 951, 952, 952, 952, 952, 952, 953, 953, 953,
953, 954, 954, 954, 954, 955, 955, 955, 955, 955, 956, 956, 956, 956, 957, 957,
957, 957, 957, 958, 958, 958, 958, 959, 959, 959, 959, 959, 960, 960, 960, 960,
961, 961, 961, 961, 961, 962, 962, 962, 962, 963, 963, 963, 963, 963, 964, 964,
964, 964, 964, 965, 965, 965, 965, 966, 966, 966, 966, 966, 967, 967, 967, 967,
967, 968, 968, 968, 968, 968, 969, 969, 969, 969, 969, 970, 970, 970, 970, 971,
971, 971, 971, 971, 972, 972, 972, 972, 972, 973, 973, 973, 973, 973, 974, 974,
974, 974, 974, 975, 975, 975, 975, 975, 976, 976, 976, 976, 976, 977, 977, 977,
977, 977, 978, 978, 978, 978, 978, 978, 979, 979, 979, 979, 979, 980, 980, 980,
980, 980, 981, 981, 981, 981, 981, 982, 982, 982, 982, 982, 983, 983, 983, 983,
983, 983, 984, 984, 984, 984, 984, 985, 985, 985, 985, 985, 986, 986, 986, 986,
986, 986, 987, 987, 987, 987, 987, 988, 988, 988, 988, 988, 988, 989, 989, 989,
989, 989, 990, 990, 990, 990, 990, 990, 991, 991, 991, 991, 991, 991, 992, 992,
992, 992, 992, 993, 993, 993, 993, 993, 993, 994, 994, 994, 994, 994, 994, 995,
995, 995, 995, 995, 996, 996, 996, 996, 996, 996, 997, 997, 997, 997, 997, 997,
998, 998, 998, 998, 998, 998, 999, 999, 999, 999, 999, 999, 1000, 1000, 1000, 1000,
1000, 1000, 1001, 1001, 1001, 1001, 1001, 1001, 1002, 1002, 1002, 1002, 1002, 1002, 1003, 1003,
1003, 1003, 1003, 1003, 1004, 1004, 1004, 1004, 1004, 1004, 1005, 1005, 1005, 1005, 1005, 1005,
1006, 1006, 1006, 1006, 1006, 1006, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1008, 1008, 1008,
1008, 1008, 1008, 1009, 1009, 1009, 1009, 1009, 1009, 1010, 1010, 1010, 1010, 1010, 1010, 1010,
1011, 1011, 1011, 1011, 1011, 1011, 1012, 1012, 1012, 1012, 1012, 1012, 1013, 1013, 1013, 1013,
1013, 1013, 1013, 1014, 1014, 1014, 1014, 1014, 1014, 1015, 1015, 1015, 1015, 1015, 1015, 1015,
1016, 1016, 1016, 1016, 1016, 1016, 1016, 1017, 1017, 1017, 1017, 1017, 1017, 1018, 1018, 1018,
1018, 1018, 1018, 1018, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1020, 1020, 1020, 1020, 1020,
1020, 1020, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1022, 1022, 1022, 1022, 1022, 1022, 1023
};

// LDR correction (sqrt)
// Function: y = sqrt(x)*1023/sqrt(x);
const uint16_t PROGMEM LDR_corr_sqrt[] = {
  0, 31, 45, 55, 63, 71, 78, 84, 90, 95, 101, 106, 110, 115, 119, 123,
  127, 131, 135, 139, 143, 146, 150, 153, 156, 159, 163, 166, 169, 172, 175, 178,
  180, 183, 186, 189, 191, 194, 197, 199, 202, 204, 207, 209, 212, 214, 216, 219,
  221, 223, 226, 228, 230, 232, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253,
  255, 257, 259, 261, 263, 265, 267, 269, 271, 273, 275, 276, 278, 280, 282, 284,
  286, 287, 289, 291, 293, 294, 296, 298, 300, 301, 303, 305, 306, 308, 310, 311,
  313, 315, 316, 318, 319, 321, 323, 324, 326, 327, 329, 330, 332, 333, 335, 336,
  338, 339, 341, 342, 344, 345, 347, 348, 350, 351, 353, 354, 356, 357, 359, 360,
  361, 363, 364, 366, 367, 368, 370, 371, 372, 374, 375, 377, 378, 379, 381, 382,
  383, 385, 386, 387, 389, 390, 391, 393, 394, 395, 396, 398, 399, 400, 402, 403,
  404, 405, 407, 408, 409, 410, 412, 413, 414, 415, 417, 418, 419, 420, 421, 423,
  424, 425, 426, 427, 429, 430, 431, 432, 433, 435, 436, 437, 438, 439, 440, 442,
  443, 444, 445, 446, 447, 448, 450, 451, 452, 453, 454, 455, 456, 457, 459, 460,
  461, 462, 463, 464, 465, 466, 467, 468, 470, 471, 472, 473, 474, 475, 476, 477,
  478, 479, 480, 481, 482, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494,
  495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510,
  511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526,
  527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 539, 540, 541,
  542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 553, 554, 555, 556,
  557, 558, 559, 560, 561, 562, 563, 564, 564, 565, 566, 567, 568, 569, 570, 571,
  572, 573, 573, 574, 575, 576, 577, 578, 579, 580, 581, 581, 582, 583, 584, 585,
  586, 587, 588, 588, 589, 590, 591, 592, 593, 594, 594, 595, 596, 597, 598, 599,
  600, 600, 601, 602, 603, 604, 605, 606, 606, 607, 608, 609, 610, 611, 611, 612,
  613, 614, 615, 616, 616, 617, 618, 619, 620, 621, 621, 622, 623, 624, 625, 625,
  626, 627, 628, 629, 630, 630, 631, 632, 633, 634, 634, 635, 636, 637, 638, 638,
  639, 640, 641, 642, 642, 643, 644, 645, 646, 646, 647, 648, 649, 649, 650, 651,
  652, 653, 653, 654, 655, 656, 657, 657, 658, 659, 660, 660, 661, 662, 663, 664,
  664, 665, 666, 667, 667, 668, 669, 670, 670, 671, 672, 673, 673, 674, 675, 676,
  676, 677, 678, 679, 679, 680, 681, 682, 682, 683, 684, 685, 685, 686, 687, 688,
  688, 689, 690, 691, 691, 692, 693, 694, 694, 695, 696, 697, 697, 698, 699, 700,
  700, 701, 702, 702, 703, 704, 705, 705, 706, 707, 708, 708, 709, 710, 710, 711,
  712, 713, 713, 714, 715, 715, 716, 717, 718, 718, 719, 720, 720, 721, 722, 723,
  723, 724, 725, 725, 726, 727, 727, 728, 729, 730, 730, 731, 732, 732, 733, 734,
  734, 735, 736, 737, 737, 738, 739, 739, 740, 741, 741, 742, 743, 743, 744, 745,
  745, 746, 747, 748, 748, 749, 750, 750, 751, 752, 752, 753, 754, 754, 755, 756,
  756, 757, 758, 758, 759, 760, 760, 761, 762, 762, 763, 764, 764, 765, 766, 766,
  767, 768, 768, 769, 770, 770, 771, 772, 772, 773, 774, 774, 775, 776, 776, 777,
  778, 778, 779, 780, 780, 781, 782, 782, 783, 784, 784, 785, 786, 786, 787, 788,
  788, 789, 789, 790, 791, 791, 792, 793, 793, 794, 795, 795, 796, 797, 797, 798,
  798, 799, 800, 800, 801, 802, 802, 803, 804, 804, 805, 805, 806, 807, 807, 808,
  809, 809, 810, 811, 811, 812, 812, 813, 814, 814, 815, 816, 816, 817, 817, 818,
  819, 819, 820, 821, 821, 822, 822, 823, 824, 824, 825, 826, 826, 827, 827, 828,
  829, 829, 830, 830, 831, 832, 832, 833, 834, 834, 835, 835, 836, 837, 837, 838,
  838, 839, 840, 840, 841, 841, 842, 843, 843, 844, 845, 845, 846, 846, 847, 848,
  848, 849, 849, 850, 851, 851, 852, 852, 853, 854, 854, 855, 855, 856, 857, 857,
  858, 858, 859, 860, 860, 861, 861, 862, 862, 863, 864, 864, 865, 865, 866, 867,
  867, 868, 868, 869, 870, 870, 871, 871, 872, 873, 873, 874, 874, 875, 875, 876,
  877, 877, 878, 878, 879, 880, 880, 881, 881, 882, 882, 883, 884, 884, 885, 885,
  886, 886, 887, 888, 888, 889, 889, 890, 890, 891, 892, 892, 893, 893, 894, 894,
  895, 896, 896, 897, 897, 898, 898, 899, 900, 900, 901, 901, 902, 902, 903, 904,
  904, 905, 905, 906, 906, 907, 908, 908, 909, 909, 910, 910, 911, 911, 912, 913,
  913, 914, 914, 915, 915, 916, 917, 917, 918, 918, 919, 919, 920, 920, 921, 922,
  922, 923, 923, 924, 924, 925, 925, 926, 926, 927, 928, 928, 929, 929, 930, 930,
  931, 931, 932, 933, 933, 934, 934, 935, 935, 936, 936, 937, 937, 938, 939, 939,
  940, 940, 941, 941, 942, 942, 943, 943, 944, 945, 945, 946, 946, 947, 947, 948,
  948, 949, 949, 950, 950, 951, 952, 952, 953, 953, 954, 954, 955, 955, 956, 956,
  957, 957, 958, 958, 959, 960, 960, 961, 961, 962, 962, 963, 963, 964, 964, 965,
  965, 966, 966, 967, 968, 968, 969, 969, 970, 970, 971, 971, 972, 972, 973, 973,
  974, 974, 975, 975, 976, 976, 977, 978, 978, 979, 979, 980, 980, 981, 981, 982,
  982, 983, 983, 984, 984, 985, 985, 986, 986, 987, 987, 988, 988, 989, 989, 990,
  990, 991, 992, 992, 993, 993, 994, 994, 995, 995, 996, 996, 997, 997, 998, 998,
  999, 999, 1000, 1000, 1001, 1001, 1002, 1002, 1003, 1003, 1004, 1004, 1005, 1005, 1006, 1006,
  1007, 1007, 1008, 1008, 1009, 1009, 1010, 1010, 1011, 1011, 1012, 1012, 1013, 1013, 1014, 1014,
  1015, 1015, 1016, 1016, 1017, 1017, 1018, 1018, 1019, 1019, 1020, 1020, 1021, 1021, 1022, 1023
};

