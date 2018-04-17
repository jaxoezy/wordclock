
#include <Adafruit_NeoPixel.h>
//#include <TimeLib.h>
//#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
//#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <EEPROM.h>
//#include <ArduinoJson.h>
//#include <DNSServer.h>
//#include <WiFiManager.h>
#include <RGBConverter.h>
#include <Wire.h>
#include "RTClib.h"
#include <Bounce2.h>

RTC_DS3231 rtc;
const int sclPin = D1;
const int sdaPin = D2;

// Set Pins
#define PIN D5 // LED data pin
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

// Initialize LEDs
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(256, PIN, NEO_GRB + NEO_KHZ800);
const int MAX_NUM_LEDS = 170;
RGBConverter rgb_conv;
byte clock_rgb[3];
byte ambilight_rgb[3];
double hsv_value_inc = 0.05;
extern const uint8_t gamma8[];
extern const uint16_t LDR_corr_log[];
extern const uint16_t LDR_corr_sqrt[];
extern const uint16_t LDR_corr_square[];

#define NUM_BUTTONS 4
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {2, 0, 12, 13}; // Pins D4, D3, D6, D7
Bounce * buttons = new Bounce[NUM_BUTTONS];

// German
byte es_ist[7] =      {1, 2, 4, 5, 6, 0, 0};
byte fuenf_min[7] =   {8, 9, 10, 11, 0, 0, 0};
byte zehn_min[7] =    {19, 20, 21, 22, 0, 0, 0};
byte viertel_min[7] = {27, 28, 29, 30, 31, 32, 33};
byte zwanzig_min[7] = {12, 13, 14, 15, 16, 17, 18};
byte nach[7] =        {34, 35, 36, 37, 0, 0, 0};
byte vor[7] =         {42, 43, 44, 0, 0, 0, 0};
byte halb[7] =        {45, 46, 47, 48, 0, 0, 0};
byte ein_std[7] =     {66, 65, 64, 0, 0, 0, 0};
byte eins_std[7] =    {66, 65, 64, 63, 0, 0, 0};
byte zwei_std[7] =    {56, 57, 58, 59, 0, 0, 0};
byte drei_std[7] =    {67, 68, 69, 70, 0, 0, 0};
byte vier_std[7] =    {74, 75, 76, 77, 0, 0, 0};
byte fuenf_std[7] =   {52, 53, 54, 55, 0, 0, 0};
byte sechs_std[7] =   {88, 87, 86, 85, 84, 0, 0};
byte sieben_std[7] =  {89, 90, 91, 92, 93, 94, 0};
byte acht_std[7] =    {78, 79, 80, 81, 0, 0, 0};
byte neun_std[7] =    {107, 106, 105, 104, 0, 0, 0};
byte zehn_std[7] =    {110, 109, 108, 107, 0, 0, 0};
byte elf_std[7] =     {50, 51, 52, 0, 0, 0, 0};
byte zwoelf_std[7] =  {95, 96, 97, 98, 99, 0, 0};
byte null_std[7] =    {95, 96, 97, 98, 99, 0, 0}; // Keine null Uhr mehr?
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

byte confirm[17] = {32, 36, 52, 60, 72, 84, 70, 64, 12, 46, 0, 0, 0, 0, 0, 0, 0};
byte back[17] = {31, 37, 51, 61, 73, 81, 97, 71, 85, 91, 49, 41, 25, 21, 13, 101, 109};

// Characters
byte text_D[17] = {24, 25, 26, 43, 40, 46, 49, 65, 62, 68, 71, 87, 86, 85, 0, 0, 0};
byte text_E_right[17] = {29, 30, 31, 32, 38, 51, 52, 53, 54, 60, 73, 82, 81, 80, 79, 0, 0};
byte text_E_left[17] = {24, 25, 26, 27, 43, 46, 47, 48, 49, 65, 68, 87, 86, 85, 84, 0, 0};
byte text_O_small[17] = {25, 43, 41, 46, 48, 65, 63, 68, 70, 86, 0, 0, 0, 0, 0, 0, 0};
byte text_O[17] = {25, 26, 43, 40, 46, 49, 65, 62, 68, 71, 86, 85, 0, 0, 0, 0, 0};
byte text_ff[17] = {28, 29, 39, 50, 51, 61, 72, 83, 31, 32, 36, 53, 54, 58, 75, 80, 0};
byte text_N[17] = {29, 32, 38, 35, 51, 52, 54, 60, 58, 57, 73, 76, 82, 79, 0, 0, 0};

// LED matrix indices
byte LED_matrix[10][11];
byte row_offset[10] = {1, 22, 23, 44, 45, 66, 67, 88, 89, 110};

int hours = 10;
int minutes = 25;
byte min_five = 0;
byte single_min = 0;

byte days;
byte months;
int years;
float temp_rtc;

double min_user_clock_brightness;
double max_user_clock_brightness;
int min_clock_LDR_value;
int max_clock_LDR_value;
double min_user_ambilight_brightness;
double max_user_ambilight_brightness;
int min_ambilight_LDR_value;
int max_ambilight_LDR_value;
byte LDR_corr_type;
byte en_it_is;
byte en_am_pm;
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
byte ambilight_activation_type;
int ambilight_LDR_threshold;

byte op_mode = 0;
const byte PAGE_MAX = 12;
int pos[PAGE_MAX];
int page = 0;
bool page_entry[PAGE_MAX + 1];

// EEPROM address assignment
const int EEPROM_addr_min_user_clock_brightness = 0;
const int EEPROM_addr_clock_LDR_min = 1; // needs two bytes
const int EEPROM_addr_max_user_clock_brightness = 3;
const int EEPROM_addr_clock_LDR_max = 4; // needs two bytes
const int EEPROM_addr_min_user_ambilight_brightness = 6;
const int EEPROM_addr_ambilight_LDR_min = 7; // needs two bytes
const int EEPROM_addr_max_user_ambilight_brightness = 9;
const int EEPROM_addr_ambilight_LDR_max = 10; // needs two bytes
const int EEPROM_addr_it_is = 12;
const int EEPROM_addr_oclock = 13;
const int EEPROM_addr_am_pm = 14;
const int EEPROM_addr_single_min = 15;
const int EEPROM_addr_ambilight = 16;
const int EEPROM_addr_language = 17;
const int EEPROM_addr_t_night_1 = 18;
const int EEPROM_addr_t_night_2 = 19;
const int EEPROM_addr_h_clock = 20;
const int EEPROM_addr_s_clock = 21;
const int EEPROM_addr_v_clock = 22;
const int EEPROM_addr_h_ambilight = 23;
const int EEPROM_addr_s_ambilight = 24;
const int EEPROM_addr_v_ambilight = 25;
const int EEPROM_addr_LDR_corr_type = 26;
const int EEPROM_addr_ambilight_activation_type = 27;
const int EEPROM_addr_ambilight_LDR_threshold = 28; // needs two bytes

int prevMinute = 100;

DateTime now;


