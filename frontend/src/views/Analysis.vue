<template>
  

  <v-container fluid class="plant-tama-bg px-6 pb-6 with-navbar">
    <!-- Header/Date Row -->
    <v-row class="mb-4">
      <v-col cols="12">
        <v-card flat class="pixel-card pa-4 bg-card relative">
          <div class="d-flex justify-space-between align-center mb-4">
            <div class="text-h5 pixel-font brand-text">PLANT-TAMA STATION</div>
            <!-- Plant Sprite Sprite -->
            <div class="sprite-container">
              <img src="C:\Users\dello\Desktop\ELET2415\Weatherstation\frontend\src\assets\flower.png" alt="Plant Sprite" class="plant-sprite" />
            </div>
          </div>
          <v-row align="center">
            <v-col cols="12" md="3">
              <v-text-field label="START DATE" type="date" density="comfortable" variant="outlined" class="pixel-input" v-model="start" hide-details></v-text-field>
            </v-col>
            <v-col cols="12" md="3">
              <v-text-field label="END DATE" type="date" density="comfortable" variant="outlined" class="pixel-input" v-model="end" hide-details></v-text-field>
            </v-col>
            <v-col cols="12" md="2">
              <v-btn block height="48" class="pixel-btn pixel-font-sm" @click="performAnalysis">REFRESH</v-btn>
            </v-col>
          </v-row>
        </v-card>
      </v-col>
    </v-row>

    <!-- Top Metrics & 3D Tank Visualization -->
    <v-row>
      <v-col cols="12" md="4">
        <v-row>
          <!-- Temperature Card -->
          <v-col cols="6" md="12">
            <v-card class="pixel-card pa-4 mb-4 metric-temp">
              <div class="d-flex justify-space-between align-start">
                <div class="pixel-label">TEMPERATURE</div>
                <select v-model="tempUnit" class="pixel-dropdown">
                  <option value="°C">°C</option>
                  <option value="°F">°F</option>
                  <option value="K">K</option>
                </select>
              </div>
              <div class="d-flex align-end">
                <span class="pixel-value">{{ displayTemp }}</span>
                <span class="pixel-unit ml-1 mb-2">{{ tempUnit }}</span>
              </div>
            </v-card>
          </v-col>

          <!-- Humidity Card -->
          <v-col cols="6" md="12">
            <v-card class="pixel-card pa-4 mb-4 metric-humid">
              <div class="d-flex justify-space-between align-start">
                <div class="pixel-label">HUMIDITY</div>
                <select v-model="humidUnit" class="pixel-dropdown">
                  <option value="%">%</option>
                  <option value="dec">0.x</option>
                </select>
              </div>
              <div class="d-flex align-end">
                <span class="pixel-value">{{ displayHumid }}</span>
                <span class="pixel-unit ml-1 mb-2">{{ humidUnit === 'dec' ? '' : humidUnit }}</span>
              </div>
            </v-card>
          </v-col>

          <!-- Pressure Card -->
          <v-col cols="6" md="12">
            <v-card class="pixel-card pa-4 mb-4 metric-pressure">
              <div class="d-flex justify-space-between align-start">
                <div class="pixel-label">PRESSURE</div>
                <select v-model="pressureUnit" class="pixel-dropdown">
                  <option value="hPa">hPa</option>
                  <option value="inHg">inHg</option>
                  <option value="psi">psi</option>
                </select>
              </div>
              <div class="d-flex align-end">
                <span class="pixel-value-sm">{{ displayPressure }}</span>
                <span class="pixel-unit-sm ml-1 mb-1">{{ pressureUnit }}</span>
              </div>
            </v-card>
          </v-col>

          <!-- Altitude Card -->
          <v-col cols="6" md="12">
            <v-card class="pixel-card pa-4 mb-4 metric-altitude">
              <div class="d-flex justify-space-between align-start">
                <div class="pixel-label">ALTITUDE</div>
                <select v-model="altitudeUnit" class="pixel-dropdown">
                  <option value="m">m</option>
                  <option value="ft">ft</option>
                </select>
              </div>
              <div class="d-flex align-end">
                <span class="pixel-value-sm">{{ displayAltitude }}</span>
                <span class="pixel-unit-sm ml-1 mb-1">{{ altitudeUnit }}</span>
              </div>
            </v-card>
          </v-col>

          <!-- Heat Index Card -->
          <v-col cols="6" md="12">
            <v-card class="pixel-card pa-4 metric-heat">
              <div class="d-flex justify-space-between align-start">
                <div class="pixel-label">HEAT INDEX</div>
                <select v-model="heatUnit" class="pixel-dropdown">
                  <option value="°C">°C</option>
                  <option value="°F">°F</option>
                  <option value="K">K</option>
                </select>
              </div>
              <div class="d-flex align-end">
                <span class="pixel-value-sm">{{ displayHeatIndex }}</span>
                <span class="pixel-unit-sm ml-1 mb-1">{{ heatUnit }}</span>
              </div>
            </v-card>
          </v-col>
        </v-row>
      </v-col>

      <!-- 3D Tank -->
      <v-col cols="12" md="8">
        <v-card class="pa-0 overflow-hidden tank-container soil-texture pixel-card">
          <div ref="threeJsContainer" class="three-layer"></div>
          
          <div class="water-overlay" :style="{ height: soilMoisture + '%' }">
            <div class="wave"></div>
            <div class="wave"></div>
            <div class="bubbles">
              <div v-for="n in 15" :key="n" class="bubble" :style="bubbleStyle()"></div>
            </div>
          </div>
          <div class="glass-reflection"></div>
          
          <div class="tank-info pa-6">
            <div class="pixel-font-sm text-white mb-1">SOIL MOISTURE</div>
            <div class="mono-font text-blue-lighten-4">Volumetric Water Content</div>
            <div class="pixel-font-xs text-blue-lighten-4 mt-2">LIVE MQTT READOUT</div>
            <div class="percentage-display">
              <span class="val pixel-font">{{ soilMoisture }}</span>
              <span class="unit pixel-font-sm">%</span>
            </div>
          </div>
          <div class="gauge-scale">
            <div v-for="mark in [100, 75, 50, 25, 0]" :key="mark" class="scale-mark">
              <span class="mark-label mono-font">{{ mark }}%</span>
              <div class="mark-line"></div>
            </div>
          </div>
        </v-card>
      </v-col>
    </v-row>

    <!-- Multi-Chart Dashboard -->
    <v-row class="mt-4">
      <v-col cols="12" md="6">
        <v-card class="pixel-card pa-4 h-100 bg-card">
          <div class="pixel-font-sm mb-4">SOIL & HEAT TRENDS</div>
          <div id="soilHeatChart" class="chart-box"></div>
        </v-card>
      </v-col>
      <v-col cols="12" md="6">
        <v-card class="pixel-card pa-4 h-100 bg-card">
          <div class="pixel-font-sm mb-4">ENV CORRELATION</div>
          <div id="tempHumidChart" class="chart-box"></div>
        </v-card>
      </v-col>
      <v-col cols="12">
        <v-card class="pixel-card pa-4 bg-card">
          <div class="pixel-font-sm mb-4">ATMOSPHERIC PRESSURE</div>
          <div id="pressureTimelineChart" class="chart-box-wide"></div>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup>
