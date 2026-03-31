
#include <rom/rtc.h> 
#include <math.h>  
#include <ctype.h>


#include <PubSubClient.h>

#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include "DHT.h"
#include <Adafruit_BMP280.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>

#ifndef _WIFI_H 
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620169500";                    // Add your ID number here
static const char* subtopic[]    = {"620169500_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "www.yanacreations.com";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;

// WIFI CREDENTIALS
const char* ssid       = "MonaConnect";     // Add your Wi-Fi ssid
const char* password   = ""; // Add your Wi-Fi password 



// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  


extern const char* subtopic[2];
extern const char* mqtt_server;
extern uint16_t mqtt_port;
extern const char* ssid;
extern const char* password;

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );  
bool isNumber(double number);

#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// ── Pins ─────────────────────────────────────────────────────
#define DHTPIN       2
#define DHTTYPE      DHT22
#define SOIL_PIN     32
#define TFT_CS       5
#define TFT_DC      17
#define TFT_RST     16
#define TOUCH_CS    27
#define BMP_SCL     22
#define BMP_SDA     21

// ── Touch calibration ────────────────────────────────────────
#define TS_MINX  200
#define TS_MINY  300
#define TS_MAXX 3800
#define TS_MAXY 3700

// ── Sensor thresholds ────────────────────────────────────────
#define TEMP_WARN     35.0f
#define TEMP_EXTREME  38.0f
#define HUMI_WARN     70.0f
#define HUMI_EXTREME  80.0f
#define PRES_WARN   1020.0f
#define PRES_EXTREME 1022.0f
#define SOIL_WARN    3200    // ADC above this = dry warning
#define SOIL_EXTREME 3800    // ADC above this = extreme dry
#define SOIL_INVALID_LOW 500  // ADC below this = invalid/no reading

// ── Soil calibration ─────────────────────────────────────────
#define SOIL_WET_ADC  2300   // fully wet (100% health)
#define SOIL_DRY_ADC  4095   // fully dry (0% health)
#define SOIL_CAL_ADC_50 2500 // ~50% moisture
#define SOIL_CAL_ADC_30 2800 // ~30% moisture
#define SOIL_CAL_ADC_14 3200 // ~14% moisture

// ── Timing ───────────────────────────────────────────────────
#define FRAME_MS        80
#define SENSOR_MS     2000
#define TOUCH_DEBOUNCE  300
#define TOUCH_MIN_Z    1200
#define SIMPLE_UI_MODE true
#define LOVE_DECAY_MIN 10000UL
#define LOVE_DECAY_MAX 20000UL

// ── Love bar ─────────────────────────────────────────────────
#define LOVE_PER_TAP    10
#define LOVE_MAX       100

// ── Plant tap zone radius (px) ───────────────────────────────
#define PLANT_TAP_R     80

// ── Layout ───────────────────────────────────────────────────
#define HEADER_H     18
#define LOVE_BAR_Y   18
#define LOVE_H       14
#define ANIM_TOP     32
#define GROUND_Y    155
#define ANIM_BOT    165
#define QUAD_TOP    175
#define QUAD_MID    240
#define QUAD_BOT    305
#define PLANT_CX    120
#define PLANT_GY    152
#define TILE_W      120
#define TILE_H       65

// ── Colours ──────────────────────────────────────────────────
#define C_BG        0x0001
#define C_HEADER    0x080F
#define C_LOVE_BG   0x0A05
#define C_QUAD_BG   0x2008   // dark magenta — readable against all text colours
#define C_DIVIDER   0x18C3
#define C_WHITE     ILI9341_WHITE
#define C_BLACK     ILI9341_BLACK
#define C_GREEN     0x07E0
#define C_RED       ILI9341_RED
#define C_YELLOW    ILI9341_YELLOW
#define C_ORANGE    0xFD20
#define C_CYAN      0x07FF
#define C_LTGREY    0xC618
#define C_DKGREY    0x4208
#define C_LOVEPINK  0xFA14
#define C_HOTPINK   0xF8B2
#define C_CHEEK     0xF8B2
#define C_SWEAT     0x051F

// Sky colours per mode
#define C_SKY_HAPPY  0x1B6D   // bright blue
#define C_SKY_THRSTY 0x5A28   // dusty amber-brown
#define C_SKY_HOT    0x2808   // burnt dark orange
#define C_SKY_OVER   0x0C10   // grey-blue overcast
#define C_SKY_CRIT   0x0800   // near-black red-tint

// Ground colours per mode
#define C_GND_GREEN  0x1700
#define C_GND_DRY    0x4A20
#define C_GND_CRIT   0x0800

// Plant colours
#define C_STEM       0x2B60
#define C_LEAF       0x17C0
#define C_LEAF_DRY   0x8B40
#define C_LEAF_HOT   0xFB00
#define C_LEAF_DEAD  0x2940
#define C_POT        0xA185
#define C_POT_RIM    0x8124
#define C_POT_DARK   0x5103

// Background decoration
#define C_SUN        0xFFE0
#define C_SUN_RAY    0xFD20
#define C_CLOUD      0xDEFB
#define C_GRAVE      0x2104
#define C_FLAME1     0xF800
#define C_FLAME2     0xFC00
#define C_FLAME3     0xFFE0

// Quadrant accents
#define C_Q0  0xF800
#define C_Q1  0x07FF
#define C_Q2  0x07E0
#define C_Q3  0xC81F

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );  
bool isNumber(double number);
float convert_Celsius_to_fahrenheit(float celsius);      // Convert Celsius to Fahrenheit
float convert_fahrenheit_to_Celsius(float fahrenheit);   // Convert Fahrenheit to Celsius
float calcHeatIndex(float tempF, float humidity);        // Calculate heat index in Fahrenheit

// ── Objects ──────────────────────────────────────────────────
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS);
bool bmpAvailable = false;

// ── State ────────────────────────────────────────────────────
enum Screen     { SCR_HOME, SCR_GAME };
enum PlantState { PS_HAPPY, PS_THIRSTY, PS_HOT, PS_DEAD };
Screen     currentScreen  = SCR_HOME;
PlantState plantState     = PS_HAPPY;
PlantState lastPlantState = (PlantState)99;

float sTemp = NAN, sHumi = NAN, sPres = NAN;
int   sSoil = 2800;   // default mid-range (between wet 2370 and dry 4095)
float love  = 50.0f;

float lastTempDrawn  = -999, lastHumiDrawn  = -999;
float lastPresDrawn  = -999, lastHealthDrawn = -999;
float lastSoilDrawn  = -1;

int  animTick      = 0;
int  prevSway      = 0;
int  wiggle        = 0;
int  loveFlashTick = 0;

// Heart animation — supports 3 simultaneous hearts
#define MAX_HEARTS 3
struct Heart {
  bool  active;
  int   x, y;       // start position
  int   age;        // 0..HEART_LIFE
};
#define HEART_LIFE 28
Heart hearts[MAX_HEARTS];

bool bgNeedsRedraw = true;

unsigned long lastFrameMs   = 0;
unsigned long lastSensorMs  = 0;
unsigned long lastTouchMs   = 0;
unsigned long lastLoveDecay = 0;
unsigned long nextLoveDecay = 15000UL;


