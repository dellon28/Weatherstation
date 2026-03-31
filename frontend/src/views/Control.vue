<template>
  <v-app class="lab-bg">
    <!-- Lab Navigation -->
    <v-app-bar flat class="bg-dark border-b-sm" density="compact" dark color="#1a1a1a">
      <div class="px-4 d-flex align-center w-100">
        <v-icon color="green" class="mr-2">mdi-transition-masked</v-icon>
        <div class="pixel-font-sm text-green mr-6">IOT DIGITAL TWIN LAB <span class="version-tag">V3.0</span></div>
        <v-spacer></v-spacer>
        <div class="d-flex align-center mr-4">
          <v-icon size="small" class="mr-2" :color="connected ? 'green' : 'red'">mdi-broadcast</v-icon>
          <span class="pixel-font-xs text-white opacity-60 mr-2">MCU: ESP32-WROOM-32D</span>
          <v-chip size="x-small" :color="connected ? 'green' : 'red'" label class="pixel-font-xs">
            {{ connected ? 'LIVE_SYNC' : 'OFFLINE' }}
          </v-chip>
        </div>
      </div>
    </v-app-bar>

    <v-main>
      <v-container fluid class="pa-6">
        <v-row>
          <!-- Left Column: Virtual Environment Simulator -->
          <v-col cols="12" md="7">
            <v-card class="pixel-card-dark pa-0 overflow-hidden bg-black relative" style="height: 600px;">
              <!-- Simulation Overlay Info -->
              <div class="lab-header pa-4 d-flex justify-space-between align-start">
                <div class="glass-hud pa-3">
                  <div class="pixel-font-sm text-green mb-1">ENV_SIM_01: ACTIVE</div>
                  <div class="d-flex gap-2">
                    <div class="hud-stat">
                      <div class="pixel-font-xs text-grey">TEMP</div>
                      <div class="pixel-font-sm text-white">{{ temperature }}°C</div>
                    </div>
                    <div class="hud-stat">
                      <div class="pixel-font-xs text-grey">VPD</div>
                      <div class="pixel-font-sm text-white">1.2kPa</div>
                    </div>
                  </div>
                </div>
                <div class="text-right glass-hud pa-3">
                  <div class="pixel-font-xs text-blue">LATENCY: 42ms</div>
                  <div class="pixel-font-xs text-orange">MCU_CORE: 240MHz</div>
                </div>
              </div>

              <!-- 3D/Sprite Simulation Area -->
              <div class="sim-container d-flex align-center justify-center">
                <!-- Floating Sensor Data Tags -->
                <div class="floating-data" :style="{ top: '30%', left: '25%' }">
                  <div class="data-tag-line"></div>
                  <div class="data-tag-box">
                    <div class="pixel-font-xs text-blue">HUMIDITY</div>
                    <div class="pixel-font-sm">{{ humidity }}%</div>
                  </div>
                </div>

                <!-- Dynamic Weather Effects -->
                <div :class="['weather-layer', { 'raining': humidity > 80, 'heat-wave': temperature > 30 }]">
                  <div class="plant-platform">
                    <!-- Scanner Beam Effect -->
                    <div class="scanner-beam"></div>
                    
                    <img 
                      src="C:\Users\dello\Desktop\ELET2415\Weatherstation\frontend\src\assets\flower.png" 
                      alt="Digital Twin" 
                      class="twin-sprite"
                      :style="twinVisuals"
                    />
                    <!-- Holographic Circle Under Plant -->
                    <div class="hologram-ring"></div>
                  </div>
                </div>
                
                <!-- Floor with Grid -->
                <div class="lab-floor-grid"></div>
              </div>

              <!-- Simulation Controls -->
              <div class="lab-footer pa-4">
                <v-row align="center" no-gutters>
                  <v-col cols="3">
                    <div class="pixel-font-xs text-white">HISTORY_PLAYBACK</div>
                  </v-col>
                  <v-col cols="9">
                    <v-slider
                      v-model="simScrub"
                      :max="100"
                      step="1"
                      color="green"
                      track-color="grey-darken-3"
                      hide-details
                      prepend-icon="mdi-history"
                      @update:model-value="simulateHistory"
                    ></v-slider>
                  </v-col>
                </v-row>
              </div>
            </v-card>
          </v-col>

          <!-- Right Column: Hardware Interface & Diagnostics -->
          <v-col cols="12" md="5">
            <!-- ESP32 Board Map -->
            <v-card class="pixel-card-dark pa-4 bg-dark-card mb-4">
              <div class="pixel-font-sm text-white mb-4 border-b-green pb-2 d-flex justify-space-between">
                <span>HARDWARE INTERFACE</span>
                <span class="pixel-font-xs text-grey">REVISION: A2</span>
              </div>
              
              <div class="esp32-visualizer mx-auto my-6">
                <div class="mcu-body">
                  <!-- Boot Button -->
                  <div class="mcu-btn boot-btn"></div>
                  <!-- RST Button -->
                  <div class="mcu-btn rst-btn"></div>
                  
                  <div class="pins-left">
                    <div v-for="n in 15" :key="'l'+n" :class="['pin', { 'pin-active': isPinActive('L', n), 'pin-pwr': n === 1 }]"></div>
                  </div>
                  <div class="chip-center d-flex flex-column align-center justify-center">
                    <div class="mcu-logo"></div>
                    <span class="pixel-font-xs" style="font-size: 6px; letter-spacing: 1px;">ESP32-WROOM</span>
                    <!-- Status LED -->
                    <div :class="['onboard-led', { 'blink': connected }]"></div>
                  </div>
                  <div class="pins-right">
                    <div v-for="n in 15" :key="'r'+n" :class="['pin', { 'pin-active': isPinActive('R', n), 'pin-gnd': n === 15 }]"></div>
                  </div>
                </div>
              </div>

              <!-- Pin Metadata -->
              <div class="bg-black pa-4 rounded border-green">
                <div class="d-flex justify-space-between mb-2">
                  <span class="pixel-font-xs text-grey">GPIO 32 (ADC)</span>
                  <div class="text-right">
                    <div class="pixel-font-xs text-green">SOIL: {{ soilMoisture }}%</div>
                    <div class="pixel-font-xs text-grey opacity-50">{{ (soilMoisture * 40.95).toFixed(0) }} RAW</div>
                  </div>
                </div>
                <v-progress-linear :model-value="soilMoisture" color="green" height="4" class="mb-3"></v-progress-linear>
                
                <div class="d-flex justify-space-between mb-2">
                  <span class="pixel-font-xs text-grey">GPIO 33 (I2C)</span>
                  <span class="pixel-font-xs text-blue">DHT_BUS: 1.25V</span>
                </div>
                <v-progress-linear :model-value="humidity" color="blue" height="4"></v-progress-linear>
              </div>
            </v-card>

            <!-- MongoDB Live Stream -->
            <v-card class="pixel-card-dark pa-4 bg-dark-card">
              <div class="pixel-font-sm text-white mb-4 border-b-blue pb-2 d-flex justify-space-between">
                <span>MONGODB_STREAM</span>
                <v-icon size="x-small" color="blue">mdi-sync</v-icon>
              </div>
              <div class="console-box pa-2">
                <div v-for="(log, idx) in logs" :key="idx" class="console-line">
                  <span class="text-green">[{{ log.time }}]</span>
                  <span class="text-white ml-2">{{ log.msg }}</span>
                </div>
                <div class="cursor-blink">_</div>
              </div>
            </v-card>
          </v-col>
        </v-row>
      </v-container>
    </v-main>
  </v-app>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';