import { ref, computed, watch, onMounted, onBeforeUnmount } from "vue";
import { useMqttStore } from '@/store/mqttStore';
import { useAppStore } from "@/store/appStore";
import NavBar from '@/components/NavBar.vue';
import * as THREE from 'three';
import Highcharts from 'highcharts';

const Mqtt = useMqttStore();
const AppStore = useAppStore();
const LIVE_TOPICS = ['620169500', '620169500_pub', '620169500_sub'];

// Units
const tempUnit = ref("°C");
const humidUnit = ref("%");
const pressureUnit = ref("hPa");
const heatUnit = ref("°C");
const altitudeUnit = ref("m");

// Data
const start = ref("");
const end = ref("");
const soilMoisture = ref(0);
const temperature = ref(0);
const humidity = ref(0);
const pressure = ref(1013.25);
const heatIndex = ref(0);
const altitude = ref(0);
const readings = ref([]);

const clampPercent = (value) => Math.max(0, Math.min(100, value));

const resolveSoilPercent = (raw) => {
  const directPercent = Number(raw?.soil ?? raw?.soil_moisture ?? raw?.soilPercent ?? raw?.soil_percent);
  if (Number.isFinite(directPercent)) {
    return clampPercent(directPercent);
  }

  const adcValue = Number(raw?.soil_adc ?? raw?.adc ?? NaN);
  if (Number.isFinite(adcValue)) {
    return clampPercent(((4095 - adcValue) / 4095) * 100);
  }

  return 0;
};