// ════════════════════════════════════════════════════════════
//  HELPERS
// ════════════════════════════════════════════════════════════
int mapTouch(int raw, int rMin, int rMax, int sMin, int sMax) {
  return constrain(map(raw, rMin, rMax, sMin, sMax), sMin, sMax);
}

bool floatChanged(float a, float b, float eps) {
  if (isnan(a) && isnan(b)) return false;
  if (isnan(a) || isnan(b)) return true;
  return fabsf(a - b) > eps;
}

int readSoilRaw() {
  long acc = 0;
  const int samples = 8;
  for (int i = 0; i < samples; i++) {
    acc += analogRead(SOIL_PIN);
    delay(2);
  }
  return (int)(acc / samples);
}

void scheduleNextLoveDecay() {
  nextLoveDecay = (unsigned long)random(LOVE_DECAY_MIN, LOVE_DECAY_MAX + 1);
}

void spawnHeart(int x, int y) {
  for (int i = 0; i < MAX_HEARTS; i++) {
    if (!hearts[i].active) {
      hearts[i] = {true, x, y, 0};
      return;
    }
  }
  // All slots full — overwrite oldest
  hearts[0] = {true, x, y, 0};
}

// ── Health ───────────────────────────────────────────────────
float healthTemp() {
  if (isnan(sTemp)) return 0.0f;
  if (sTemp <= 30.0f) return 1.0f;
  return constrain(1.0f - (sTemp - 30.0f) / 8.0f, 0.0f, 1.0f);
}
float healthHumi() {
  if (isnan(sHumi)) return 0.0f;
  if (sHumi >= 40.0f && sHumi <= 70.0f) return 1.0f;
  if (sHumi < 40.0f) return constrain(sHumi / 40.0f, 0.0f, 1.0f);
  return constrain(1.0f - (sHumi - 70.0f) / 30.0f, 0.0f, 1.0f);
}

float interpolatePct(int adc, int x0, float y0, int x1, float y1) {
  if (x1 == x0) return y0;
  float t = ((float)(adc - x0)) / ((float)(x1 - x0));
  return y0 + t * (y1 - y0);
}

float soilPercentFromRaw(int raw) {
  if (raw < SOIL_INVALID_LOW) return NAN;

  float pct;
  if (raw <= SOIL_WET_ADC) {
    pct = 100.0f;
  } else if (raw <= SOIL_CAL_ADC_50) {
    pct = interpolatePct(raw, SOIL_WET_ADC, 100.0f, SOIL_CAL_ADC_50, 50.0f);
  } else if (raw <= SOIL_CAL_ADC_30) {
    pct = interpolatePct(raw, SOIL_CAL_ADC_50, 50.0f, SOIL_CAL_ADC_30, 30.0f);
  } else if (raw <= SOIL_CAL_ADC_14) {
    pct = interpolatePct(raw, SOIL_CAL_ADC_30, 30.0f, SOIL_CAL_ADC_14, 14.0f);
  } else if (raw <= SOIL_DRY_ADC) {
    pct = interpolatePct(raw, SOIL_CAL_ADC_14, 14.0f, SOIL_DRY_ADC, 0.0f);
  } else {
    pct = 0.0f;
  }

  return constrain(pct, 0.0f, 100.0f);
}
float healthSoil() {
  float pct = soilPercentFromRaw(sSoil);
  if (isnan(pct)) return 0.0f;
  return pct / 100.0f;
}
float healthPres() {
  if (isnan(sPres)) return 0.0f;
  if (sPres >= 1005.0f && sPres <= 1015.0f) return 1.0f;
  if (sPres < 1005.0f) return constrain(1.0f - (1005.0f - sPres) / 20.0f, 0.0f, 1.0f);
  return constrain(1.0f - (sPres - 1015.0f) / 15.0f, 0.0f, 1.0f);
}
float healthOverall() {
  return healthTemp()*0.30f + healthSoil()*0.35f +
         healthHumi()*0.20f + healthPres()*0.15f;
}

uint16_t barColour(float h) {
  if (h > 0.6f) return C_GREEN;
  if (h > 0.3f) return C_YELLOW;
  return C_RED;
}
uint16_t loveColour() {
  if (love > 60) return C_LOVEPINK;
  if (love > 30) return C_HOTPINK;
  return 0xA812;
}

PlantState getPlantState() {
  bool extreme = sTemp>=TEMP_EXTREME && sHumi>=HUMI_EXTREME &&
                 sPres>=PRES_EXTREME && sSoil>=SOIL_EXTREME;
  if (extreme)            return PS_DEAD;
  if (sTemp >= TEMP_WARN) return PS_HOT;
  if (sSoil >= SOIL_WARN) return PS_THIRSTY;
  return PS_HAPPY;
}

uint16_t skyColour() {
  bool crit = sTemp>=TEMP_EXTREME && sHumi>=HUMI_EXTREME &&
              sPres>=PRES_EXTREME && sSoil>=SOIL_EXTREME;
  if (crit)                                   return C_SKY_CRIT;
  if (sTemp > TEMP_WARN)                      return C_SKY_HOT;
  if (sSoil >= SOIL_WARN)                     return C_SKY_THRSTY;
  if (sPres > PRES_WARN || sHumi > HUMI_WARN) return C_SKY_OVER;
  return C_SKY_HAPPY;
}
uint16_t groundColour() {
  bool crit = sTemp>=TEMP_EXTREME && sHumi>=HUMI_EXTREME &&
              sPres>=PRES_EXTREME && sSoil>=SOIL_EXTREME;
  if (crit)              return C_GND_CRIT;
  if (sSoil >= SOIL_WARN) return C_GND_DRY;
  return C_GND_GREEN;
}

void thickLine(int x0,int y0,int x1,int y1,uint16_t col,int w=2){
  for(int i=0;i<w;i++) tft.drawLine(x0+i,y0,x1+i,y1,col);
}

void goToGame() {
  currentScreen  = SCR_GAME;
  plantState     = getPlantState();
  lastPlantState = (PlantState)99;
  bgNeedsRedraw  = true;
}


// ════════════════════════════════════════════════════════════
//  DRAW: PIXEL HEART at (cx, cy), size s, colour col
//  Uses a 7x6 pixel bitmap heart shape scaled by s
// ════════════════════════════════════════════════════════════
void drawHeart(int cx, int cy, int s, uint16_t col) {
  // Heart bitmap (7 wide x 6 tall at s=1)
  // Each row: offset from centre-left, width of filled run
  // Row 0: two bumps
  tft.fillRect(cx - 3*s, cy - 2*s, 2*s, 2*s, col);
  tft.fillRect(cx + 1*s, cy - 2*s, 2*s, 2*s, col);
  // Row 1: full top width
  tft.fillRect(cx - 3*s, cy - 0*s, 6*s, 2*s, col);
  // Row 2: slightly narrower
  tft.fillRect(cx - 2*s, cy + 2*s, 4*s, 2*s, col);
  // Row 3: narrower still
  tft.fillRect(cx - 1*s, cy + 4*s, 2*s, 2*s, col);
  // Row 4: tip
  tft.fillRect(cx,       cy + 6*s, s,   s,   col);
}