////////////////////////////////////
// Setup
////////////////////////////////////
void setup() {

  Serial.begin(9600);
  delay(200);
  Wire.begin(sdaPin, sclPin);
  delay(200);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Uncomment this when flashing ESP for the first time
  //  if (rtc.lostPower()) {
  //    Serial.println("RTC lost power, lets set the time!");
  //    // following line sets the RTC to the date & time this sketch was compiled
  //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //    // This line sets the RTC with an explicit date & time, for example to set
  //    // January 21, 2014 at 3am you would call:
  //    // rtc.adjust(DateTime(2018, 2, 13, 15, 0, 0));
  //  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);       //setup the bounce instance for the current button
    buttons[i].interval(50);              // interval in ms
  }

  pixels.begin(); // This initializes the NeoPixel library.
  EEPROM.begin(512); // There are 512 bytes of EEPROM, from 0 to 511

  //    // Execute this code only once to initialize default values
  //    EEPROM.write(EEPROM_addr_min_user_clock_brightness, 45); // Min brightness set by user
  //    EEPROMWriteInt(EEPROM_addr_clock_LDR_min, 124); // Correspondig LDR value
  //    EEPROM.write(EEPROM_addr_max_user_clock_brightness, 100); // Max brightness set by user
  //    EEPROMWriteInt(EEPROM_addr_clock_LDR_max, 912); // Correspondig LDR value
  //    EEPROM.write(EEPROM_addr_min_user_ambilight_brightness, 45); // Min brightness set by user
  //    EEPROMWriteInt(EEPROM_addr_ambilight_LDR_min, 124); // Correspondig LDR value
  //    EEPROM.write(EEPROM_addr_max_user_ambilight_brightness, 100); // Max brightness set by user
  //    EEPROMWriteInt(EEPROM_addr_ambilight_LDR_max, 912); // Correspondig LDR value
  //    EEPROM.write(EEPROM_addr_it_is, 1); // "Es ist", default: on
  //    EEPROM.write(EEPROM_addr_am_pm, 0); // "am/pm", default: off
  //    EEPROM.write(EEPROM_addr_oclock, 1); // "Uhr", default: on
  //    EEPROM.write(EEPROM_addr_single_min, 1); // Display singles minutes, default: on
  //    EEPROM.write(EEPROM_addr_ambilight, 0); // Ambilight, default: off
  //    EEPROM.write(EEPROM_addr_language, 0); // Language: 0: German, 1: English, default: German
  //    EEPROM.write(EEPROM_addr_t_night_1, 1); // Starting hour of nighttime, default: 1 am
  //    EEPROM.write(EEPROM_addr_t_night_2, 7); // Ending hour of nighttime, default: 7 am
  //    EEPROM.write(EEPROM_addr_h_clock, 8); // Hue LED clock, default: 0
  //    EEPROM.write(EEPROM_addr_s_clock, 100); // Saturation LED clock, default: 100
  //    EEPROM.write(EEPROM_addr_v_clock, 75); // Value LED clock, default: 0
  //    EEPROM.write(EEPROM_addr_h_ambilight, 8); // Hue LED ambilight, default: 0
  //    EEPROM.write(EEPROM_addr_s_ambilight, 100); // Saturaion LED ambilight, default: 100
  //    EEPROM.write(EEPROM_addr_v_ambilight, 75); // Value LED ambilight, default: 0
  //    EEPROM.write(EEPROM_addr_LDR_corr_type, 0); // LDR correction type, default: 0
  //    EEPROM.write(EEPROM_addr_ambilight_activation_type, 0); // Ambilight activation type, default: 0 (always active)
  //    EEPROMWriteInt(EEPROM_addr_ambilight_LDR_threshold, 1023); // Correspondig LDR value
  //    EEPROM.commit();

  // Read settings from EEPROM
  min_user_clock_brightness = (double) EEPROM.read(EEPROM_addr_min_user_clock_brightness) / 100;
  max_user_clock_brightness = (double) EEPROM.read(EEPROM_addr_max_user_clock_brightness) / 100;
  min_user_clock_brightness = constrain(min_user_clock_brightness, 0, 1);
  max_user_clock_brightness = constrain(max_user_clock_brightness, 0, 1);
  min_clock_LDR_value = EEPROMReadInt(EEPROM_addr_clock_LDR_min);
  max_clock_LDR_value = EEPROMReadInt(EEPROM_addr_clock_LDR_max);
  min_user_ambilight_brightness = (double) EEPROM.read(EEPROM_addr_min_user_ambilight_brightness) / 100;
  max_user_ambilight_brightness = (double) EEPROM.read(EEPROM_addr_max_user_ambilight_brightness) / 100;
  min_user_ambilight_brightness = constrain(min_user_ambilight_brightness, 0, 1);
  max_user_ambilight_brightness = constrain(max_user_ambilight_brightness, 0, 1);
  min_ambilight_LDR_value = EEPROMReadInt(EEPROM_addr_ambilight_LDR_min);
  max_ambilight_LDR_value = EEPROMReadInt(EEPROM_addr_ambilight_LDR_max);
  en_it_is = EEPROM.read(EEPROM_addr_it_is);
  en_am_pm = EEPROM.read(EEPROM_addr_am_pm);
  en_oclock = EEPROM.read(EEPROM_addr_oclock);
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
  ambilight_activation_type = EEPROM.read(EEPROM_addr_ambilight_activation_type);
  ambilight_LDR_threshold = EEPROMReadInt(EEPROM_addr_ambilight_LDR_threshold);

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

}

////////////////////////////////////
// Main loop
////////////////////////////////////
void loop() {

  // Get RTC time
  now = rtc.now();
  //days = now.day();
  //months = now.month();
  //years = now.year();
  //minutes = now.minute();
  //hours = now.hour();

  // Read buttons
  for (int i = 0; i < NUM_BUTTONS; i++)  {
    buttons[i].update();
  }

  // Go to current page
  switch (op_mode) {
    case 0: // normal mode
      normal_op();
      break;

    case 1: // settings menu
      switch (page) {
        case 0: // Page 0: Top level, select an entry
          page0();
          break;
        case 1: // Page 1: Set time
          page1();
          break;
        case 2: // Page 2: Set date
          page2();
          break;
        case 3: // Page 3: Set color
          page3();
          break;
        case 4: // Page 4: Brightness calibration bright room
          page4();
          break;
        case 5: // Page 5: Brightness calibration dark room
          page5();
          break;
        case 6: // Page 6: LDR correction type
          page6();
          break;
        case 7: // Page 7: Night time
          page7();
          break;
        case 8: // Page 8: Language
          page8();
          break;
        case 9: // Page 9: Display 'o'clock'
          page9();
          break;
        case 10: // Page 10: Display 'it is' 
          page10();
          break;
        case 11: // Page 11: Display am/pm (English only)
          page11();
          break;
        case 12: // Page 12: Enable/Disable single minute LEDs
          page12();
          break;
        case 13: // Page 13: LED test
          page12();
          break;
      }
      break;
  }
}