const calculateAltitudeM = (pressureHpa, seaLevelHpa = 1013.25) => {
  const pressureValue = Number(pressureHpa);
  const seaLevelValue = Number(seaLevelHpa);

  if (!pressureValue || !Number.isFinite(pressureValue) || !Number.isFinite(seaLevelValue) || seaLevelValue <= 0) {
    return 0;
  }

  return 44330 * (1 - Math.pow(pressureValue / seaLevelValue, 1 / 5.255));
};

const normalizeReading = (raw) => {
  if (!raw || typeof raw !== 'object') return null;

  const reading = {
    timestamp: Number(raw.timestamp ?? Math.floor(Date.now() / 1000)),
    temperature: Number(raw.temperature ?? 0),
    humidity: Number(raw.humidity ?? 0),
    pressure: Number(raw.pressure ?? 1013.25),
    heatindex: Number(raw.heatindex ?? raw.heat_index ?? NaN),
    soilPercent: resolveSoilPercent(raw),
  };

  if (!Number.isFinite(reading.timestamp)) reading.timestamp = Math.floor(Date.now() / 1000);
  if (!Number.isFinite(reading.temperature)) reading.temperature = 0;
  if (!Number.isFinite(reading.humidity)) reading.humidity = 0;
  if (!Number.isFinite(reading.pressure)) reading.pressure = 1013.25;
  if (!Number.isFinite(reading.heatindex)) reading.heatindex = NaN;
  if (!Number.isFinite(reading.soilPercent)) reading.soilPercent = 0;

  return reading;
};

const syncDerivedMetrics = () => {
  altitude.value = calculateAltitudeM(pressure.value);
};

const applyLiveReading = (topic, payload) => {
  if (!topic || !LIVE_TOPICS.includes(topic)) return;

  const reading = normalizeReading(payload);
  if (!reading) return;

  temperature.value = reading.temperature;
  humidity.value = reading.humidity;
  pressure.value = reading.pressure;
  heatIndex.value = Number.isFinite(reading.heatindex)
    ? reading.heatindex
    : calculateHeatIndexC(reading.temperature, reading.humidity);
  soilMoisture.value = reading.soilPercent;
  syncDerivedMetrics();
};

const fetchLatest = async () => {
  const data = await AppStore.getLatest()
  if (data && Object.keys(data).length > 0) {
    const reading = normalizeReading(data);
    if (!reading) return;
    temperature.value = reading.temperature;
    humidity.value = reading.humidity;
    pressure.value = reading.pressure;
    heatIndex.value = Number.isFinite(reading.heatindex)
      ? reading.heatindex
      : calculateHeatIndexC(reading.temperature, reading.humidity);
    soilMoisture.value = reading.soilPercent;
    syncDerivedMetrics();
  }
}

const toUnixSeconds = (dateValue, endOfDay = false) => {
  if (!dateValue) return null;
  const date = endOfDay
    ? new Date(`${dateValue}T23:59:59`)
    : new Date(`${dateValue}T00:00:00`);
  if (Number.isNaN(date.getTime())) return null;
  return Math.floor(date.getTime() / 1000);
};