// ════════════════════════════════════════════════════════════
//  DRAW: POT
// ════════════════════════════════════════════════════════════
void drawPot(int cx, int gy) {
  tft.fillTriangle(cx-18,gy-6,  cx+18,gy-6,  cx+13,gy+18, C_POT);
  tft.fillTriangle(cx-18,gy-6,  cx+13,gy+18, cx-13,gy+18, C_POT);
  tft.fillRect(cx-20,gy-14,40,9,  C_POT_RIM);
  tft.fillRect(cx-4, gy+14, 8,5,  C_POT_DARK);
}


// ════════════════════════════════════════════════════════════
//  DRAW: FACE  (0=happy 1=sad 2=hot 3=dead)
// ════════════════════════════════════════════════════════════
void drawFace(int px, int py, int expr) {
  tft.fillCircle(px, py, 13, C_WHITE);
  if (expr == 3) {
    tft.drawLine(px-6,py-5,px-1,py,  C_BLACK); tft.drawLine(px-1,py-5,px-6,py,  C_BLACK);
    tft.drawLine(px+1,py-5,px+6,py,  C_BLACK); tft.drawLine(px+6,py-5,px+1,py,  C_BLACK);
    tft.drawLine(px-4,py+6,px+4,py+6,C_BLACK); return;
  }
  tft.fillCircle(px-4,py-3,3,C_BLACK); tft.fillCircle(px+4,py-3,3,C_BLACK);
  tft.fillCircle(px-3,py-4,1,C_WHITE); tft.fillCircle(px+5,py-4,1,C_WHITE);
  if (expr == 0) {
    tft.drawLine(px-4,py+5,px-1,py+8,C_BLACK); tft.drawLine(px-1,py+8,px+2,py+8,C_BLACK);
    tft.drawLine(px+2,py+8,px+5,py+5,C_BLACK);
    tft.fillCircle(px-7,py+3,2,C_CHEEK); tft.fillCircle(px+7,py+3,2,C_CHEEK);
  } else if (expr == 1) {
    tft.drawLine(px-4,py+8,px-1,py+5,C_BLACK); tft.drawLine(px-1,py+5,px+2,py+5,C_BLACK);
    tft.drawLine(px+2,py+5,px+5,py+8,C_BLACK);
    tft.fillCircle(px+11,py-1,3,C_SWEAT);
    tft.fillTriangle(px+11,py-7,px+8,py-1,px+14,py-1,C_SWEAT);
  } else if (expr == 2) {
    tft.drawLine(px-4,py+5,px-1,py+7,C_BLACK); tft.drawLine(px-1,py+7,px+2,py+5,C_BLACK);
    tft.drawLine(px+2,py+5,px+5,py+7,C_BLACK);
  }
}


// ════════════════════════════════════════════════════════════
//  DRAW: FLAME
// ════════════════════════════════════════════════════════════
void drawFlame(int cx, int fy) {
  int fl = (int)(sinf(animTick*0.25f)*3);
  int fr = (int)(sinf(animTick*0.30f+1.0f)*2);
  tft.fillTriangle(cx-6,fy,cx+6,fy,cx+fl,   fy-20+fr,C_FLAME1);
  tft.fillTriangle(cx-4,fy,cx+4,fy,cx+fl/2, fy-14+fr,C_FLAME2);
  tft.fillTriangle(cx-2,fy,cx+2,fy,cx,      fy-8,    C_FLAME3);
}


// ════════════════════════════════════════════════════════════
//  DRAW: BACKGROUND
//
//  HAPPY   — bright blue sky, sun, two white clouds
//  THIRSTY — dusty amber sky, haze sun, cracked ground, NO clouds
//  HOT     — burnt orange sky, heat shimmer, orange haze sun
//  OVERCAST— grey-blue sky, flat clouds, rain ONLY if humi>warn
//  CRITICAL— near-black sky, tombstones, lightning
// ════════════════════════════════════════════════════════════
void drawBackground() {
  bool crit  = sTemp>=TEMP_EXTREME && sHumi>=HUMI_EXTREME &&
               sPres>=PRES_EXTREME && sSoil>=SOIL_EXTREME;
  bool hot   = sTemp > TEMP_WARN;
  bool dry   = sSoil >= SOIL_WARN;
  bool over  = (sPres > PRES_WARN || sHumi > HUMI_WARN) && !hot && !dry && !crit;
  bool happy = !crit && !hot && !dry && !over;

  tft.fillRect(0, ANIM_TOP, 240, GROUND_Y - ANIM_TOP, skyColour());

  if (crit) {
    // ── CRITICAL: tombstones + lightning ──
    // Large tombstone
    tft.fillRect(140,78,26,34,C_GRAVE); tft.fillRect(136,76,34,6,C_GRAVE);
    tft.fillCircle(153,76,13,C_GRAVE);
    tft.setTextColor(0x4208); tft.setTextSize(1); tft.setCursor(143,93); tft.print("R.I.P");
    // Small tombstone
    tft.fillRect(30,98,16,22,C_GRAVE); tft.fillRect(27,96,22,4,C_GRAVE);
    tft.fillCircle(38,96,8,C_GRAVE);
    // Lightning bolt
    if ((animTick/6) % 8 == 0) {
      tft.drawLine(78,ANIM_TOP+2, 70,ANIM_TOP+18, 0xFFE0);
      tft.drawLine(70,ANIM_TOP+18,76,ANIM_TOP+18,0xFFE0);
      tft.drawLine(76,ANIM_TOP+18,66,ANIM_TOP+36,0xFFE0);
    }

  } else if (happy) {
    // ── HAPPY: bright sun + two clouds ──
    int sx=182, sy=54;
    for (int i=0;i<8;i++){
      float a=i*PI/4.0f;
      int x1=sx+(int)(16*cosf(a)), y1=sy+(int)(16*sinf(a));
      int x2=sx+(int)(24*cosf(a)), y2=sy+(int)(24*sinf(a));
      tft.drawLine(x1,y1,x2,y2,C_SUN_RAY); tft.drawLine(x1+1,y1,x2+1,y2,C_SUN_RAY);
    }
    tft.fillCircle(sx,sy,14,C_SUN); tft.fillCircle(sx-4,sy-4,4,C_WHITE);
    // Cloud 1
    tft.fillTriangle(18,54,58,54,38,41,C_CLOUD); tft.fillRect(18,54,40,9,C_CLOUD);
    // Cloud 2
    tft.fillTriangle(60,51,90,51,75,42,C_CLOUD); tft.fillRect(60,51,30,8,C_CLOUD);

  } else if (dry) {
    // ── THIRSTY: haze sun, NO clouds, cracked ground ──
    // Pale hazy sun
    tft.fillCircle(170,50,14,0xCC60);
    tft.fillCircle(170,50,9, 0xEE80);
    // Dust shimmer
    for (int i=0;i<6;i++) {
      tft.drawPixel(20+i*28, 55+(i%3)*8, 0x8B40);
      tft.drawPixel(34+i*28, 62+(i%3)*6, 0x8B40);
    }

  } else if (hot) {
    // ── HOT: heat shimmer lines + orange haze sun ──
    for (int i=0;i<5;i++){
      tft.drawLine(16+i*42,ANIM_TOP+4,20+i*42,ANIM_TOP+18,0x4200);
      tft.drawLine(20+i*42,ANIM_TOP+4,24+i*42,ANIM_TOP+18,0x3100);
    }
    // Haze sun
    tft.fillCircle(175,52,16,0x3000);
    tft.fillCircle(175,52,11,0x6800);
    tft.fillCircle(175,52, 7,0xCC20);

  } else if (over) {
    // ── OVERCAST: flat clouds, rain ONLY if humidity high ──
    tft.fillTriangle(14,52,84,52,49,40, C_CLOUD); tft.fillRect(14,52,70,9,C_CLOUD);
    tft.fillTriangle(78,50,158,50,118,38,C_CLOUD); tft.fillRect(78,50,80,9,C_CLOUD);
    tft.fillTriangle(143,54,223,54,183,42,C_CLOUD); tft.fillRect(143,54,80,8,C_CLOUD);
    // Rain streaks — only when humidity truly high
    if (sHumi > HUMI_WARN) {
      for (int r=0;r<6;r++){
        int rx=r*34+14; if(rx>86&&rx<154) continue;
        tft.drawLine(rx,68,rx-2,80,C_SWEAT);
        tft.drawLine(rx,86,rx-2,98,C_SWEAT);
      }
    }
  }

  // ── Ground strip ──────────────────────────────────────
  tft.fillRect(0,GROUND_Y,240,ANIM_BOT-GROUND_Y,groundColour());
  if (!crit) {
    if (dry) {
      // Cracked ground lines
      tft.drawLine(8, GROUND_Y+4, 48, GROUND_Y+2, 0x3A1E04);
      tft.drawLine(48,GROUND_Y+2, 68,GROUND_Y+8,  0x3A1E04);
      tft.drawLine(80,GROUND_Y+3,118,GROUND_Y+7,  0x3A1E04);
      tft.drawLine(148,GROUND_Y+5,188,GROUND_Y+2, 0x3A1E04);
    } else {
      // Grass tufts
      uint16_t gc = 0x2A50;
      for (int i=0;i<7;i++){
        int gx=i*32+12; if(gx>PLANT_CX-26&&gx<PLANT_CX+26) continue;
        tft.fillTriangle(gx-6,GROUND_Y,gx+6,GROUND_Y,gx,GROUND_Y-8,gc);
      }
    }
  }
  drawPot(PLANT_CX, PLANT_GY);
}


