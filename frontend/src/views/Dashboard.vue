<template>
  <section class="dashboard-screen with-navbar">
    <div class="pixel-overlay"></div>

    <div class="dash-layout">
      <!-- Main -->
      <main class="dash-main">
        <!-- Top bar -->
        <div class="topbar">
          <h1 class="dash-title">Dashboard</h1>
          <div class="topbar-right">
            <div class="live-pill">
              <span class="live-dot"></span>
              Live · ESP32 connected
            </div>
            <div class="topbar-time">{{ currentTime }}</div>
          </div>
        </div>

        <div class="dash-content">
          <!-- Pet card -->
          <div class="pixel-card pet-card">
            <div class="pet-container" @click="handlePetClick">
              <!-- Using the flower sprite from your project -->
              <img 
                :class="['pet-sprite', { 'is-bouncing': isHappy }]" 
                src="C:\Users\dello\Desktop\ELET2415\Weatherstation\frontend\src\assets\flower.png" 
                alt="Plant sprite" 
              />
              <transition name="pop">
                <div v-if="speechBubble" class="speech-bubble">{{ speechBubble }}</div>
              </transition>
              <div class="click-hint"></div>
            </div>
            
            <div class="pet-info">
              <div class="pet-header">
                <div>
                  <div class="pet-name">{{ petName }}</div>
                  <div class="pet-mood">{{ moodText }}</div>
                </div>
              </div>
              
              <div class="stat-row">
                <span class="stat-label">HP</span>
                <div class="stat-bar-wrap"><div class="stat-bar bar-hp" :style="{ width: stats.hp + '%' }"></div></div>
                <span class="stat-val">{{ stats.hp }}%</span>
              </div>
              <div class="stat-row">
                <span class="stat-label">LOVE</span>
                <div class="stat-bar-wrap"><div class="stat-bar bar-love" :style="{ width: stats.love + '%' }"></div></div>
                <span class="stat-val">{{ stats.love }}%</span>
              </div>
              <div class="stat-row">
                <span class="stat-label">XP</span>
                <div class="stat-bar-wrap"><div class="stat-bar bar-xp" :style="{ width: (stats.xp % 100) + '%' }"></div></div>
                <span class="stat-val">{{ stats.xp }}</span>
              </div>
              <div class="badge-row">
                <span class="badge badge-streak">🔥 3 day streak</span>
                <span class="badge badge-lvl">Lvl {{ Math.floor(stats.xp / 100) }} Botanist</span>
                <span v-if="sensors[2].statusClass === 'st-yellow'" class="badge badge-warn">⚠ Soil drying</span>
              </div>
            </div>
          </div>

          <!-- Sensor cards -->
          <div class="sensor-grid">
            <div class="pixel-card sensor-card interactive-sensor" v-for="s in sensors" :key="s.label">
              <div class="sensor-label">{{ s.label }}</div>
              <div :class="['sensor-value', s.color]">{{ s.value }}</div>
              <div class="sensor-sub">{{ s.sub }}</div>
              <div class="sensor-bar-wrap">
                <div class="sensor-bar" :style="{ width: s.bar + '%', background: s.barColor }"></div>
              </div>
              <span :class="['sensor-status', s.statusClass]">{{ s.status }}</span>
            </div>
          </div>

          <!-- Reading history with Filters -->
          <div class="pixel-card history-card">
            <div class="card-header history-header">
              <div class="header-left">
                <span class="card-title">Reading history</span>
                <span class="card-meta">Database: MongoDB Atlas</span>
              </div>
              <div class="filter-controls">
                <select v-model="filterRange" class="pixel-select">
                  <option value="5">LAST 5</option>
                  <option value="today">TODAY</option>
                  <option value="all">ALL TIME</option>
                </select>
                <input 
                  v-model="searchQuery" 
                  type="text" 
                  placeholder="SEARCH READINGS..." 
                  class="pixel-input"
                />
              </div>
            </div>
            <div class="table-wrap">
              <table class="history-table">
                <thead>
                  <tr>
                    <th>TIME</th><th>TEMP</th><th>HUMIDITY</th><th>SOIL ADC</th><th>PRESSURE</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="row in filteredHistory" :key="row.time" class="history-row-hover">
                    <td class="col-time">{{ row.time }}</td>
                    <td class="col-temp">{{ row.temp }}</td>
                    <td class="col-hum">{{ row.hum }}</td>
                    <td class="col-soil">{{ row.soil }}</td>
                    <td class="col-pres">{{ row.pres }}</td>
                  </tr>
                  <tr v-if="filteredHistory.length === 0">
                    <td colspan="5" class="no-data">NO MATCHING ENTRIES FOUND</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
      </main>
    </div>
  </section>