const fetchAnalysisData = async () => {
  const now = Math.floor(Date.now() / 1000);
  const startTs = toUnixSeconds(start.value) ?? 0;
  const endTs = toUnixSeconds(end.value, true) ?? now;

  const safeStart = Math.min(startTs, endTs);
  const safeEnd = Math.max(startTs, endTs);

  const data = await AppStore.getAllInRange(safeStart, safeEnd);
  readings.value = Array.isArray(data)
    ? [...data].sort((a, b) => (a.timestamp ?? 0) - (b.timestamp ?? 0))
    : [];

  if (readings.value.length > 0) {
    const latest = readings.value[readings.value.length - 1];
    temperature.value = Number(latest.temperature ?? 0);
    humidity.value = Number(latest.humidity ?? 0);
    pressure.value = Number(latest.pressure ?? 1013.25);
    heatIndex.value = Number.isFinite(Number(latest.heatindex ?? latest.heat_index))
      ? Number(latest.heatindex ?? latest.heat_index)
      : calculateHeatIndexC(Number(latest.temperature ?? 0), Number(latest.humidity ?? 0));
    soilMoisture.value = resolveSoilPercent(latest);
    syncDerivedMetrics();
    return;
  }

  await fetchLatest();
};

// Converters
const convertTemp = (val, unit) => {
  if (unit === '°F') return (val * 1.8 + 32);
  if (unit === 'K') return (val + 273.15);
  return val;
};

const displayTemp = computed(() => convertTemp(temperature.value, tempUnit.value).toFixed(1));
const displayHumid = computed(() => humidUnit.value === 'dec' ? (humidity.value / 100).toFixed(2) : humidity.value);
const displayPressure = computed(() => {
  if (pressureUnit.value === 'inHg') return (pressure.value * 0.02953).toFixed(2);
  if (pressureUnit.value === 'psi') return (pressure.value * 0.0145038).toFixed(2);
  return pressure.value.toFixed(1);
});

const displayAltitude = computed(() => {
  if (altitudeUnit.value === 'ft') {
    return (altitude.value * 3.28084).toFixed(1);
  }
  return altitude.value.toFixed(1);
});

const calculateHeatIndexC = (tempC, rh) => {
  const tempF = tempC * 1.8 + 32;
  const humidityPct = Math.max(0, Math.min(100, Number(rh ?? 0)));

  if (tempF < 80 || humidityPct < 40) {
    return tempC;
  }

  let hiF = -42.379 + (2.04901523 * tempF) + (10.14333127 * humidityPct)
    - (0.22475541 * tempF * humidityPct)
    - (0.00683783 * tempF * tempF)
    - (0.05481717 * humidityPct * humidityPct)
    + (0.00122874 * tempF * tempF * humidityPct)
    + (0.00085282 * tempF * humidityPct * humidityPct)
    - (0.00000199 * tempF * tempF * humidityPct * humidityPct);

  if (humidityPct < 13 && tempF >= 80 && tempF <= 112) {
    hiF -= ((13 - humidityPct) / 4) * Math.sqrt((17 - Math.abs(tempF - 95)) / 17);
  } else if (humidityPct > 85 && tempF >= 80 && tempF <= 87) {
    hiF += ((humidityPct - 85) / 10) * ((87 - tempF) / 5);
  }

  return (hiF - 32) / 1.8;
};

const displayHeatIndex = computed(() => {
  return convertTemp(heatIndex.value, heatUnit.value).toFixed(1);
});

// 3D Visuals
const threeJsContainer = ref(null);
let scene, camera, renderer, animFrameId;

const bubbleStyle = () => {
  const size = Math.random() * 8 + 4;
  return {
    left: Math.random() * 100 + '%',
    width: size + 'px',
    height: size + 'px',
    animationDelay: Math.random() * 5 + 's',
    animationDuration: (Math.random() * 3 + 4) + 's',
    opacity: Math.random() * 0.5 + 0.2
  };
};