// ════════════════════════════════════════════════════════════
//  ERASE old plant pixels
// ════════════════════════════════════════════════════════════
void erasePlant(int sway) {
  uint16_t sc = skyColour(), gc = groundColour();
  int cx = PLANT_CX + sway;
  tft.fillRect(cx-28,        PLANT_GY-104,62,64,sc);
  tft.fillRect(PLANT_CX-46,  PLANT_GY-58, 98,34,sc);
  tft.fillRect(PLANT_CX-6,   PLANT_GY-44, 14,44,sc);
  tft.fillRect(PLANT_CX-6,   GROUND_Y,    14,ANIM_BOT-GROUND_Y,gc);
  drawPot(PLANT_CX, PLANT_GY);
}


// ════════════════════════════════════════════════════════════
//  DRAW: PLANT BODY
// ════════════════════════════════════════════════════════════
void drawPlantBody(int sway,int droop,int expr,uint16_t stemCol,uint16_t leafCol){
  int cx=PLANT_CX+sway, gy=PLANT_GY;
  // Love aura when love high
  if (love > 70) {
    tft.drawCircle(cx,gy-62,19,C_LOVEPINK);
    tft.drawCircle(cx,gy-62,20,C_LOVEPINK);
  }
  thickLine(PLANT_CX,gy,cx,gy-40+droop,stemCol);
  tft.fillTriangle(cx-6, gy-30+droop, cx-32,gy-22+droop, cx-6, gy-18+droop, leafCol);
  tft.fillTriangle(cx+6, gy-30+droop, cx+32,gy-22+droop, cx+6, gy-18+droop, leafCol);
  tft.fillTriangle(cx-10,gy-44+droop, cx+10,gy-44+droop, cx,   gy-68+droop, leafCol);
  drawFace(cx,gy-62+droop,expr);
  if (expr==2) drawFlame(cx,gy-77+droop);
  if (expr==1) {
    tft.fillCircle(cx+13,gy-72+droop,3,C_SWEAT);
    tft.fillTriangle(cx+13,gy-78+droop,cx+10,gy-72+droop,cx+16,gy-72+droop,C_SWEAT);
  }
}


// ════════════════════════════════════════════════════════════
//  DRAW: DEAD PLANT
// ════════════════════════════════════════════════════════════
void drawDeadPlant() {
  int cx=PLANT_CX, gy=PLANT_GY;
  thickLine(cx,gy,    cx-3, gy-22,C_LEAF_DEAD,2);
  thickLine(cx-3,gy-22,cx-17,gy-35,C_LEAF_DEAD,2);
  thickLine(cx-3,gy-22,cx+13,gy-33,C_LEAF_DEAD,2);
  tft.fillTriangle(cx-17,gy-35,cx-30,gy-28,cx-10,gy-28,C_LEAF_DEAD);
  tft.fillTriangle(cx+13,gy-33,cx+26,gy-26,cx+6, gy-26,C_LEAF_DEAD);
  drawFace(cx-3,gy-47,3);
  tft.setTextColor(C_LTGREY); tft.setTextSize(1);
  tft.setCursor(cx-10,gy-67); tft.print("R.I.P");
  drawPot(cx,gy);
}


// ════════════════════════════════════════════════════════════
//  TICK: floating hearts
//  Each heart floats upward and fades over HEART_LIFE frames
// ════════════════════════════════════════════════════════════
void tickHearts() {
  for (int i=0;i<MAX_HEARTS;i++){
    if (!hearts[i].active) continue;
    int age = hearts[i].age;
    int hx  = hearts[i].x;
    int hy  = hearts[i].y - (age * 2);   // rise 2px per frame

    // Erase previous position (overwrite with sky colour)
    // Only erase if age > 0 (nothing to erase on first frame)
    if (age > 0) {
      int prevY = hy + 2;
      tft.fillRect(hx-7, prevY-14, 18, 18, skyColour());
    }

    hearts[i].age++;
    if (hearts[i].age >= HEART_LIFE) {
      hearts[i].active = false;
      // Final erase
      tft.fillRect(hx-7, hy-14, 18, 18, skyColour());
      continue;
    }

    // Fade: full colour for first half, dimmer for second
    uint16_t col = (age < HEART_LIFE/2) ? C_LOVEPINK : C_HOTPINK;

    // Draw pixel heart (size 1 = ~14x14px)
    drawHeart(hx, hy, 1, col);
  }
}