const connected = ref(true);
const soilMoisture = ref(65);
const temperature = ref(24.5);
const humidity = ref(62);
const simScrub = ref(100);

const logs = ref([
  { time: '14:20:01', msg: 'DB_INIT: MONGODB_ATLAS' },
  { time: '14:20:05', msg: 'COLLECTION: SENSOR_READINGS' },
  { time: '14:20:10', msg: 'ESP32_GPIO_ATTACH: 32, 33' }
]);

const twinVisuals = computed(() => {
  const scale = 0.6 + (soilMoisture.value / 150);
  let filter = 'none';
  if (soilMoisture.value < 30) filter = 'sepia(0.6) saturate(0.5) contrast(1.1)';
  if (temperature.value > 30) filter = 'hue-rotate(330deg) brightness(1.3) drop-shadow(0 0 10px rgba(255,0,0,0.3))';
  return { transform: `scale(${scale})`, filter: filter };
});

const isPinActive = (side, index) => {
  if (side === 'L' && index === 6) return true; // Analog Soil
  if (side === 'R' && index === 4) return true; // DHT
  return false;
};

const simulateHistory = (val) => {
  soilMoisture.value = Math.floor(20 + (val * 0.7));
  temperature.value = Math.floor(18 + (val * 0.15));
  humidity.value = Math.floor(30 + (val * 0.6));
};