const buildBranch = (parent, origin, dir, length, thickness, depth) => {
  if (depth <= 0 || length < 0.05) return;
  const geom = new THREE.CylinderGeometry(thickness * 0.7, thickness, length, 12);
  const mat = new THREE.MeshStandardMaterial({ color: 0x8d6e63, roughness: 0.9, metalness: 0.1 });
  const mesh = new THREE.Mesh(geom, mat);
  const axis = new THREE.Vector3(0, 1, 0);
  const q = new THREE.Quaternion().setFromUnitVectors(axis, dir.clone().normalize());
  mesh.quaternion.copy(q);
  mesh.position.copy(origin).addScaledVector(dir.clone().normalize(), length / 2);
  parent.add(mesh);
  const tip = origin.clone().addScaledVector(dir.clone().normalize(), length);
  const count = depth > 3 ? 3 : 2;
  for (let i = 0; i < count; i++) {
    const nd = dir.clone().normalize();
    nd.x += (Math.random() - 0.5) * 1.4;
    nd.z += (Math.random() - 0.5) * 1.4;
    nd.y -= 0.4;
    buildBranch(parent, tip, nd.normalize(), length * 0.72, thickness * 0.62, depth - 1);
  }
};

const initThreeJS = () => {
  const container = threeJsContainer.value;
  if (!container) return;
  scene = new THREE.Scene();
  camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 100);
  camera.position.set(0, 0, 14);
  renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
  renderer.setSize(container.clientWidth, container.clientHeight);
  container.appendChild(renderer.domElement);
  scene.add(new THREE.AmbientLight(0xffffff, 0.8));
  const dirLight = new THREE.DirectionalLight(0xffffff, 1.0);
  dirLight.position.set(5, 5, 5);
  scene.add(dirLight);
  const roots = new THREE.Group();
  buildBranch(roots, new THREE.Vector3(0, 6, 0), new THREE.Vector3(0, -1, 0), 2.5, 0.3, 6);
  scene.add(roots);
  const animate = () => {
    animFrameId = requestAnimationFrame(animate);
    roots.rotation.y += 0.002;
    renderer.render(scene, camera);
  };
  animate();
};

const initCharts = () => {
  const soilSeries = readings.value.map((r) => [
    Number(r.timestamp ?? 0) * 1000,
    resolveSoilPercent(r)
  ]);
  const heatSeries = readings.value.map((r) => {
    const t = Number(r.temperature ?? 0);
    const h = Number(r.humidity ?? 0);
    const hiC = Number.isFinite(Number(r.heatindex ?? r.heat_index))
      ? Number(r.heatindex ?? r.heat_index)
      : calculateHeatIndexC(t, h);
    return [Number(r.timestamp ?? 0) * 1000, convertTemp(hiC, heatUnit.value)];
  });
  const tempSeries = readings.value.map((r) => [
    Number(r.timestamp ?? 0) * 1000,
    convertTemp(Number(r.temperature ?? 0), tempUnit.value)
  ]);
  const humidSeries = readings.value.map((r) => [
    Number(r.timestamp ?? 0) * 1000,
    Number(r.humidity ?? 0)
  ]);
  const pressureSeries = readings.value.map((r) => {
    const p = Number(r.pressure ?? 1013.25);
    if (pressureUnit.value === 'inHg') return [Number(r.timestamp ?? 0) * 1000, p * 0.02953];
    if (pressureUnit.value === 'psi') return [Number(r.timestamp ?? 0) * 1000, p * 0.0145038];
    return [Number(r.timestamp ?? 0) * 1000, p];
  });

  Highcharts.chart('soilHeatChart', {
    chart: { type: 'line', backgroundColor: 'transparent', style: { fontFamily: 'Courier New' } },
    title: { text: null },
    xAxis: { type: 'datetime' },
    yAxis: [{ title: { text: 'MOISTURE (%)' } }, { title: { text: `HEAT (${heatUnit.value})` }, opposite: true }],
    credits: { enabled: false },
    series: [
      { name: 'Soil Moisture', data: soilSeries, color: '#5B9EF0' },
      { name: 'Heat Index', yAxis: 1, data: heatSeries, color: '#E0A020' }
    ]
  });

  Highcharts.chart('tempHumidChart', {
    chart: { type: 'areaspline', backgroundColor: 'transparent', style: { fontFamily: 'Courier New' } },
    title: { text: null },
    xAxis: { type: 'datetime' },
    yAxis: [{ title: { text: `TEMP (${tempUnit.value})` } }, { title: { text: 'HUMIDITY (%)' }, opposite: true }],
    credits: { enabled: false },
    series: [
      { name: 'Air Temp', data: tempSeries, color: '#E05C5C' },
      { name: 'Humidity', yAxis: 1, data: humidSeries, color: '#5B9EF0' }
    ]
  });

  Highcharts.chart('pressureTimelineChart', {
    chart: { type: 'spline', backgroundColor: 'transparent', style: { fontFamily: 'Courier New' } },
    title: { text: null },
    xAxis: { type: 'datetime' },
    yAxis: { title: { text: `PRESSURE (${pressureUnit.value})` } },
    credits: { enabled: false },
    series: [{ 
      name: 'Atmospheric Pressure', 
      data: pressureSeries, 
      color: '#9B6ECF' 
    }]
  });
};