// ════════════════════════════════════════════════════════════
//  DRAW: HEADER BAR (no clock)
// ════════════════════════════════════════════════════════════
void drawHeaderBar() {
  tft.fillRect(0,0,240,HEADER_H,C_HEADER);
  tft.setTextSize(1);
  tft.setTextColor(C_GREEN);  tft.setCursor(4, 5);  tft.print("WEATHER");
  tft.setTextColor(C_CYAN);   tft.setCursor(58,5);  tft.print("STATION");
  tft.setTextColor(C_DKGREY); tft.setCursor(118,5); tft.print("PLANT TAMA v4");
  tft.drawFastHLine(0,HEADER_H-1,240,C_DIVIDER);
}


// ════════════════════════════════════════════════════════════
//  DRAW: LOVE BAR
// ════════════════════════════════════════════════════════════
void drawLoveBar() {
  tft.fillRect(0,LOVE_BAR_Y,240,LOVE_H,C_LOVE_BG);
  tft.setTextColor(C_LOVEPINK); tft.setTextSize(1);
  tft.setCursor(4,LOVE_BAR_Y+3); tft.print("LOVE");
  int bx=34,by=LOVE_BAR_Y+3,bw=164,bh=8;
  tft.fillRect(bx,by,bw,bh,0x1008);
  int fw=(int)((love/100.0f)*bw);
  if(fw>0) tft.fillRect(bx,by,fw,bh,loveColour());
  for(int i=1;i<10;i++) tft.drawFastVLine(bx+i*(bw/10),by,bh,0x0000);
  tft.drawRect(bx,by,bw,bh,loveFlashTick>0?C_LOVEPINK:0x2010);
  char pct[5]; snprintf(pct,sizeof(pct),"%d%%",(int)love);
  tft.setTextColor(loveColour()); tft.setCursor(202,LOVE_BAR_Y+3); tft.print(pct);
  if(love<20){ tft.setTextColor(C_LOVEPINK); tft.setCursor(108,LOVE_BAR_Y+3); tft.print("needs love!"); }
  tft.drawFastHLine(0,LOVE_BAR_Y+LOVE_H-1,240,C_DIVIDER);
}


// ════════════════════════════════════════════════════════════
//  DRAW: QUADRANT TILE
//  - No highlight on tap
//  - No coloured accent letters — plain white label only
//  - No Hlth:% sub-text
// ════════════════════════════════════════════════════════════
void drawQuadrant(int idx, bool /*tapped*/, bool partial=false) {
  int qx=(idx%2==0)?0:120;
  int qy=(idx<2)?QUAD_TOP:QUAD_MID;

  float hv; uint16_t acc;
  char mainVal[16], subVal[16], label[14];

  switch(idx){
    case 0:
      hv=healthTemp(); acc=C_Q0;
      snprintf(mainVal,sizeof(mainVal),"%.1fC",sTemp);
      snprintf(subVal, sizeof(subVal), "warn: >35C");
      strcpy(label,"TEMPERATURE"); break;
    case 1:
      hv=healthHumi(); acc=C_Q1;
      snprintf(mainVal,sizeof(mainVal),"%.1f%%",sHumi);
      snprintf(subVal, sizeof(subVal), "ideal: 40-70%%");
      strcpy(label,"HUMIDITY"); break;
    case 2:
      hv=healthSoil(); acc=C_Q2;
      {
        float soilPct = soilPercentFromRaw(sSoil);
        if (isnan(soilPct)) {
          strcpy(mainVal,"NaN");
          snprintf(subVal,sizeof(subVal),"NO READING");
        } else {
          snprintf(mainVal,sizeof(mainVal),"%d%%",(int)roundf(soilPct));
          snprintf(subVal,sizeof(subVal),"%s ADC:%d", sSoil>=SOIL_WARN ? "dry" : "wet", sSoil);
        }
      }
      strcpy(label,"SOIL MOIST."); break;
    case 3:
      hv=healthPres(); acc=C_Q3;
      snprintf(mainVal,sizeof(mainVal),"%.1fhPa",sPres);
      snprintf(subVal, sizeof(subVal), "ideal: 1005-1015");
      strcpy(label,"PRESSURE"); break;
    default: return;
  }

  uint16_t bc    = barColour(hv);
  uint16_t bgCol = C_QUAD_BG;

  if (!partial) {
    tft.fillRect(qx,qy,TILE_W,TILE_H,bgCol);
    // Plain white label — no coloured badge letter
    tft.setTextColor(C_WHITE); tft.setTextSize(1);
    tft.setCursor(qx+5, qy+11); tft.print(label);
    // Grid lines
    tft.drawFastVLine(120,qy,TILE_H,C_DIVIDER);
    tft.drawFastHLine(qx,QUAD_TOP,TILE_W,C_DIVIDER);
    tft.drawFastHLine(qx,QUAD_MID,TILE_W,C_DIVIDER);
  } else {
    tft.fillRect(qx+4,qy+18,110,16,bgCol);
    tft.fillRect(qx+4,qy+36,110, 8,bgCol);
  }

  // Main value — large, colour coded
  uint16_t valCol = bc;
  if (idx == 2) {
    float soilPct = soilPercentFromRaw(sSoil);
    if (isnan(soilPct)) valCol = C_LTGREY;
    else valCol = (sSoil>=SOIL_WARN?C_RED:C_GREEN);
  }
  tft.setTextColor(valCol);
  tft.setTextSize(2); tft.setCursor(qx+4,qy+18); tft.print(mainVal); tft.setTextSize(1);

  // Sub text
  tft.setTextColor(0x3838); tft.setCursor(qx+4,qy+36); tft.print(subVal);

  // Health bar
  int bx=qx+4, by=qy+47, bw=112, bh=9;
  tft.fillRect(bx,by,bw,bh,0x1010);
  int fw=(int)(hv*bw); if(fw>0) tft.fillRect(bx,by,fw,bh,bc);
  tft.drawRect(bx,by,bw,bh,0x1818);
  tft.fillRect(bx+bw-22,by+1,20,7,(fw>bw-22)?bc:0x1010);
  char pct[5]; snprintf(pct,sizeof(pct),"%d%%",(int)(hv*100));
  tft.setTextColor(hv>0.5f?C_WHITE:C_LTGREY); tft.setCursor(bx+bw-22,by+1); tft.print(pct);

  // Warning flash
  if(hv<0.3f&&(millis()/500)%2==0){
    tft.setTextColor(C_RED); tft.setCursor(qx+TILE_W-12,qy+11); tft.print("!");
  }
}


// ════════════════════════════════════════════════════════════
//  DRAW: OVERALL HEALTH BAR
// ════════════════════════════════════════════════════════════
void drawHealthBar(bool partial=false) {
  float ov=healthOverall();
  if(!partial){
    tft.fillRect(0,QUAD_BOT,240,320-QUAD_BOT,0x0402);
    tft.drawFastHLine(0,QUAD_BOT,240,C_DIVIDER);
    tft.setTextColor(C_GREEN); tft.setTextSize(1);
    tft.setCursor(4,QUAD_BOT+8); tft.print("HEALTH");
  } else {
    tft.fillRect(44,QUAD_BOT+4,150,8,0x0402);
    tft.fillRect(198,QUAD_BOT+4,40,10,0x0402);
  }
  int bx=44,by=QUAD_BOT+4,bw=150,bh=8;
  tft.fillRect(bx,by,bw,bh,0x1010);
  int fw=(int)(ov*bw); if(fw>0) tft.fillRect(bx,by,fw,bh,barColour(ov));
  tft.drawRect(bx,by,bw,bh,0x2020);
  char pct[5]; snprintf(pct,sizeof(pct),"%d%%",(int)(ov*100));
  tft.setTextColor(barColour(ov)); tft.setCursor(198,QUAD_BOT+8); tft.print(pct);
}