////////////////////////////////////
// Normal operation
////////////////////////////////////
void normal_op() {
  if (buttons[0].fell()) {
    if(nighttime())
      en_nighttime = false; // Temporary, will be activated next morning
    if(!en_clock)  
      en_clock = 1;
    op_mode = 1;
    page = 0;
    Serial.println("Enter menu");
    pos[page] = 0;
    dispPage();
  }

  // Display current day of the month
  if (buttons[1].fell()) {
    if(nighttime())
      en_nighttime = false; // Temporary, will be activated next morning
    if(!en_clock)  
      en_clock = 1;
    showDay();
    settings_changed = true;
  }

  // Display temperature
  if (buttons[2].fell()) {
    temp_rtc = rtc.getTemperature();
    if(nighttime())
      en_nighttime = false; // Temporary, will be activated next morning    
    if(!en_clock)  
      en_clock = 1;
    Serial.println(temp_rtc, 2);
    int int_temp_rtc = (int) temp_rtc;
    dispNum(int_temp_rtc);
    delay(3000);
    settings_changed = true;
  }
  
  // Disable/Enable LEDs 
  if (buttons[3].fell()) {

    if(en_clock) {
      en_clock = 0;
      en_nighttime = true;
    }
    else {
      if(nighttime())
        en_nighttime = false; // Temporary, will be activated next morning    
      en_clock = 1;
    }
    settings_changed = 1;
  }

  if ((now.minute() != prevMinute || settings_changed) ) {

    // Check whether nighttime is active
    if (!nighttime() && en_clock) {
      getClockBrightness();
      clockDisplay(); // Real clock
      serialClockDisplay(); // Serial port
    }
    else
      disableAllLED();

    prevMinute = now.minute();
    settings_changed = 0;

  }
}

////////////////////////////////////
// Settings menu: Top level
////////////////////////////////////
void page0() {

  if (page_entry[page]) {
    dispPage();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    pos[page] = 0;
    op_mode = 0;
    Serial.println("Normal mode");
    settings_changed = true;
    dispBack();
    delay(700);
  }

  if (buttons[1].fell()) {
    pos[page]--;
    if (pos[page] < 0) pos[page] = PAGE_MAX - 1;
    if (pos[page] > PAGE_MAX - 1) pos[page] = 0;
    Serial.print("Page ");
    Serial.println(pos[page] + 1);
    dispPage();
  }
  if (buttons[2].fell()) {
    pos[page]++;
    if (pos[page] < 0) pos[page] = PAGE_MAX - 1;
    if (pos[page] > PAGE_MAX - 1) pos[page] = 0;
    Serial.print("Page ");
    Serial.println(pos[page] + 1);
    dispPage();
  }

  if (buttons[3].fell()) {
    page = pos[page] + 1;
    pos[page] = 0;
    page_entry[page] = true;
    Serial.print("Enter page ");
    Serial.println(page);
    dispConfirm();
    delay(700);
  }
}

////////////////////////////////////
// Page 1: Set time
////////////////////////////////////
void page1() {

  if (page_entry[page]) {
    minutes = now.minute();
    hours = now.hour();
    dispNum(hours);
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    page = 0;
    page_entry[page] = true;
    Serial.println("Return to menu");
    dispBack();
    delay(700);
  }

  switch (pos[page]) {

    case 0: // hour
      if (buttons[1].fell()) {
        hours--;
        if (hours < 0) hours = 23;
        if (hours > 23) hours = 0;
        Serial.print("Hour: ");
        Serial.println(hours);
        dispNum(hours);
      }
      if (buttons[2].fell()) {
        hours++;
        if (hours < 0) hours = 23;
        if (hours > 23) hours = 0;
        Serial.print("Hour: ");
        Serial.println(hours);
        dispNum(hours);
      }

      if (buttons[3].fell()) {
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), hours, now.minute(), now.second()));
        pos[page] = 1; // go to minutes
        Serial.print("Set hour to ");
        Serial.println(hours);
        Serial.println("Proceed to minutes ");
        dispConfirm();
        delay(700);
        dispNum(minutes);
      }
      break;

    case 1: // minute
      if (buttons[1].fell()) {
        minutes--;
        if (minutes < 0) minutes = 59;
        if (minutes > 59) minutes = 0;
        Serial.print("Minute: ");
        Serial.println(minutes);
        dispNum(minutes);
      }
      if (buttons[2].fell()) {
        minutes++;
        if (minutes < 0) minutes = 59;
        if (minutes > 59) minutes = 0;
        Serial.print("Minute: ");
        Serial.println(minutes);
        dispNum(minutes);
      }

      if (buttons[3].fell()) {
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), hours, minutes, 0));
        Serial.print("Set minute to ");
        Serial.println(minutes);
        Serial.println("Return to menu");
        pos[page] = 0;
        page = 0;
        page_entry[page] = true;
        dispConfirm();
        delay(700);
      }
      break;
  }
}

////////////////////////////////////
// Page 2: Set date
////////////////////////////////////
void page2() {

  if (page_entry[page]) {
    days = now.day();
    months = now.month();
    years = now.year();
    years = years-2000;
    dispNum(years);
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }

  switch (pos[page]) {

    case 0: // year
      if (buttons[1].fell()) {
        years--;
        if (years < 0) years = 99;
        if (years > 99) years = 0;
        Serial.print("Year: ");
        Serial.println(years);
        dispNum(years);
      }
      if (buttons[2].fell()) {
        years++;
        if (years < 0) years = 99;
        if (years > 99) years = 0;
        Serial.print("Year: ");
        Serial.println(years);
        dispNum(hours);
      }

      if (buttons[3].fell()) {
        rtc.adjust(DateTime(2000 + years, now.month(), now.day(), now.hour(), now.minute(), now.second()));
        pos[page] = 1; // month
        Serial.print("Set year to ");
        Serial.println(years);
        Serial.println("Proceed to day");
        dispConfirm();
        delay(700);
        dispNum(months);
      }
      break;

    case 1: // month
      if (buttons[1].fell()) {
        months--;
        if (months < 1) months = 12;
        if (months > 12) months = 1;
        Serial.print("Month: ");
        Serial.println(months);
        dispNum(months);
      }
      if (buttons[2].fell()) {
        months++;
        if (months < 1) months = 12;
        if (months > 12) months = 1;
        Serial.print("Month: ");
        Serial.println(months);
        dispNum(months);
      }

      if (buttons[3].fell()) {
        rtc.adjust(DateTime(years, months, now.day(), now.hour(), now.minute(), now.second()));
        pos[page] = 2; // go to day
        Serial.print("Set month to ");
        Serial.println(months);
        Serial.println("Proceed to day ");
        dispConfirm();
        delay(700);
        dispNum(days);
      }
      break;

    case 2: // day, checking Feb. and months with only 30 days
      if (buttons[1].fell()) {
        days--;
        if (days < 1) days = 31;
        if (days > 31) days = 1;
        Serial.print("Day: ");
        Serial.println(days);
        dispNum(days);
      }
      if (buttons[2].fell()) {
        days++;
        if (days < 1) days = 31;
        if (days > 31) days = 1;
        Serial.print("Day: ");
        Serial.println(days);
        dispNum(days);
      }

      if (buttons[3].fell()) {
        rtc.adjust(DateTime(years, months, days, now.hour(), now.minute(), now.second()));
        Serial.print("Set day to ");
        Serial.println(days);
        Serial.println("Return to menu");
        dispConfirm();
        delay(700);
        pos[page] = 0;
        page = 0;
        page_entry[page] = true;
      }
      break;
  }
}