</template>

<script setup>
import { ref, reactive, computed, onMounted, onUnmounted, watch } from 'vue'
import { useRoute } from 'vue-router'
import { useAppStore } from '@/store/appStore'
import { useMqttStore } from '@/store/mqttStore'

const route = useRoute()
const AppStore = useAppStore()
const MqttStore = useMqttStore()
const LIVE_TOPICS = ['620169500', '620169500_pub', '620169500_sub']

const savedName = localStorage.getItem('plantName')
const petName = ref(route.query.name || savedName || 'Sprout')
const currentTime = ref('')
const isHappy = ref(false)
const speechBubble = ref('')
const moodText = ref('Happy · All conditions healthy')
const searchQuery = ref('')
const filterRange = ref('5')

const stats = reactive({
  hp: 78,
  love: 62,
  xp: 450
})

const handlePetClick = () => {
  isHappy.value = true
  stats.love = Math.min(100, stats.love + 2)
  stats.xp += 1
  
  const phrases = ['Lets go!', 'Yay!', '<3', 'Howdie', 'UWU', 'Soft!']
  speechBubble.value = phrases[Math.floor(Math.random() * phrases.length)]
  
  setTimeout(() => { isHappy.value = false }, 500)
  setTimeout(() => { speechBubble.value = '' }, 1500)
}

let timer
let dataTimer

const getSensorStatus = (label, value) => {
  if (label === 'TEMPERATURE') {
    if (value < 15 || value > 35) return { status: 'Warning', statusClass: 'st-yellow' }
    return { status: 'Healthy', statusClass: 'st-green' }
  }
  if (label === 'HUMIDITY') {
    if (value < 30 || value > 70) return { status: 'Warning', statusClass: 'st-yellow' }
    return { status: 'Optimal', statusClass: 'st-green' }
  }
  if (label === 'SOIL MOISTURE') {
    if (value > 3000) return { status: 'Dry', statusClass: 'st-red' }
    if (value > 2500) return { status: 'Drying soon', statusClass: 'st-yellow' }
    return { status: 'Wet', statusClass: 'st-green' }
  }
  if (label === 'PRESSURE') {
    if (value < 1000 || value > 1020) return { status: 'Variable', statusClass: 'st-yellow' }
    return { status: 'Stable', statusClass: 'st-green' }
  }
  return { status: 'Unknown', statusClass: 'st-green' }
}

const sensors = computed(() => {
  const temp = latestData.value?.temperature ?? 0
  const hum = latestData.value?.humidity ?? 0
  const soil = latestData.value?.soil_adc ?? 0
  const pres = latestData.value?.pressure ?? 1013
  const heat = latestData.value?.heatindex ?? 0

  return [
    { 
      label: 'TEMPERATURE', 
      value: `${temp.toFixed(1)}°C`, 
      sub: `Heat index: ${heat.toFixed(1)}°C`, 
      color: 'val-red',    
      bar: Math.min(100, Math.max(0, temp)), 
      barColor: '#e05c5c',
      ...getSensorStatus('TEMPERATURE', temp)
    },
    { 
      label: 'HUMIDITY',    
      value: `${hum.toFixed(1)}%`,  
      sub: 'Ideal: 40–70%',      
      color: 'val-blue',   
      bar: Math.min(100, Math.max(0, hum)), 
      barColor: '#5b9ef0',
      ...getSensorStatus('HUMIDITY', hum)
    },
    { 
      label: 'SOIL MOISTURE', 
      value: soil > 2500 ? 'DRY' : soil > 1500 ? 'MOIST' : 'WET',
      sub: `ADC: ${soil}`,          
      color: 'val-yellow', 
      bar: Math.min(100, Math.max(0, (4095 - soil) / 40.95)), 
      barColor: '#d4a017',
      ...getSensorStatus('SOIL MOISTURE', soil)
    },
    { 
      label: 'PRESSURE',   
      value: `${pres.toFixed(0)}hPa`, 
      sub: 'Ideal: 1005–1015',   
      color: 'val-purple', 
      bar: Math.min(100, Math.max(0, (pres - 980) / 4)), 
      barColor: '#9b6ecf',
      ...getSensorStatus('PRESSURE', pres)
    },
  ]
})