// ════════════════════════════════════════════════════════════
//  DRAW: HOME SCREEN
// ════════════════════════════════════════════════════════════
void drawHomeScreen() {
  tft.fillScreen(C_BG);
  // Starfield
  uint16_t sc[3]={0x4208,0x630C,0x2104};
  for(int i=0;i<55;i++) tft.drawPixel((i*47+13)%228+6,(i*31+7)%290+8,sc[i%3]);

  tft.setTextSize(2);
  tft.setTextColor(C_GREEN); tft.setCursor(38,28); tft.print("WEATHER");
  tft.setTextColor(C_CYAN);  tft.setCursor(42,50); tft.print("STATION");
  tft.setTextSize(1);
  tft.setTextColor(0x2828);  tft.setCursor(28,70); tft.print("PLANT TAMAGOTCHI EDITION");

  // Mini plant
  int hpx=120,hpy=145;
  thickLine(hpx,hpy,hpx,hpy-30,C_STEM,2);
  tft.fillTriangle(hpx-4,hpy-18,hpx-22,hpy-10,hpx-4,hpy-8, C_LEAF);
  tft.fillTriangle(hpx+4,hpy-18,hpx+22,hpy-10,hpx+4,hpy-8, C_LEAF);
  tft.fillTriangle(hpx-8,hpy-28,hpx+8, hpy-28,hpx,  hpy-46,C_LEAF);
  tft.fillCircle(hpx,hpy-50,10,C_WHITE);
  tft.fillCircle(hpx-3,hpy-52,2,C_BLACK); tft.fillCircle(hpx+3,hpy-52,2,C_BLACK);
  tft.drawLine(hpx-3,hpy-46,hpx-1,hpy-44,C_BLACK);
  tft.drawLine(hpx-1,hpy-44,hpx+2,hpy-44,C_BLACK);
  tft.drawLine(hpx+2,hpy-44,hpx+4,hpy-46,C_BLACK);
  tft.fillCircle(hpx-5,hpy-48,2,C_CHEEK); tft.fillCircle(hpx+5,hpy-48,2,C_CHEEK);
  tft.fillTriangle(hpx-12,hpy-4,hpx+12,hpy-4,hpx+8, hpy+12,C_POT);
  tft.fillTriangle(hpx-12,hpy-4,hpx+8, hpy+12,hpx-8,hpy+12,C_POT);
  tft.fillRect(hpx-13,hpy-10,26,7,C_POT_RIM);

  // Love preview bar
  tft.fillRect(20,160,200,12,0x0808); tft.drawRect(20,160,200,12,0x1818);
  int previewFw=(int)((love/100.0f)*198);
  if(previewFw>0) tft.fillRect(21,161,previewFw,10,loveColour());
  tft.setTextColor(C_LOVEPINK); tft.setTextSize(1); tft.setCursor(24,163); tft.print("LOVE");

  // Tap anywhere prompt
  tft.drawRect(4,4,232,312,C_GREEN); tft.drawRect(6,6,228,308,0x0340);
  tft.fillRect(30,182,180,40,0x0820); tft.drawRect(30,182,180,40,C_GREEN);
  tft.setTextColor(C_GREEN); tft.setTextSize(2);
  tft.setCursor(36,190); tft.print("TAP ANYWHERE");
  tft.setTextSize(1); tft.setTextColor(0x3464);
  tft.setCursor(72,208); tft.print("to start monitoring");

  // Sensor dots
  tft.fillRect(10,234,220,22,0x0808);
  uint16_t dcols[4]={C_Q0,C_Q1,C_Q2,C_Q3};
  const char* dlbls[4]={"TEMP","HUMI","SOIL","PRES"};
  for(int i=0;i<4;i++){
    int dx=28+i*56;
    tft.fillCircle(dx,243,4,dcols[i]);
    tft.setTextColor(0x2828); tft.setTextSize(1);
    tft.setCursor(dx-8,250); tft.print(dlbls[i]);
  }
  tft.setTextColor(0x1818); tft.setTextSize(1);
  tft.setCursor(16,266); tft.print("ESP32  DHT22  BMP280  SOIL");
  tft.setCursor(20,280); tft.print("XPT2046 TOUCH  |  v4.0");
}

void drawSimpleScreenFrame() {
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRect(0, 0, 240, 320, C_CYAN);
  tft.fillRect(0, 0, 240, 24, C_HEADER);
  tft.setTextSize(2);
  tft.setTextColor(C_WHITE);
  tft.setCursor(8, 4);
  tft.print("WEATHER LIVE");

  tft.setTextSize(1);
  tft.setTextColor(C_LTGREY);
  tft.setCursor(8, 30);  tft.print("Temp:");
  tft.setCursor(8, 58);  tft.print("Humi:");
  tft.setCursor(8, 86);  tft.print("Pres:");
  tft.setCursor(8, 114); tft.print("Soil:");
  tft.setCursor(8, 142); tft.print("Health:");
  tft.setCursor(8, 170); tft.print("MQTT:");
}

void updateSimpleScreen() {
  char line[32];

  tft.fillRect(70, 28, 160, 160, ILI9341_BLACK);
  tft.setTextSize(2);

  tft.setTextColor(C_YELLOW);
  snprintf(line, sizeof(line), "%5.1f C", sTemp);
  tft.setCursor(70, 30); tft.print(line);

  tft.setTextColor(C_CYAN);
  snprintf(line, sizeof(line), "%5.1f %%", sHumi);
  tft.setCursor(70, 58); tft.print(line);

  tft.setTextColor(C_GREEN);
  snprintf(line, sizeof(line), "%6.1f", sPres);
  tft.setCursor(70, 86); tft.print(line);

  float soilPct = soilPercentFromRaw(sSoil);
  tft.setTextColor(C_ORANGE);
  if (isnan(soilPct)) snprintf(line, sizeof(line), "NO DATA");
  else snprintf(line, sizeof(line), "%5.0f %%", soilPct);
  tft.setCursor(70, 114); tft.print(line);

  tft.setTextColor(barColour(healthOverall()));
  snprintf(line, sizeof(line), "%5.0f %%", healthOverall() * 100.0f);
  tft.setCursor(70, 142); tft.print(line);

  tft.setTextColor(mqtt.connected() ? C_GREEN : C_RED);
  snprintf(line, sizeof(line), "%s", mqtt.connected() ? "CONNECTED" : "OFFLINE");
  tft.setCursor(70, 170); tft.print(line);
}