////////////////////////////////////
// Page 3: Set color
////////////////////////////////////
void page3() {

  if (page_entry[page]) {
    s_clock = 1.0;
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    disableAllLED();
    sendTime2LED(es_ist);
    sendTime2LED(zwanzig_min);
    sendTime2LED(vor);
    sendTime2LED(hours_GER[12]);
    pixels.show();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    h_clock = (double) EEPROM.read(EEPROM_addr_h_clock) / 100; // Must be in range [0..1]
    s_clock = (double) EEPROM.read(EEPROM_addr_s_clock) / 100; // Must be in range [0..1]
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }

  switch (pos[page]) {

    case 0: // hue
      if (buttons[1].read() == LOW && buttons[2].read() != LOW) {
        h_clock -= 0.01;
        if (h_clock < 0) h_clock = 0;
        if (h_clock > 1) h_clock = 1;
        rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
        Serial.print("Hue: ");
        Serial.println(h_clock);
        sendTime2LED(es_ist);
        sendTime2LED(zwanzig_min);
        sendTime2LED(vor);
        sendTime2LED(hours_GER[12]);
        pixels.show();
        delay(100);
      }
      if (buttons[2].read() == LOW && buttons[1].read() != LOW) {
        h_clock += 0.01;
        if (h_clock < 0) h_clock = 0;
        if (h_clock > 1) h_clock = 1;
        rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
        Serial.print("Hue: ");
        Serial.println(h_clock);
        sendTime2LED(es_ist);
        sendTime2LED(zwanzig_min);
        sendTime2LED(vor);
        sendTime2LED(hours_GER[12]);
        pixels.show();
        delay(100);
      }

      if (buttons[3].fell()) {
        Serial.println("Proceed to saturation");
        pos[page] = 1; // saturation
        dispConfirm();
        delay(700);
        disableAllLED();
        sendTime2LED(es_ist);
        sendTime2LED(zwanzig_min);
        sendTime2LED(vor);
        sendTime2LED(hours_GER[12]);
        pixels.show();
      }
      break;

    case 1: // saturation
      if (buttons[1].read() == LOW && buttons[2].read() != LOW) {
        s_clock -= 0.01;
        if (s_clock < 0) s_clock = 0;
        if (s_clock > 1) s_clock = 1;
        rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
        Serial.print("Saturation: ");
        Serial.println(s_clock);
        sendTime2LED(es_ist);
        sendTime2LED(zwanzig_min);
        sendTime2LED(vor);
        sendTime2LED(hours_GER[12]);
        pixels.show();
        delay(100);
      }
      if (buttons[2].read() == LOW && buttons[1].read() != LOW) {
        s_clock += 0.01;
        if (s_clock < 0) s_clock = 0;
        if (s_clock > 1) s_clock = 1;
        rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
        Serial.print("Saturation: ");
        Serial.println(s_clock);
        sendTime2LED(es_ist);
        sendTime2LED(zwanzig_min);
        sendTime2LED(vor);
        sendTime2LED(hours_GER[12]);
        pixels.show();
        delay(100);
      }

      if (buttons[3].fell()) {
        byte temp = (int) (s_clock * 100);
        EEPROM.write(EEPROM_addr_s_clock, temp);
        temp = (int) (h_clock * 100);
        EEPROM.write(EEPROM_addr_h_clock, temp);
        EEPROM.commit();
        Serial.print("Set hue to ");
        Serial.println(h_clock);
        Serial.print("Set saturation to ");
        Serial.println(s_clock);
        Serial.println("Return to menu");
        dispConfirm();
        delay(700);
        pos[page] = 0;
        page = 0;
        page_entry[page] = true;
      }
      break;
  }
}