const latestData = ref({})
const history = reactive([])

const normalizeReading = (raw) => {
  if (!raw || typeof raw !== 'object') return null

  const reading = {
    timestamp: Number(raw.timestamp ?? Math.floor(Date.now() / 1000)),
    temperature: Number(raw.temperature ?? 0),
    humidity: Number(raw.humidity ?? 0),
    heatindex: Number(raw.heatindex ?? 0),
    soil_adc: Number(raw.soil_adc ?? raw.soil ?? 0),
    pressure: Number(raw.pressure ?? 1013),
  }

  if (!Number.isFinite(reading.timestamp)) reading.timestamp = Math.floor(Date.now() / 1000)
  if (!Number.isFinite(reading.temperature)) reading.temperature = 0
  if (!Number.isFinite(reading.humidity)) reading.humidity = 0
  if (!Number.isFinite(reading.heatindex)) reading.heatindex = 0
  if (!Number.isFinite(reading.soil_adc)) reading.soil_adc = 0
  if (!Number.isFinite(reading.pressure)) reading.pressure = 1013

  return reading
}

const pushHistoryRow = (reading) => {
  history.unshift({
    time: new Date(reading.timestamp * 1000).toTimeString().slice(0, 8),
    temp: `${reading.temperature}°C`,
    hum: `${reading.humidity}%`,
    soil: reading.soil_adc?.toString() ?? 'N/A',
    pres: `${reading.pressure}hPa`
  })

  if (history.length > 10) {
    history.splice(10)
  }
}

const fetchLatest = async () => {
  const data = await AppStore.getLatest()
  if (data && Object.keys(data).length > 0) {
    const normalized = normalizeReading(data)
    if (normalized) {
      latestData.value = normalized
    }
  }
}

const fetchHistory = async () => {
  const now = Math.floor(Date.now() / 1000)
  // Fetch all available data by using year 2000 as start
  const allTimeStart = 946684800 // Jan 1, 2000
  const data = await AppStore.getAllInRange(allTimeStart, now)
  if (data && data.length > 0) {
    history.length = 0
    // Show the most recent 10 readings
    data.slice(-10).reverse().forEach(r => {
      const normalized = normalizeReading(r)
      if (!normalized) return
      history.push({
        time: new Date(normalized.timestamp * 1000).toTimeString().slice(0, 8),
        temp: `${normalized.temperature}°C`,
        hum: `${normalized.humidity}%`,
        soil: normalized.soil_adc?.toString() ?? 'N/A',
        pres: `${normalized.pressure}hPa`
      })
    })
  }
}

const applyLivePayload = (topic, payload) => {
  if (!topic || !LIVE_TOPICS.includes(topic)) return

  const normalized = normalizeReading(payload)
  if (!normalized) return

  latestData.value = normalized
  pushHistoryRow(normalized)
}

onMounted(() => {
  const tick = () => {
    const now = new Date()
    currentTime.value = now.toTimeString().slice(0, 8)
  }
  tick()
  timer = setInterval(tick, 1000)
  
  fetchLatest()
  fetchHistory()

  MqttStore.connect()
  LIVE_TOPICS.forEach((topic) => MqttStore.subscribe(topic))

  dataTimer = setInterval(() => {
    fetchLatest()
    fetchHistory()
  }, 30000)
})
onUnmounted(() => {
  clearInterval(timer)
  clearInterval(dataTimer)
  MqttStore.unsubcribeAll()
})