// ════════════════════════════════════════════════════════════
//  FULL GAME SCENE REDRAW
// ════════════════════════════════════════════════════════════
void redrawGameScene() {
  tft.fillScreen(C_BG);
  drawHeaderBar();
  drawLoveBar();
  drawBackground();
  if(plantState==PS_DEAD) drawDeadPlant();
  else drawPlantBody(0,0,0,C_STEM,C_LEAF);
  for(int i=0;i<4;i++) drawQuadrant(i,false,false);
  drawHealthBar(false);
  // Clear heart slots on full redraw
  for(int i=0;i<MAX_HEARTS;i++) hearts[i].active=false;
  prevSway=0; bgNeedsRedraw=false;
}


// ════════════════════════════════════════════════════════════
//  TOUCH HANDLER
// ════════════════════════════════════════════════════════════
bool handleTouch() {
  if(!touch.touched()) return false;
  TS_Point p=touch.getPoint();
  if(p.z<TOUCH_MIN_Z) return false;
  if(p.x < TS_MINX || p.x > TS_MAXX || p.y < TS_MINY || p.y > TS_MAXY) return false;

  unsigned long now=millis();
  if(now-lastTouchMs<TOUCH_DEBOUNCE) return false;
  lastTouchMs=now;

  int tx=mapTouch(p.x,TS_MINX,TS_MAXX,0,239);
  int ty=mapTouch(p.y,TS_MINY,TS_MAXY,0,319);

  // HOME: any touch → game
  if(currentScreen==SCR_HOME){ goToGame(); return true; }

  // GAME: pet plant — generous radius around plant centre
  int plantScreenY = PLANT_GY - 30;  // approx visual centre of plant
  int dx = tx - PLANT_CX;
  int dy = ty - plantScreenY;
  if((dx*dx + dy*dy) < PLANT_TAP_R*PLANT_TAP_R){
    love = min((float)LOVE_MAX, love + LOVE_PER_TAP);
    loveFlashTick = 30;
    wiggle = 10;
    // Spawn heart near the tap point, clamped to anim region
    int hx = constrain(tx, 30, 210);
    int hy = constrain(ty, ANIM_TOP+20, ANIM_BOT-10);
    spawnHeart(hx, hy);
    drawLoveBar();
    return true;
  }

  // GAME: tap quadrant — partial refresh, no highlight
  for(int i=0;i<4;i++){
    int qx=(i%2==0)?0:120;
    int qy=(i<2)?QUAD_TOP:QUAD_MID;
    if(tx>=qx&&tx<qx+TILE_W&&ty>=qy&&ty<qy+TILE_H){
      drawQuadrant(i,false,true);
      return true;
    }
  }
  return false;
}


// ════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  // Keep both CS lines deasserted while peripherals initialize on shared SPI bus.
  pinMode(TFT_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TOUCH_CS, HIGH);

  tft.begin(); tft.setRotation(0); tft.fillScreen(C_BG);
  touch.begin(); touch.setRotation(0);
  dht.begin();

  pinMode(SOIL_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(SOIL_PIN, ADC_11db);

  if(!bmp.begin(0x76)) {
    bmpAvailable = false;
    Serial.println("BMP280 not found!");
  } else {
    bmpAvailable = true;
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
  }

  for(int i=0;i<MAX_HEARTS;i++) hearts[i].active=false;

  if (SIMPLE_UI_MODE) {
    currentScreen = SCR_GAME;
    drawSimpleScreenFrame();
    updateSimpleScreen();
  } else {
    drawHomeScreen();
  }
  lastFrameMs=lastSensorMs=lastLoveDecay=millis();
  scheduleNextLoveDecay();

  initialize();
}


// ════════════════════════════════════════════════════════════
//  LOOP
// ════════════════════════════════════════════════════════════
void loop() {
  unsigned long now=millis();

  if (SIMPLE_UI_MODE) {
    if(now-lastSensorMs>=SENSOR_MS){
      lastSensorMs=now;
      float h=dht.readHumidity(), t=dht.readTemperature();
      sHumi = isnan(h) ? NAN : h;
      sTemp = isnan(t) ? NAN : t;

      if (bmpAvailable) {
        float p = bmp.readPressure()/100.0f;
        sPres = (isnan(p) || p < 300.0f || p > 1200.0f) ? NAN : p;
      } else {
        sPres = NAN;
      }

      int rawSoil = readSoilRaw();
      sSoil = (int)(0.75f * (float)sSoil + 0.25f * (float)rawSoil);
      Serial.printf("SIMPLE UI | T:%.1f H:%.1f P:%.1f S:%d\n", sTemp,sHumi,sPres,sSoil);
      updateSimpleScreen();
    }
    return;
  }

  if(handleTouch()) return;

  // ── Love decay ─────────────────────────────────────────
  if(now-lastLoveDecay>=nextLoveDecay){
    lastLoveDecay=now; scheduleNextLoveDecay();
    love=max(0.0f,love-1.0f);
    if(currentScreen==SCR_GAME) drawLoveBar();
  }
  if(loveFlashTick>0) loveFlashTick--;

  if(currentScreen==SCR_HOME){
    if(now-lastFrameMs>=FRAME_MS){ animTick++; lastFrameMs=now; }
    return;
  }

  // ── Animation frame ────────────────────────────────────
  if(now-lastFrameMs>=FRAME_MS){
    lastFrameMs=now;

    PlantState ps=getPlantState();
    if(ps!=lastPlantState||bgNeedsRedraw){
      lastPlantState=ps; plantState=ps; redrawGameScene();
    }

    if(plantState!=PS_DEAD){
      float s=sinf(animTick*0.1f);
      int newSway=0,newDroop=0;
      uint16_t stemCol=C_STEM,leafCol=C_LEAF; int expr=0;

      if(plantState==PS_HAPPY){
        newSway=(int)(s*7.0f); expr=0;
      } else if(plantState==PS_THIRSTY){
        newSway=(int)(s*3.0f);
        newDroop=(int)((sinf(animTick*0.04f)+1.0f)*2.5f);
        stemCol=C_LEAF_DRY; leafCol=C_LEAF_DRY; expr=1;
      } else if(plantState==PS_HOT){
        newSway=((animTick%2)==0)?-5:5;
        leafCol=C_LEAF_HOT; expr=2;
      }

      if(newSway!=prevSway||plantState==PS_THIRSTY){
        erasePlant(prevSway);
        drawPlantBody(newSway,newDroop,expr,stemCol,leafCol);
        prevSway=newSway;
      }
    }

    // Tick floating hearts
    tickHearts();

    if(wiggle>0) wiggle=max(0,wiggle-1);
    animTick++;
  }

  // ── Sensor read ────────────────────────────────────────
  if(now-lastSensorMs>=SENSOR_MS){
    lastSensorMs=now;
    float h=dht.readHumidity(), t=dht.readTemperature();
    sHumi = isnan(h) ? NAN : h;
    sTemp = isnan(t) ? NAN : t;

    if (bmpAvailable) {
      float p = bmp.readPressure()/100.0f;
      sPres = (isnan(p) || p < 300.0f || p > 1200.0f) ? NAN : p;
    } else {
      sPres = NAN;
    }

    int rawSoil = readSoilRaw();
    sSoil = (int)(0.75f * (float)sSoil + 0.25f * (float)rawSoil);
    Serial.printf("T:%.1f H:%.1f P:%.1f S:%d Love:%.0f Health:%.0f%%\n",
                  sTemp,sHumi,sPres,sSoil,love,healthOverall()*100.0f);

    if(floatChanged(sTemp,lastTempDrawn,0.05f)){  drawQuadrant(0,false,true); lastTempDrawn=sTemp; }
    if(floatChanged(sHumi,lastHumiDrawn,0.05f)){  drawQuadrant(1,false,true); lastHumiDrawn=sHumi; }
    float prevSoilPct = soilPercentFromRaw((int)lastSoilDrawn);
    float currSoilPct = soilPercentFromRaw(sSoil);
    bool soilNanFlip = (isnan(prevSoilPct) != isnan(currSoilPct));
    if(abs((float)sSoil-lastSoilDrawn)>2 || soilNanFlip){ drawQuadrant(2,false,true); lastSoilDrawn=(float)sSoil; }
    if(floatChanged(sPres,lastPresDrawn,0.05f)){  drawQuadrant(3,false,true); lastPresDrawn=sPres; }

    float nh=healthOverall();
    if(floatChanged(nh,lastHealthDrawn,0.005f)){ drawHealthBar(true); lastHealthDrawn=nh; }
  }
}