onMounted(() => {
  setInterval(() => {
    const now = new Date();
    const timeStr = now.toTimeString().split(' ')[0];
    const events = [
      `POST_DOC: VWC_${soilMoisture.value}%`,
      `SIGNAL_STRENGTH: -${Math.floor(Math.random()*20+50)}dBm`,
      `ACK_RECV: PID_${Math.floor(Math.random()*9999)}`,
      `BUFFER_SYNC: SUCCESS`
    ];
    logs.value.push({ time: timeStr, msg: events[Math.floor(Math.random()*events.length)] });
    if (logs.value.length > 8) logs.value.shift();
  }, 3500);
});
</script>

<style scoped>
@import url('https://fonts.googleapis.com/css2?family=Press+Start+2P&display=swap');

.lab-bg { background-color: #080808 !important; }
.bg-dark { background-color: #121212 !important; }
.bg-dark-card { background-color: #0f0f0f !important; }
.version-tag { font-size: 8px; vertical-align: super; opacity: 0.5; margin-left: 4px; }

.pixel-font-sm { font-family: 'Press Start 2P', cursive; font-size: 0.7rem; }
.pixel-font-xs { font-family: 'Press Start 2P', cursive; font-size: 0.5rem; }

.pixel-card-dark {
  border: 2px solid #2a2a2a !important;
  border-radius: 0 !important;
  box-shadow: 0 0 20px rgba(0, 255, 100, 0.03);
}

.border-green { border: 1px solid #2ecc71; }
.text-green { color: #2ecc71; }
.text-blue { color: #3498db; }
.text-orange { color: #e67e22; }

/* Simulation HUD Elements */
.glass-hud {
  background: rgba(255, 255, 255, 0.03);
  backdrop-filter: blur(8px);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 4px;
}

.hud-stat { border-left: 2px solid #2ecc71; padding-left: 8px; margin-top: 4px; }

.floating-data {
  position: absolute;
  display: flex;
  align-items: center;
  pointer-events: none;
  z-index: 100;
}
.data-tag-line { width: 40px; height: 1px; background: #3498db; transform: rotate(-45deg); transform-origin: left; }
.data-tag-box { background: rgba(0,0,0,0.8); border: 1px solid #3498db; padding: 4px 8px; }

/* Simulation Area */
.sim-container {
  height: 100%;
  position: relative;
  background: radial-gradient(circle at center, #151a15 0%, #000 100%);
  overflow: hidden;
}

.lab-floor-grid {
  position: absolute; bottom: 0; width: 100%; height: 120px;
  background-image: 
    linear-gradient(rgba(46, 204, 113, 0.1) 1px, transparent 1px),
    linear-gradient(90deg, rgba(46, 204, 113, 0.1) 1px, transparent 1px);
  background-size: 30px 30px;
  transform: perspective(100px) rotateX(45deg);
  transform-origin: center bottom;
}

.scanner-beam {
  position: absolute; top: 0; left: 0; width: 100%; height: 4px;
  background: rgba(46, 204, 113, 0.4);
  box-shadow: 0 0 15px #2ecc71;
  animation: scan-loop 3s ease-in-out infinite;
  z-index: 15;
}
@keyframes scan-loop { 0%, 100% { top: 20%; opacity: 0; } 50% { top: 80%; opacity: 1; } }

.twin-sprite { width: 160px; image-rendering: pixelated; z-index: 5; transition: all 1s ease; }
.hologram-ring {
  position: absolute; bottom: -30px; width: 140px; height: 50px;
  background: radial-gradient(ellipse at center, rgba(46, 204, 113, 0.2) 0%, transparent 80%);
  border: 1px solid rgba(46, 204, 113, 0.4);
  border-radius: 50%;
  transform: rotateX(70deg);
  animation: rotate-ring 6s linear infinite;
}

@keyframes rotate-ring { 0% { transform: rotateX(70deg) rotateZ(0deg); } 100% { transform: rotateX(70deg) rotateZ(360deg); } }

/* ESP32 Hardware Styles */
.esp32-visualizer {
  width: 200px; height: 260px;
  background: #1e272e; border: 4px solid #3d464d;
  padding: 10px; position: relative;
}

.mcu-body { height: 100%; display: flex; justify-content: space-between; position: relative; }

.mcu-btn { position: absolute; width: 12px; height: 12px; background: #57606f; border: 1px solid #7f8c8d; border-radius: 2px; }
.boot-btn { bottom: 5px; left: 20px; }
.rst-btn { bottom: 5px; right: 20px; }

.pins-left, .pins-right { display: flex; flex-direction: column; justify-content: space-between; height: 90%; }
.pin { width: 12px; height: 4px; background: #485460; }
.pin-active { background: #2ecc71; box-shadow: 0 0 8px #2ecc71; }
.pin-pwr { background: #e74c3c; }
.pin-gnd { background: #34495e; }

.chip-center {
  flex-grow: 1; margin: 20px 15px;
  background: #2d3436; border: 1px solid #485460;
  color: white; position: relative;
}

.onboard-led { width: 6px; height: 6px; background: #3d464d; position: absolute; top: 10px; right: 10px; }
.onboard-led.blink { background: #3498db; box-shadow: 0 0 5px #3498db; animation: led-blink 0.5s infinite; }
@keyframes led-blink { 0%, 100% { opacity: 1; } 50% { opacity: 0; } }

/* Console Box */
.console-box {
  background: #000; height: 180px; overflow: hidden;
  font-family: 'Courier New', monospace; font-size: 10px; border: 1px solid #222;
}
.console-line { margin-bottom: 4px; border-left: 2px solid #2ecc71; padding-left: 8px; line-height: 1.4; }
.cursor-blink { display: inline-block; width: 8px; height: 12px; background: #2ecc71; animation: blink 1s step-end infinite; }
@keyframes blink { 0%, 100% { opacity: 1; } 50% { opacity: 0; } }

/* Weather Effects */
.raining::before {
  content: ""; position: absolute; top: 0; left: 0; width: 100%; height: 100%;
  background: repeating-linear-gradient(0deg, transparent, transparent 30px, rgba(52, 152, 219, 0.1) 31px);
  animation: rain-drop 0.3s linear infinite; z-index: 10;
}
@keyframes rain-drop { from { background-position: 0 0; } to { background-position: 0 60px; } }

.heat-wave::after {
  content: ""; position: absolute; top: 0; left: 0; width: 100%; height: 100%;
  background: rgba(255, 100, 0, 0.05); animation: heat-shimmer 1s ease-in-out infinite; z-index: 10;
}
@keyframes heat-shimmer { 0%, 100% { opacity: 0.1; } 50% { opacity: 0.3; } }
</style>