watch([tempUnit, pressureUnit, humidUnit, heatUnit], () => initCharts());
const performAnalysis = async () => {
  await fetchAnalysisData();
  initCharts();
};

onMounted(() => {
  initThreeJS();
  Mqtt.connect();
  LIVE_TOPICS.forEach((topic) => Mqtt.subscribe(topic));
  performAnalysis();
  syncDerivedMetrics();
});

onBeforeUnmount(() => {
  Mqtt.unsubcribeAll();
  cancelAnimationFrame(animFrameId);
  renderer?.dispose();
});

watch(
  () => [Mqtt.payloadTopic, Mqtt.payload],
  ([topic, payload]) => {
    applyLiveReading(topic, payload);
  },
  { deep: true }
);
</script>

<style scoped>
.plant-tama-bg { background-color: #FFF3CF !important; min-height: 100vh; }
.with-navbar { padding-top: 70px !important; }
.bg-card { background-color: #FFF9EC !important; position: relative; }
.pixel-font { font-family: 'Press Start 2P', cursive; }
.pixel-font-sm { font-family: 'Press Start 2P', cursive; font-size: 0.7rem; }
.mono-font { font-family: 'Courier New', monospace; font-weight: bold; }
.brand-text { color: #3A2540; }

/* Sprite Styles */
.sprite-container {
  position: absolute;
  top: 10px;
  right: 20px;
}
.plant-sprite {
  width: 80px;
  height: 80px;
  object-fit: contain;
  image-rendering: pixelated;
  animation: float 3s ease-in-out infinite;
}
@keyframes float {
  0% { transform: translateY(0px); }
  50% { transform: translateY(-10px); }
  100% { transform: translateY(0px); }
}

.pixel-card {
  border: 3px solid #3A2540 !important;
  box-shadow: 4px 4px 0 rgba(0, 0, 0, 0.1) !important;
  border-radius: 0 !important;
}

.pixel-btn { background-color: #4CAF7D !important; color: white !important; border: 3px solid #3A2540 !important; border-radius: 0 !important; }

.pixel-dropdown {
  background: rgba(0, 0, 0, 0.2); border: 2px solid rgba(255, 255, 255, 0.4);
  color: white; font-family: 'Courier New', monospace; font-weight: bold; font-size: 0.75rem;
  padding: 1px 4px; cursor: pointer; border-radius: 4px; outline: none;
}
.pixel-dropdown option { background: #3A2540; color: white; }

.pixel-label { font-family: 'Press Start 2P', cursive; font-size: 0.6rem; color: rgba(255, 255, 255, 0.9); margin-bottom: 8px; }
.pixel-value { font-family: 'Press Start 2P', cursive; font-size: 2.2rem; color: white; }
.pixel-value-sm { font-family: 'Press Start 2P', cursive; font-size: 1.4rem; color: white; }
.pixel-unit { font-family: 'Courier New', monospace; font-size: 1.2rem; font-weight: 900; color: rgba(255, 255, 255, 0.8); }
.pixel-unit-sm { font-family: 'Courier New', monospace; font-size: 0.9rem; font-weight: 900; color: rgba(255, 255, 255, 0.8); }

.metric-temp { background-color: #E05C5C !important; }
.metric-humid { background-color: #5B9EF0 !important; }
.metric-pressure { background-color: #9B6ECF !important; }
.metric-altitude { background-color: #6C8CD5 !important; }
.metric-heat { background-color: #E0A020 !important; }

.tank-container { height: 550px; background: #2b1d16; position: relative; border: 3px solid #3A2540 !important; }
.three-layer { position: absolute; inset: 0; z-index: 1; }
.soil-texture {
  background-image: radial-gradient(circle at 50% 50%, rgba(62, 39, 35, 0.2) 0%, transparent 100%),
    url('data:image/svg+xml;utf8,<svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg"><filter id="n"><feTurbulence type="fractalNoise" baseFrequency="0.6" numOctaves="3" stitchTiles="stitch"/></filter><rect width="100%" height="100%" filter="url(%23n)" opacity="0.1"/></svg>');
}
.water-overlay { position: absolute; bottom: 0; left: 0; width: 100%; background: linear-gradient(180deg, rgba(33, 150, 243, 0.2) 0%, rgba(13, 71, 161, 0.5) 100%); z-index: 2; transition: height 1s ease-in-out; }
.wave {
  position: absolute; top: -30px; left: 0; width: 200%; height: 60px;
  background: url('data:image/svg+xml;utf8,<svg viewBox="0 0 1000 100" xmlns="http://www.w3.org/2000/svg"><path d="M0,50 C150,100 350,0 500,50 C650,100 850,0 1000,50 L1000,100 L0,100 Z" fill="rgba(33, 150, 243, 0.3)"/></svg>');
  background-size: 1000px 60px; animation: move-wave 6s linear infinite;
}
@keyframes move-wave { 0% { transform: translateX(0); } 100% { transform: translateX(-50%); } }

.bubbles { position: absolute; width: 100%; height: 100%; overflow: hidden; }
.bubble { background: rgba(255, 255, 255, 0.2); border-radius: 50%; bottom: -20px; position: absolute; animation: rise linear infinite; }
@keyframes rise { 0% { transform: translateY(0) scale(1); opacity: 0.4; } 100% { transform: translateY(-500px) scale(1.5); opacity: 0; } }

.glass-reflection { position: absolute; inset: 0; background: linear-gradient(115deg, rgba(255,255,255,0.02) 0%, rgba(255,255,255,0.1) 50%, rgba(255,255,255,0.02) 100%); z-index: 3; pointer-events: none; }
.tank-info { position: absolute; top: 0; left: 0; z-index: 4; }
.percentage-display { margin-top: 30px; color: white; text-shadow: 4px 4px 0 rgba(0, 0, 0, 0.8); }
.percentage-display .val { font-size: 4.5rem; line-height: 1; }
.percentage-display .unit { font-size: 1.2rem; margin-left: 5px; opacity: 0.8; }

.gauge-scale { position: absolute; right: 20px; top: 50px; bottom: 50px; display: flex; flex-direction: column; justify-content: space-between; z-index: 5; color: white; text-shadow: 2px 2px 0px #3A2540; }
.scale-mark { display: flex; align-items: center; gap: 10px; }
.mark-label { font-size: 12px; font-weight: bold; }
.mark-line { width: 15px; height: 2px; background: white; border: 1px solid #3A2540; }

.slider-control { position: absolute; right: 60px; top: 50px; bottom: 50px; z-index: 6; }
.vertical-slider { appearance: slider-vertical; height: 100%; width: 10px; cursor: pointer; }
.chart-box { height: 300px; }
.chart-box-wide { height: 350px; }


.plant-sprite { width: 100px; height: 100px;}
</style>