////////////////////////////////////
// Page 4: Brightness calibration light room
////////////////////////////////////
void page4() {

  if (page_entry[page]) {
    disableAllLED();
    sendTime2LED(es_ist);
    sendTime2LED(zwanzig_min);
    sendTime2LED(vor);
    sendTime2LED(hours_GER[12]);
    pixels.show();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }

  if (buttons[1].read() == LOW && buttons[2].read() != LOW) {
    v_clock -= 0.01;
    if (v_clock < 0) v_clock = 0;
    if (v_clock > 1) v_clock = 1;
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    Serial.print("Brightness: ");
    Serial.println(v_clock);
    delay(100);
    sendTime2LED(es_ist);
    sendTime2LED(zwanzig_min);
    sendTime2LED(vor);
    sendTime2LED(hours_GER[12]);
    pixels.show();
    delay(100);
  }
  if (buttons[2].read() == LOW && buttons[1].read() != LOW) {
    v_clock += 0.01;
    if (v_clock < 0) v_clock = 0;
    if (v_clock > 1) v_clock = 1;
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    Serial.print("Brightness: ");
    Serial.println(v_clock);
    delay(100);
    sendTime2LED(es_ist);
    sendTime2LED(zwanzig_min);
    sendTime2LED(vor);
    sendTime2LED(hours_GER[12]);
    pixels.show();
    delay(100);
  }

  if (buttons[3].fell()) {
    byte temp = (int) (v_clock * 100);
    setMaxClockBrightness();
    Serial.print("Set brightness for light room to ");
    Serial.println(v_clock);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 5: Brightness calibration dark room
////////////////////////////////////
void page5() {

  if (page_entry[page]) {
    disableAllLED();
    sendTime2LED(es_ist);
    sendTime2LED(zwanzig_min);
    sendTime2LED(vor);
    sendTime2LED(hours_GER[12]);
    pixels.show();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }
  if (buttons[1].read() == LOW && buttons[2].read() != LOW) {
    v_clock -= 0.01;
    if (v_clock < 0) v_clock = 0;
    if (v_clock > 1) v_clock = 1;
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    Serial.print("Brightness: ");
    Serial.println(v_clock);
    delay(100);
    sendTime2LED(es_ist);
    sendTime2LED(zwanzig_min);
    sendTime2LED(vor);
    sendTime2LED(hours_GER[12]);
    pixels.show();
  }
  if (buttons[2].read() == LOW && buttons[1].read() != LOW) {
    v_clock += 0.01;
    if (v_clock < 0) v_clock = 0;
    if (v_clock > 1) v_clock = 1;
    rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
    Serial.print("Brightness: ");
    Serial.println(v_clock);
    delay(100);
    sendTime2LED(es_ist);
    sendTime2LED(zwanzig_min);
    sendTime2LED(vor);
    sendTime2LED(hours_GER[12]);
    pixels.show();
  }

  if (buttons[3].fell()) {
    byte temp = (int) (v_clock * 100);
    setMinClockBrightness();
    Serial.print("Set brightness for dark room to ");
    Serial.println(v_clock);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 6: LDR correction type
////////////////////////////////////
void page6() {

  if (page_entry[page]) {
    dispNum(LDR_corr_type + 1);
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    LDR_corr_type = EEPROM.read(EEPROM_addr_LDR_corr_type);
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }

  if (buttons[1].fell()) {
    LDR_corr_type--;
    if(LDR_corr_type < 0)
      LDR_corr_type = 0;
    dispNum(LDR_corr_type + 1);
  }
  
  if (buttons[2].fell()) {
    LDR_corr_type++;
    if(LDR_corr_type > 2)
      LDR_corr_type = 2;
    dispNum(LDR_corr_type + 1);
  }

  if (buttons[3].fell()) {
    EEPROM.write(EEPROM_addr_LDR_corr_type, LDR_corr_type);
    EEPROM.commit();
    Serial.print("Set LDR correction type to ");
    Serial.println(LDR_corr_type);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 7: Night time
////////////////////////////////////
void page7() {

  if (page_entry[page]) {
    dispNum(t_night_1);
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    t_night_1 = EEPROM.read(EEPROM_addr_t_night_1);
    t_night_2 = EEPROM.read(EEPROM_addr_t_night_2);
    page = 0;
    page_entry[page] = true;
    Serial.println("Return to menu");
    dispBack();
    delay(700);
  }

  switch (pos[page]) {

    case 0: // start of night-time
      if (buttons[1].fell()) {
        t_night_1--;
        if (t_night_1 < 0) t_night_1 = 23;
        if (t_night_1 > 23) t_night_1 = 0;
        Serial.print("Starting night-time at: ");
        Serial.println(t_night_1);
        dispNum(t_night_1);
      }
      if (buttons[2].fell()) {
        t_night_1++;
        if (t_night_1 < 0) t_night_1 = 23;
        if (t_night_1 > 23) t_night_1 = 0;
        Serial.print("Starting night-time at: ");
        Serial.println(t_night_1);
        dispNum(t_night_1);
      }

      if (buttons[3].fell()) {
        EEPROM.write(EEPROM_addr_t_night_1, t_night_1);
        EEPROM.commit();
        pos[page] = 1; // go to minutes
        Serial.print("Set start of night-time to ");
        Serial.println(t_night_1);
        Serial.println("Proceed to end of night-time ");
        dispConfirm();
        delay(700);
        dispNum(t_night_2);
      }
      break;

    case 1: // end of night-time
      if (buttons[1].fell()) {
        t_night_2--;
        if (t_night_2 < 0) t_night_2 = 23;
        if (t_night_2 > 23) t_night_2 = 0;
        Serial.print("Ending night-time at: ");
        Serial.println(t_night_2);
        dispNum(t_night_2);
      }
      if (buttons[2].fell()) {
        t_night_2++;
        if (t_night_2 < 0) t_night_2 = 23;
        if (t_night_2 > 23) t_night_2 = 0;
        Serial.print("Ending night-time at: ");
        Serial.println(t_night_2);
        dispNum(t_night_2);
      }

      if (buttons[3].fell()) {
        EEPROM.write(EEPROM_addr_t_night_2, t_night_2);
        EEPROM.commit();
        Serial.print("Set end of night-time to ");
        Serial.println(t_night_2);
        Serial.println("Return to menu");
        dispConfirm();
        delay(700);
        pos[page] = 0;
        page = 0;
        page_entry[page] = true;
      }
      break;
  }
}

////////////////////////////////////
// Page 8: Language
////////////////////////////////////
void page8() {

  if (page_entry[page]) {
    if (language)
      dispEN();
    else
      dispDE();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    language = EEPROM.read(EEPROM_addr_language);
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }
  if (buttons[1].fell() || buttons[2].fell()) {
    if (language) {
      language = 0;
      Serial.println("Language: German");
      dispDE();
    }
    else {
      language = 1;
      Serial.println("Language: English");
      dispEN();
    }
  }

  if (buttons[3].fell()) {
    EEPROM.write(EEPROM_addr_language, language);
    EEPROM.commit();
    Serial.print("Set language to ");
    Serial.println(language);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 9: Display 'o'clock'
////////////////////////////////////
void page9() {

  if (page_entry[page]) {
    if (en_oclock)
      dispON();
    else
      dispOFF();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    en_oclock = EEPROM.read(EEPROM_addr_oclock);
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }
  if (buttons[1].fell() || buttons[2].fell()) {
    if (en_oclock) {
      en_oclock = 0;
      Serial.println("O'clock disabled");
      dispOFF();
    }
    else {
      en_oclock = 1;
      Serial.println("O'clock enabled");
      dispON();
    }
  }

  if (buttons[3].fell()) {
    EEPROM.write(EEPROM_addr_oclock, en_oclock);
    EEPROM.commit();
    Serial.print("Set o'clock to ");
    Serial.println(en_oclock);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}


////////////////////////////////////
// Page 10: Display 'it is'
////////////////////////////////////
void page10() {

  if (page_entry[page]) {
    if (en_it_is)
      dispON();
    else
      dispOFF();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    en_it_is = EEPROM.read(EEPROM_addr_it_is);
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }
  if (buttons[1].fell() || buttons[2].fell()) {
    if (en_it_is) {
      en_it_is = 0;
      Serial.println("'It is' disabled");
      dispOFF();
    }
    else {
      en_it_is = 1;
      Serial.println("'It is' enabled");
      dispON();
    }
  }

  if (buttons[3].fell()) {
    EEPROM.write(EEPROM_addr_it_is, en_it_is);
    EEPROM.commit();
    Serial.print("Set 'it is' to ");
    Serial.println(en_it_is);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 11: Display am/pm (English only)
////////////////////////////////////
void page11() {

  if (page_entry[page]) {
    if (en_am_pm)
      dispON();
    else
      dispOFF();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    en_am_pm = EEPROM.read(EEPROM_addr_am_pm);
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }

  if (buttons[1].fell() || buttons[2].fell()) {
    if (en_am_pm) {
      en_am_pm = 0;
      Serial.println("Display 'am/pm' disabled");
      dispOFF();
    }
    else {
      en_am_pm = 1;
      Serial.println("Display 'am/pm' enabled");
      dispON();
    }
  }

  if (buttons[3].fell()) {
    EEPROM.write(EEPROM_addr_am_pm, en_am_pm);
    EEPROM.commit();
    Serial.print("Set 'am/pm' to ");
    Serial.println(en_am_pm);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 12: Enable/Disable single minute LEDs
////////////////////////////////////
void page12() {

  if (page_entry[page]) {
    if (en_single_min)
      dispON();
    else
      dispOFF();
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    en_single_min = EEPROM.read(EEPROM_addr_single_min);
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }
  if (buttons[1].fell() || buttons[2].fell()) {
    if (en_single_min) {
      en_single_min = 0;
      Serial.println("Display single minute LEDs: off");
      dispOFF();
    }
    else {
      en_single_min = 1;
      Serial.println("Display single minute LEDs: on");
      dispON();
    }
  }

  if (buttons[3].fell()) {
    EEPROM.write(EEPROM_addr_single_min, en_single_min);
    EEPROM.commit();
    Serial.print("Set 'am/pm' to ");
    Serial.println(en_single_min);
    Serial.println("Return to menu");
    dispConfirm();
    delay(700);
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 13: LED test
////////////////////////////////////
void page13() {

  if (page_entry[page]) {
    page_entry[page] = false;
  }

  if (buttons[0].fell()) {
    page = 0;
    page_entry[page] = true;
    Serial.println("Enter menu ");
    dispBack();
    delay(700);
  }
  if (buttons[1].fell() || buttons[2].fell() || buttons[3].fell()) {
    Serial.println("Testing all LEDs");
    LEDTest();
    pos[page] = 0;
    page = 0;
    page_entry[page] = true;
  }
}

////////////////////////////////////
// Page 13: Correction type light measurement
////////////////////////////////////
// TODO

////////////////////////////////////////////////////
// Determine current time and send data to LEDs
////////////////////////////////////////////////////
void clockDisplay() {

  // All clock LED off
  disableClockLED();

  // Get NTP time
  minutes = now.minute();
  hours = now.hour();

  // Single minutes
  single_min = minutes % 5;
  if (single_min > 0 && en_single_min)
    sendTime2LED(single_mins[single_min - 1]);

  // Five minutes
  min_five = minutes - single_min;

  // Hours
  if (hours > 12)
    hours = hours % 12; // hours modulo 12  

  // Output time depending on language
  switch (language) {

    // German
    case 0:
      // Display "es ist"
      if (en_it_is)
        sendTime2LED(es_ist);

      switch (min_five) {
        case 0:
          sendTime2LED(full_hours_GER[hours]);
          if (en_oclock) sendTime2LED(uhr);
          break;
        case 5:
          sendTime2LED(fuenf_min);
          sendTime2LED(nach);
          sendTime2LED(hours_GER[hours]);
          break;
        case 10:
          sendTime2LED(zehn_min);
          sendTime2LED(nach);
          sendTime2LED(hours_GER[hours]);
          break;
        case 15:
          sendTime2LED(viertel_min);
          sendTime2LED(nach);
          sendTime2LED(hours_GER[hours]);
          break;
        case 20:
          sendTime2LED(zwanzig_min);
          sendTime2LED(nach);
          sendTime2LED(hours_GER[hours]);
          break;
        case 25:
          sendTime2LED(fuenf_min);
          sendTime2LED(vor);
          sendTime2LED(halb);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_GER[hours]);
          break;
        case 30:
          sendTime2LED(halb);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_GER[hours]);
          break;
        case 35:
          sendTime2LED(fuenf_min);
          sendTime2LED(nach);
          sendTime2LED(halb);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_GER[hours]);
          break;
        case 40:
          sendTime2LED(zwanzig_min);
          sendTime2LED(vor);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_GER[hours]);
          break;
        case 45:
          sendTime2LED(viertel_min);
          sendTime2LED(vor);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_GER[hours]);
          break;
        case 50:
          sendTime2LED(zehn_min);
          sendTime2LED(vor);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_GER[hours]);
          break;
        case 55:
          sendTime2LED(fuenf_min);
          sendTime2LED(vor);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_GER[hours]);
          break;
      }
      break;

    // English
    case 1:

      // Display "it is"
      if (en_it_is)
        sendTime2LED(it_is);

      // Display am/pm
      if (en_am_pm) {
        if (hours <= 12)
          sendTime2LED(am);
        else
          sendTime2LED(pm);
      }

      switch (min_five) {
        case 0:
          sendTime2LED(hours_EN[hours]);
          if (en_oclock) sendTime2LED(oclock);
          break;
        case 5:
          sendTime2LED(five_min);
          sendTime2LED(past);
          sendTime2LED(hours_EN[hours]);
          break;
        case 10:
          sendTime2LED(ten_min);
          sendTime2LED(past);
          sendTime2LED(hours_EN[hours]);
          break;
        case 15:
          sendTime2LED(quarter_min);
          sendTime2LED(past);
          sendTime2LED(hours_EN[hours]);
          break;
        case 20:
          sendTime2LED(twenty_min);
          sendTime2LED(past);
          sendTime2LED(hours_EN[hours]);
          break;
        case 25:
          sendTime2LED(twenty_min);
          sendTime2LED(five_min);
          sendTime2LED(past);
          sendTime2LED(hours_EN[hours]);
          break;
        case 30:
          sendTime2LED(half);
          sendTime2LED(past);
          sendTime2LED(hours_EN[hours]);
          break;
        case 35:
          sendTime2LED(twenty_min);
          sendTime2LED(five_min);
          sendTime2LED(to);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_EN[hours]);
          break;
        case 40:
          sendTime2LED(twenty_min);
          sendTime2LED(to);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_EN[hours]);
          break;
        case 45:
          sendTime2LED(quarter_min);
          sendTime2LED(to);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_EN[hours]);
          break;
        case 50:
          sendTime2LED(ten_min);
          sendTime2LED(to);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_EN[hours]);
          break;
        case 55:
          sendTime2LED(five_min);
          sendTime2LED(to);
          hours++;
          if (hours > 12) hours = hours % 12;
          sendTime2LED(hours_EN[hours]);
          break;
      }
      break;
  }

  pixels.show();

}

////////////////////////////////////////////////////
// Send data to LEDs
////////////////////////////////////////////////////
void sendTime2LED(byte x[]) {

  for (byte i = 0; i <= 6; i++) {
    if (x[i] != 0)
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
// Send number data to LED
////////////////////////////////////////////////////
void sendNum2LED(byte x[]) {
  for (byte i = 0; i <= 16; i++) {
    if (x[i] != 0)
      pixels.setPixelColor(x[i] - 1, pgm_read_byte(&gamma8[clock_rgb[0]]), pgm_read_byte(&gamma8[clock_rgb[1]]), pgm_read_byte(&gamma8[clock_rgb[2]]));
  }
}


////////////////////////////////////////////////////
// Dispay settings page
////////////////////////////////////////////////////
void dispPage() {

  // dispNum(pos[page] + 1);
  // Alternative:
  disableAllLED();
  for (byte i = 0; i <= pos[page]; i++) {
    pixels.setPixelColor(i, pgm_read_byte(&gamma8[clock_rgb[0]]), pgm_read_byte(&gamma8[clock_rgb[1]]), pgm_read_byte(&gamma8[clock_rgb[2]]));
  }
  pixels.show();

}

////////////////////////////////////////////////////
// Display number
////////////////////////////////////////////////////
void dispNum(byte num) {

  disableAllLED();

  byte num_left = (num - num % 10) / 10;
  byte num_right = num % 10;
  sendNum2LED(numbers_left[num_left]);
  sendNum2LED(numbers_right[num_right]);

  pixels.show();

}

////////////////////////////////////////////////////
// Display DE
////////////////////////////////////////////////////
void dispDE() {

  disableAllLED();
  sendNum2LED(text_D);
  sendNum2LED(text_E_right);
  pixels.show();

}

////////////////////////////////////////////////////
// Display EN
////////////////////////////////////////////////////
void dispEN() {

  disableAllLED();
  sendNum2LED(text_E_left);
  sendNum2LED(text_N);
  pixels.show();

}

////////////////////////////////////////////////////
// Display ON
////////////////////////////////////////////////////
void dispON() {

  disableAllLED();
  sendNum2LED(text_O);
  sendNum2LED(text_N);
  pixels.show();

}

////////////////////////////////////////////////////
// Display OFF
////////////////////////////////////////////////////
void dispOFF() {

  disableAllLED();
  sendNum2LED(text_O_small);
  sendNum2LED(text_ff);
  pixels.show();

}

////////////////////////////////////////////////////
// Display confirm
////////////////////////////////////////////////////
void dispConfirm() {

  disableAllLED();
  sendNum2LED(confirm);
  pixels.show();

}

////////////////////////////////////////////////////
// Display back cross
////////////////////////////////////////////////////
void dispBack() {

  disableAllLED();
  sendNum2LED(back);
  pixels.show();

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
  Serial.print(now.hour());
  printDigits(now.minute());
  printDigits(now.second());
  Serial.print(", ");
  Serial.print(now.day());
  Serial.print(".");
  Serial.print(now.month());
  Serial.print(".");
  Serial.print(now.year());
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
      if (now.hour() >= t_night_1 || now.hour() < t_night_2)
        return true;
      else {
        en_nighttime = 1; // Enable nighttime again during the day
        return false;
      }
    }
    // t_1 and t_2 after midnight
    else if (t_night_1 < t_night_2) {
      if (now.hour() >= t_night_1 && now.hour() < t_night_2)
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

    for (byte j = 111; j <= MAX_NUM_LEDS; j++) { // columns
      switch (n) {
        case 0: // Red
          pixels.setPixelColor(j - 1, pixels.Color(test_brightness, 0, 0));
          break;
        case 1: // Green
          pixels.setPixelColor(j - 1, pixels.Color(0, test_brightness, 0));
          break;
        case 2: // Blue
          pixels.setPixelColor(j - 1, pixels.Color(0, 0, test_brightness));
          break;
        case 3: // White
          pixels.setPixelColor(j - 1, pixels.Color(test_brightness, test_brightness, test_brightness));
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

  byte day_left = (now.day() - now.day() % 10) / 10;
  byte day_right = now.day() % 10;
  sendNum2LED(numbers_left[day_left]);
  sendNum2LED(numbers_right[day_right]);

  pixels.show();
  delay(2000);

}

////////////////////////////////////////////////////
// Determine brightness based on sensor data and limits
////////////////////////////////////////////////////
void getClockBrightness() {

  // Read EEPROM
  min_user_clock_brightness = (double) EEPROM.read(EEPROM_addr_min_user_clock_brightness); // /100
  max_user_clock_brightness = (double) EEPROM.read(EEPROM_addr_max_user_clock_brightness); // /100
  min_user_clock_brightness = constrain(min_user_clock_brightness, 0, 100);
  max_user_clock_brightness = constrain(max_user_clock_brightness, 0, 100);
  min_clock_LDR_value = EEPROMReadInt(EEPROM_addr_clock_LDR_min);
  max_clock_LDR_value = EEPROMReadInt(EEPROM_addr_clock_LDR_max);

  // Read LDR
  int sensor_value = readSensor();
  int sensor_value_clock = constrain(sensor_value, min_clock_LDR_value, max_clock_LDR_value);
  int sensor_value_ambilight = constrain(sensor_value, min_ambilight_LDR_value, max_ambilight_LDR_value);

  // Map sensor value
  v_clock = map(sensor_value_clock, min_clock_LDR_value, max_clock_LDR_value, min_user_clock_brightness, max_user_clock_brightness);
  v_clock = v_clock / 100;

  //v_ambilight = map(sensor_value_ambilight, min_ambilight_LDR_value, max_ambilight_LDR_value, min_user_ambilight_brightness * 100, max_user_ambilight_brightness * 100);
  //v_ambilight = v_ambilight / 100;

  // Convert to RGB
  rgb_conv.hsvToRgb(h_clock, s_clock, v_clock, clock_rgb);
  //rgb_conv.hsvToRgb(h_ambilight, s_ambilight, v_ambilight, ambilight_rgb);

  // Print sensor value to serial monitor
  Serial.print("Sensor value: ");
  Serial.print(sensor_value);
  Serial.print("/1024, Brightness: ");
  Serial.print(", Brightness: ");
  Serial.print(v_clock);
  Serial.println("/1");
  //  Serial.print("Min LDR: ");
  //  Serial.print(min_clock_LDR_value);
  //  Serial.print(", Max LDR: ");
  //  Serial.println(max_clock_LDR_value);
  //  Serial.print("Min User: ");
  //  Serial.print(min_user_clock_brightness);
  //  Serial.print(", Max User: ");
  //  Serial.println(max_user_clock_brightness);

}

////////////////////////////////////////////////////
// Set minimum clock brightness
////////////////////////////////////////////////////
void setMinClockBrightness() {

  // Read the analog in value
  int sensor_value = readSensor();

  byte temp = (int) (v_clock * 100);
  EEPROM.write(EEPROM_addr_min_user_clock_brightness, temp); // Min brightness set by user
  EEPROMWriteInt(EEPROM_addr_clock_LDR_min, sensor_value); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Set maximum clock brightness
////////////////////////////////////////////////////
void setMaxClockBrightness() {

  // Read the analog in value
  int sensor_value = readSensor();

  byte temp = (int) (v_clock * 100);
  EEPROM.write(EEPROM_addr_max_user_clock_brightness, temp); // Max brightness set by user
  EEPROMWriteInt(EEPROM_addr_clock_LDR_max, sensor_value); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Set minimum ambilight brightness
////////////////////////////////////////////////////
void setMinAmbilightBrightness() {

  // Read the analog in value
  int sensor_value = readSensor();

  byte temp = (int) (v_ambilight * 100);
  EEPROM.write(EEPROM_addr_min_user_ambilight_brightness, temp); // Min brightness set by user
  EEPROMWriteInt(EEPROM_addr_ambilight_LDR_min, sensor_value); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Set maximum ambilight brightness
////////////////////////////////////////////////////
void setMaxAmbilightBrightness() {

  // Read the analog in value
  int sensor_value = readSensor();

  byte temp = (int) (v_ambilight * 100);
  EEPROM.write(EEPROM_addr_max_user_ambilight_brightness, temp); // Max brightness set by user
  EEPROMWriteInt(EEPROM_addr_ambilight_LDR_max, sensor_value); // Correspondig LDR value
  EEPROM.commit();

}

////////////////////////////////////////////////////
// Read analog sensor
////////////////////////////////////////////////////
int readSensor() {
  int sensor_value = analogRead(analogInPin); // Value between 0 and 1023
  sensor_value = constrain(sensor_value, 0, 1023);
  sensor_value = LDRCorrection(sensor_value);
  return sensor_value;
}


////////////////////////////////////////////////////
// LDR correction
////////////////////////////////////////////////////
int LDRCorrection(int sensor_value) {

  switch (LDR_corr_type) {

    case 0: // None
      return sensor_value;
      break;

    case 1: // Square root
      return pgm_read_word(&LDR_corr_sqrt[sensor_value]);
      break;

    case 2: // Square
      return pgm_read_word(&LDR_corr_square[sensor_value]);
      break;
      
//  case 2: // Logarithmic
//    return pgm_read_word(&LDR_corr_log[sensor_value]);
//    break;
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
      delay(700);
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

// LDR correction (square)
// Function: y = ;
// x = linspace(0,1023,1024);
// y = 1/1023*x.^2;
// for i=1:64
// tmp = num2str(floor(y((i-1)*16+1)));
// for j=2:16
// tmp = [tmp ', ' num2str(floor(y((i-1)*16+j)))];
// end
// mat{i} = tmp;
// disp(tmp)
// end
const uint16_t PROGMEM LDR_corr_square[] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 6,
6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12,
12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15,
16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19,
20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24,
25, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29,
30, 30, 30, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35,
36, 36, 36, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40, 41, 41, 41,
42, 42, 43, 43, 43, 44, 44, 45, 45, 46, 46, 46, 47, 47, 48, 48,
49, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 53, 54, 54, 55, 55,
56, 56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63,
64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 71, 71,
72, 72, 73, 73, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80,
81, 81, 82, 82, 83, 83, 84, 85, 85, 86, 86, 87, 87, 88, 89, 89,
90, 90, 91, 92, 92, 93, 93, 94, 95, 95, 96, 96, 97, 98, 98, 99,
100, 100, 101, 101, 102, 103, 103, 104, 105, 105, 106, 107, 107, 108, 109, 109,
110, 111, 111, 112, 113, 113, 114, 115, 115, 116, 117, 117, 118, 119, 119, 120,
121, 121, 122, 123, 123, 124, 125, 125, 126, 127, 128, 128, 129, 130, 130, 131,
132, 133, 133, 134, 135, 136, 136, 137, 138, 138, 139, 140, 141, 141, 142, 143,
144, 144, 145, 146, 147, 147, 148, 149, 150, 150, 151, 152, 153, 154, 154, 155,
156, 157, 157, 158, 159, 160, 161, 161, 162, 163, 164, 165, 165, 166, 167, 168,
169, 169, 170, 171, 172, 173, 174, 174, 175, 176, 177, 178, 179, 179, 180, 181,
182, 183, 184, 184, 185, 186, 187, 188, 189, 190, 190, 191, 192, 193, 194, 195,
196, 197, 197, 198, 199, 200, 201, 202, 203, 204, 205, 205, 206, 207, 208, 209,
210, 211, 212, 213, 214, 215, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
225, 226, 227, 228, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271,
272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 285, 286, 287, 288,
289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 300, 301, 302, 303, 304, 305,
306, 307, 308, 309, 310, 312, 313, 314, 315, 316, 317, 318, 319, 320, 322, 323,
324, 325, 326, 327, 328, 329, 331, 332, 333, 334, 335, 336, 337, 339, 340, 341,
342, 343, 344, 346, 347, 348, 349, 350, 351, 353, 354, 355, 356, 357, 358, 360,
361, 362, 363, 364, 366, 367, 368, 369, 370, 372, 373, 374, 375, 376, 378, 379,
380, 381, 383, 384, 385, 386, 387, 389, 390, 391, 392, 394, 395, 396, 397, 399,
400, 401, 402, 404, 405, 406, 407, 409, 410, 411, 413, 414, 415, 416, 418, 419,
420, 421, 423, 424, 425, 427, 428, 429, 430, 432, 433, 434, 436, 437, 438, 440,
441, 442, 444, 445, 446, 448, 449, 450, 452, 453, 454, 456, 457, 458, 460, 461,
462, 464, 465, 466, 468, 469, 470, 472, 473, 474, 476, 477, 478, 480, 481, 483,
484, 485, 487, 488, 489, 491, 492, 494, 495, 496, 498, 499, 501, 502, 503, 505,
506, 508, 509, 510, 512, 513, 515, 516, 518, 519, 520, 522, 523, 525, 526, 528,
529, 530, 532, 533, 535, 536, 538, 539, 541, 542, 544, 545, 546, 548, 549, 551,
552, 554, 555, 557, 558, 560, 561, 563, 564, 566, 567, 569, 570, 572, 573, 575,
576, 578, 579, 581, 582, 584, 585, 587, 588, 590, 591, 593, 594, 596, 597, 599,
600, 602, 603, 605, 606, 608, 610, 611, 613, 614, 616, 617, 619, 620, 622, 624,
625, 627, 628, 630, 631, 633, 635, 636, 638, 639, 641, 642, 644, 646, 647, 649,
650, 652, 654, 655, 657, 658, 660, 662, 663, 665, 666, 668, 670, 671, 673, 675,
676, 678, 679, 681, 683, 684, 686, 688, 689, 691, 693, 694, 696, 697, 699, 701,
702, 704, 706, 707, 709, 711, 712, 714, 716, 717, 719, 721, 722, 724, 726, 728,
729, 731, 733, 734, 736, 738, 739, 741, 743, 744, 746, 748, 750, 751, 753, 755,
756, 758, 760, 762, 763, 765, 767, 769, 770, 772, 774, 776, 777, 779, 781, 783,
784, 786, 788, 790, 791, 793, 795, 797, 798, 800, 802, 804, 805, 807, 809, 811,
813, 814, 816, 818, 820, 821, 823, 825, 827, 829, 830, 832, 834, 836, 838, 840,
841, 843, 845, 847, 849, 850, 852, 854, 856, 858, 860, 861, 863, 865, 867, 869,
871, 872, 874, 876, 878, 880, 882, 884, 885, 887, 889, 891, 893, 895, 897, 899,
900, 902, 904, 906, 908, 910, 912, 914, 915, 917, 919, 921, 923, 925, 927, 929,
931, 933, 934, 936, 938, 940, 942, 944, 946, 948, 950, 952, 954, 956, 958, 960,
961, 963, 965, 967, 969, 971, 973, 975, 977, 979, 981, 983, 985, 987, 989, 991,
993, 995, 997, 999, 1001, 1003, 1005, 1007, 1009, 1011, 1013, 1015, 1017, 1019, 1021, 1023
};