watch(
  () => [MqttStore.payloadTopic, MqttStore.payload],
  ([topic, payload]) => {
    applyLivePayload(topic, payload)
  },
  { deep: true }
)

const filteredHistory = computed(() => {
  let list = [...history];
  
  // Filter by Range
  if (filterRange.value === '5') {
    list = list.slice(0, 5);
  } else if (filterRange.value === 'today') {
    // In a real app, you'd filter by date string. Mocking for now.
    list = list.filter(item => item.time.startsWith('14') || item.time.startsWith('09'));
  }

  // Filter by Search Query
  if (searchQuery.value.trim() !== '') {
    const q = searchQuery.value.toLowerCase();
    list = list.filter(item => 
      item.temp.toLowerCase().includes(q) || 
      item.hum.toLowerCase().includes(q) || 
      item.soil.toLowerCase().includes(q) ||
      item.pres.toLowerCase().includes(q) ||
      item.time.toLowerCase().includes(q)
    );
  }
  
  return list;
});
</script>

<style scoped>
/* ── Base ── */
* { box-sizing: border-box; }

.with-navbar {
  padding-top: 64px;
}

.dashboard-screen {
  position: relative;
  min-height: 100vh;
  font-family: 'Press Start 2P', 'Courier New', monospace;
  background:
    radial-gradient(circle at 20% 10%, #ffd08a 0, transparent 45%),
    radial-gradient(circle at 90% 85%, #ff8cb8 0, transparent 35%),
    linear-gradient(180deg, #2f223f 0%, #1e1b3a 65%, #121228 100%);
  overflow-x: hidden;
}

.pixel-overlay {
  position: absolute;
  inset: 0;
  pointer-events: none;
  background-image: repeating-linear-gradient(
    0deg,
    rgba(255,255,255,0.06) 0,
    rgba(255,255,255,0.06) 1px,
    transparent 1px,
    transparent 3px
  );
  opacity: 0.5;
  z-index: 0;
}

.dash-layout { position: relative; z-index: 1; display: flex; min-height: 100vh; }
.dash-main { flex: 1; background: #fff3cf; display: flex; flex-direction: column; overflow-y: auto; }

/* ── Topbar ── */
.topbar {
  display: flex; align-items: center; justify-content: space-between;
  padding: 20px 24px; border-bottom: 3px solid #3a2540;
  box-shadow: 0 3px 0 #ffc86d; background: #fff9ec; flex-shrink: 0;
}
.dash-title { font-size: 18px; color: #2b1236; letter-spacing: 1px; }
.topbar-right { display: flex; align-items: center; gap: 16px; }

.live-pill {
  display: flex; align-items: center; gap: 8px;
  background: #1e1333; color: #4caf7d; font-size: 9px;
  padding: 8px 14px; border: 2px solid #4caf7d; box-shadow: 3px 3px 0 #2c1a33;
}

.live-dot {
  width: 8px; height: 8px; background: #4caf7d; border-radius: 50%;
  animation: blink 1.4s ease-in-out infinite;
}
@keyframes blink { 0%, 100% { opacity: 1; } 50% { opacity: 0.2; } }
.topbar-time { font-size: 12px; color: #2b1236; }

/* ── Content ── */
.dash-content { padding: 24px; display: flex; flex-direction: column; gap: 20px; }
.pixel-card { background: #fff9ec; border: 3px solid #3a2540; padding: 20px; box-shadow: 4px 4px 0 rgba(0,0,0,0.1); }

/* ── Pet Card ── */
.pet-card { display: flex; gap: 24px; align-items: center; }
.pet-container { position: relative; cursor: pointer; user-select: none; }
.pet-sprite { 
  width: 120px; height: 120px; 
  image-rendering: pixelated; 
  border: 3px solid #3a2540; 
  background: #fff;
  transition: transform 0.1s;
}
.is-bouncing { animation: petBounce 0.4s ease-out; }
@keyframes petBounce { 0%, 100% { transform: translateY(0); } 50% { transform: translateY(-10px); } }

.speech-bubble {
  position: absolute; top: -40px; left: 50%; transform: translateX(-50%);
  background: #1e1333; color: #fff3cf; font-size: 8px; padding: 6px 10px;
  border: 2px solid #3a2540; white-space: nowrap; z-index: 10;
}

.pet-info { flex: 1; }
.pet-name { font-size: 20px; color: #4caf7d; margin-bottom: 8px; }
.pet-mood { font-size: 10px; color: #503262; margin-bottom: 16px; }

/* ── Stats ── */
.stat-row { display: flex; align-items: center; gap: 12px; margin-bottom: 10px; }
.stat-label { font-size: 10px; color: #503262; width: 50px; flex-shrink: 0; }
.stat-bar-wrap { flex: 1; height: 12px; background: #e8d9b8; border: 2px solid #3a2540; }
.stat-bar { height: 100%; transition: width 0.4s; }
.bar-hp { background: #4caf7d; }
.bar-love { background: #e05c9c; }
.bar-xp { background: #e0a020; }
.stat-val { font-size: 10px; color: #2b1236; width: 40px; text-align: right; }

.badge-row { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 14px; }
.badge { font-size: 9px; padding: 6px 10px; border: 2px solid #3a2540; box-shadow: 3px 3px 0 #2c1a33; }
.badge-streak { background: #1e1333; color: #e0a020; }
.badge-lvl { background: #1e1333; color: #fff3cf; }
.badge-warn { background: #fff0cc; color: #ba5c00; border-color: #ba5c00; }

/* ── Sensor Grid ── */
.sensor-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 16px; }
@media (max-width: 1200px) { .sensor-grid { grid-template-columns: repeat(2, 1fr); } }

.sensor-card { display: flex; flex-direction: column; gap: 10px; }
.sensor-label { font-size: 9px; color: #7a6a5a; }
.sensor-value { font-size: 20px; font-weight: bold; }
.val-red { color: #c0392b; }
.val-blue { color: #2471a3; }
.val-yellow { color: #b7770d; }
.val-purple { color: #7d3c98; }

.sensor-bar-wrap { height: 8px; background: #e8d9b8; border: 2px solid #3a2540; }
.sensor-bar { height: 100%; }
.sensor-status { font-size: 9px; padding: 4px 8px; border: 2px solid currentColor; align-self: flex-start; }
.st-green { color: #0b7a4a; background: #d4f0e3; }
.st-yellow { color: #ba5c00; background: #fff0cc; }

/* ── Filter Controls ── */
.history-header { flex-direction: row; align-items: center; }
.filter-controls { display: flex; gap: 10px; }

.pixel-select, .pixel-input {
  font-family: inherit; font-size: 8px; padding: 8px;
  border: 2px solid #3a2540; background: #fff;
  box-shadow: 2px 2px 0 #3a2540; outline: none;
}
.pixel-input { width: 180px; }
.pixel-select:focus, .pixel-input:focus { border-color: #4caf7d; }

/* ── Table ── */
.history-table { width: 100%; border-collapse: collapse; font-size: 10px; }
.history-table th { color: #7a6a5a; text-align: left; padding: 12px 10px; border-bottom: 3px solid #3a2540; }
.history-table td { padding: 12px 10px; border-bottom: 1px solid #e8d9b8; color: #2b1236; }
.no-data { text-align: center; color: #ba5c00; padding: 40px; font-size: 8px; }

.col-temp { color: #c0392b; }
.col-hum { color: #2471a3; }
.col-soil { color: #b7770d; }
.col-pres { color: #7d3c98; }

.pop-enter-active { animation: pop-in 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275); }
@keyframes pop-in { 0% { transform: translateX(-50%) scale(0); opacity: 0; } 100% { transform: translateX(-50%) scale(1); opacity: 1; } }
</style>