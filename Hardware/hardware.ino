#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include "DHT.h"
#include <Adafruit_BMP280.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// 1. ALL DEFINES FIRST
#define DHTPIN      2
#define DHTTYPE     DHT22
#define SOIL_PIN    4
#define TFT_CS      5
#define TFT_DC      17
#define TFT_RST     16
#define TFT_CLK     18
#define TFT_MOSI    23
#define TFT_MISO    19

#define SOIL_WET_ADC   1300   
#define SOIL_DRY_ADC   4095   
#define SOIL_WARN_PCT    25   
#define SOIL_DEAD_PCT    10   

#define TEMP_WARN     35.0f
#define TEMP_EXTREME  38.0f
#define HUMI_WARN     70.0f
#define HUMI_EXTREME  80.0f
#define PRES_WARN   1020.0f
#define PRES_EXTREME 1022.0f
#define PRES_LOW_NORMAL 980.0f

#define ICON_H      128
#define ICON_CX     120
#define ICON_CY      65

// 2. ENUMS AND TYPES
enum WeatherState { WS_SUNNY, WS_CLOUDY, WS_RAINY, WS_EXTREME };

// 3. GLOBAL OBJECTS
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// 4. GLOBAL VARIABLES (State Tracking)
WeatherState lastState = WS_SUNNY;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000;
bool bmpAvailable = false;

float lastTemp = -999, lastHumi = -999, lastPres = -999;
int   lastSoil = -1;
bool  lastTempWarn = false, lastHumiWarn = false;
bool  lastPresWarn = false, lastSoilWarn = false;

// 5. HELPER FUNCTIONS (Must be defined before Loop)

void drawTile(int idx, const char* label, const char* mainVal, const char* unit, const char* subVal, bool warn) {
  // Add your specific drawTile implementation here from previous version
}

void drawIconRegion(WeatherState ws) {
  // Add your specific drawing logic here
}

WeatherState getWeatherState(float t, float h, float p, int sPct) {
    bool extreme = (t >= TEMP_EXTREME) && (h >= HUMI_EXTREME) && (p >= PRES_EXTREME) && (sPct <= SOIL_DEAD_PCT);
    if (extreme)        return WS_EXTREME;
    if (h >= HUMI_WARN) return WS_RAINY;
    if (p >= PRES_WARN) return WS_CLOUDY;
    return WS_SUNNY;
}

// 6. SETUP
void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(0);
  dht.begin();
  
  if (!bmp.begin(0x76)) {
    bmpAvailable = false;
  } else {
    bmpAvailable = true;
  }
}

// 7. LOOP
void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate >= updateInterval) {
    lastUpdate = currentMillis;

    float humi   = dht.readHumidity();
    float tempC  = dht.readTemperature();
    int   soilRaw = analogRead(SOIL_PIN);
    float presHP = 1013.25;

    // Soil Percentage Logic
    int soilPct = map(soilRaw, SOIL_DRY_ADC, SOIL_WET_ADC, 0, 100);
    soilPct = constrain(soilPct, 0, 100);

    if (isnan(humi) || isnan(tempC)) return;

    if (bmpAvailable) {
      presHP = bmp.readPressure() / 100.0F;
    }

    bool tempWarn = (tempC > TEMP_WARN);
    bool humiWarn = (humi > HUMI_WARN);
    bool presWarn = bmpAvailable ? (presHP > PRES_WARN || presHP < PRES_LOW_NORMAL) : false;
    bool soilWarn = (soilPct < SOIL_WARN_PCT);

    WeatherState ws = getWeatherState(tempC, humi, presHP, soilPct);
    
    if (ws != lastState) {
      drawIconRegion(ws);
      lastState = ws;
    }

    // Update Soil Tile
    if (soilPct != lastSoil || soilWarn != lastSoilWarn) {
      char mainVal[12], subVal[16];
      snprintf(mainVal, sizeof(mainVal), "%d%%", soilPct);
      snprintf(subVal, sizeof(subVal), soilWarn ? "DRY" : "OK");
      drawTile(3, "SOIL MOISTURE", mainVal, "Capacity", subVal, soilWarn);
      lastSoil = soilPct;
      lastSoilWarn = soilWarn;
    }
    
    // ... Repeat logic for Temp, Humi, and Pressure tiles ...
  }
}