// ════════════════════════════════════════════════════════════
//  TEMPERATURE CONVERSION FUNCTIONS
// ════════════════════════════════════════════════════════════

/**
 * @brief Convert Celsius to Fahrenheit
 * @param celsius Temperature in Celsius
 * @return Temperature in Fahrenheit
 */
float convert_Celsius_to_fahrenheit(float celsius) {
  return (celsius * 9.0f / 5.0f) + 32.0f;
}

/**
 * @brief Convert Fahrenheit to Celsius
 * @param fahrenheit Temperature in Fahrenheit
 * @return Temperature in Celsius
 */
float convert_fahrenheit_to_Celsius(float fahrenheit) {
  return (fahrenheit - 32.0f) * 5.0f / 9.0f;
}

/**
 * @brief Calculate heat index in Fahrenheit
 * @param tempF Temperature in Fahrenheit
 * @param humidity Relative humidity percentage (0-100)
 * @return Heat index in Fahrenheit
 */
float calcHeatIndex(float tempF, float humidity) {
  // Heat Index calculated in Fahrenheit
  // NOAA formula for heat index
  float c1 = -42.379f;
  float c2 = 2.04901523f;
  float c3 = 10.14333127f;
  float c4 = -0.22475541f;
  float c5 = -0.00683783f;
  float c6 = -0.05481717f;
  float c7 = 0.00122874f;
  float c8 = 0.00085282f;
  float c9 = -0.00000199f;

  float hi = c1 + (c2 * tempF) + (c3 * humidity) + (c4 * tempF * humidity) +
             (c5 * tempF * tempF) + (c6 * humidity * humidity) +
             (c7 * tempF * tempF * humidity) + (c8 * tempF * humidity * humidity) +
             (c9 * tempF * tempF * humidity * humidity);

  return hi;
}

// ════════════════════════════════════════════════════════════
//  UTILITY FUNCTIONS
// ════════════════════════════════════════════════════════════

/**
 * @brief Check if a double number is valid (not NaN, not Infinity)
 * @param number The number to check
 * @return true if valid, false if NaN or Infinity
 */
bool isNumber(double number){       
  char item[20];
  snprintf(item, sizeof(item), "%f\n", number);
  if( isdigit(item[0]) )
    return true;
  return false; 
} 

/**
 * @brief Get current timestamp from NTP
 * @return unsigned long timestamp representing current time
 */
unsigned long getTimeStamp(void) {
  // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
  time_t now;         
  time(&now); // Retrieve time[Timestamp] from system and save to &now variable
  return now;
}


/**
 * @brief Publish message to MQTT topic
 * @param topic MQTT topic to publish to
 * @param payload Message payload
 * @return true if publish successful, false otherwise
 */
bool publish(const char *topic, const char *payload){   
  bool res = false;
  try{
    res = mqtt.publish(topic,payload);
    // Serial.printf("\nres : %d\n",res);
    if(!res){
      res = false;
      throw false;
    }
  }
  catch(...){
  Serial.printf("\nError (%d) >> Unable to publish message\n", res);
  }
  return res;
}

/**
 * @brief MQTT callback function - handles received messages on subscribed topics
 * @param topic MQTT topic of received message
 * @param payload Message payload bytes
 * @param length Length of payload
 */
void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic );

  char received[256] = {0};
  if (length >= sizeof(received)) {
    Serial.println("Payload too large, dropping message");
    return;
  }

  for (unsigned int i = 0; i < length; i++) {
    received[i] = (char)payload[i];
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
}


/**
 * @brief Task for button checking/touch input handling
 * @param pvParameters Task parameter (unused)
 */
void vButtonCheck( void * pvParameters ) {
  configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
  
  for ( ;; ) {
    handleTouch();
    vTaskDelay(50 / portTICK_PERIOD_MS);  // Check every 50ms
  }
}

/**
 * @brief Task for updating sensor data and publishing to MQTT
 * Reads temperature, humidity, pressure, and soil moisture from all sensors and publishes JSON formatted data
 * @param pvParameters Task parameter (unused)
 */
void vUpdate( void * pvParameters ) {
  configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
  
  for ( ;; ) {
    // Use values sampled in loop() to avoid duplicate sensor reads across tasks.
    float t = sTemp;
    float h = sHumi;
    float p = sPres;
    int soil = sSoil;
    float soilPct = soilPercentFromRaw(soil);
    
    // Check if temperature reading is valid
    if(isNumber(t)){
      // Publish update according to schema:
      // {"id": "student_id", "timestamp": 1702212234, "temperature": 30, "humidity":90, "pressure": 1013.25, "soil": 65, "heatindex": 30}

      // 1. Create JSON object
      StaticJsonDocument<256> doc; // Create JSON object
      
      // 2. Create message buffer/array to store serialized JSON object
      char message[256] = {0};
      
      // 3. Add key:value pairs to JSON object based on schema
      doc["id"]           = "620169500"; // Add your ID number here
      doc["timestamp"]    = getTimeStamp();
      doc["temperature"]  = t;
      doc["humidity"]     = h;
      doc["pressure"]     = isNumber(p) ? p : 0;  // Add pressure (or 0 if invalid)
      doc["soil"]         = soilPct;               // Add soil moisture percentage
      doc["heatindex"]    = convert_fahrenheit_to_Celsius(calcHeatIndex(convert_Celsius_to_fahrenheit(t), h));

      // 4. Serialize / Convert JSON object to JSON string and store in message array
      serializeJson(doc, message);
        
      // 5. Publish message to a topic subscribed to by both backend and frontend
      if(mqtt.connected()){
        publish(pubtopic, message);
        Serial.printf("Sensor Data: T=%.2f C, H=%.2f%%, P=%.2f hPa, Soil=%.2f%%\n", t, h, isNumber(p) ? p : 0, soilPct);
      }              
    }
       
    vTaskDelay(1000 / portTICK_PERIOD_MS);  
  }
}