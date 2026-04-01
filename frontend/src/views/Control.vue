<template>
  <section class="lab-screen">
    <div class="pixel-overlay"></div>

    <div class="lab-layout">
      <!-- Left Sidebar: System Diagnostics -->
      <aside class="lab-sidebar">
        <div class="pixel-card diag-card">
          <div class="card-label">AIR TEMP</div>
          <div class="diag-val">{{ fmt1(temperature) }}°C</div>
          <div class="diag-sub">{{ tempStatus.label }}</div>
          <div class="diag-bar"><div class="fill" :style="{ width: `${Math.min(100, Math.max(0, temperature * 2.4))}%` }"></div></div>
        </div>

        <div class="pixel-card diag-card">
          <div class="card-label">SOIL MOISTURE</div>
          <div class="diag-val">{{ fmt1(soilMoisture) }}%</div>
          <div class="diag-sub">{{ soilStatus.label }}</div>
          <div class="diag-bar"><div class="fill wifi" :style="{ width: `${soilMoisture}%` }"></div></div>
        </div>

        <div class="pixel-card diag-card">
          <div class="card-label">SYSTEM UPTIME</div>
          <div class="diag-val">{{ uptime }}</div>
          <div class="diag-sub">Packets: {{ packetCount }} | {{ packetRate }}/min | {{ lastSeenText }}</div>
        </div>

        <div class="pixel-card log-card">
          <div class="card-label">WIRING SCHEMATICS</div>
          <div class="schematic-list">
            <div v-for="pin in pinout" :key="pin.gpio" 
                 class="schematic-item" 
                 :class="{ active: activeSensor === pin.sensor }"
                 @mouseenter="activeSensor = pin.sensor">
              <span class="pin-id">GPIO {{ pin.gpio }}</span>
              <span class="pin-target">{{ pin.label }}</span>
            </div>
          </div>
        </div>
      </aside>

      <!-- Center: The Digital Twin -->
      <main class="lab-viewport">
        <div class="viewport-header">
          <div class="vp-title">PHYSICAL TWIN: ESP32-WROOM-32</div>
          <div class="vp-status" :class="{ 'is-online': isOnline }">
            {{ isOnline ? 'SYNCHRONIZED' : 'CONNECTION LOST' }}
          </div>
        </div>

        <div class="twin-container">
          <div class="esp32-model" :class="`focus-${activeSensor}`">
            <div class="pcb-base">
              <div class="mcu-chip">ESP32</div>
              <div class="antenna"></div>
              
              <!-- Physical Pins -->
              <div class="pin-header left">
                <div v-for="i in 15" :key="'l'+i" class="pin-dot" :class="{ highlight: isPinActive('left', i) }"></div>
              </div>
              <div class="pin-header right">
                <div v-for="i in 15" :key="'r'+i" class="pin-dot" :class="{ highlight: isPinActive('right', i) }"></div>
              </div>
              
              <!-- Connection Traces (SVG) -->
              <svg class="traces-layer" viewBox="0 0 200 280">
                <path v-if="activeSensor === 'temp'" d="M190 100 L110 100" class="trace-line" />
                <path v-if="activeSensor === 'soil'" d="M10 80 L60 80" class="trace-line" />
                <path v-if="activeSensor === 'hum'" d="M10 200 L60 200" class="trace-line" />
                <path v-if="activeSensor === 'pressure'" d="M190 220 L110 220" class="trace-line" />
              </svg>

              <!-- Data Overlays -->
              <div class="data-node node-temp" @mouseenter="activeSensor = 'temp'">
                <div class="node-line"></div>
                <div class="node-label">TEMP: {{ fmt1(temperature) }}°C</div>
              </div>
              <div class="data-node node-hum" @mouseenter="activeSensor = 'hum'">
                <div class="node-line"></div>
                <div class="node-label">HUM: {{ fmt1(humidity) }}%</div>
              </div>
              <div class="data-node node-soil" @mouseenter="activeSensor = 'soil'">
                <div class="node-line"></div>
                <div class="node-label">SOIL: {{ fmt1(soilMoisture) }}%</div>
              </div>
              <div class="data-node node-pres" @mouseenter="activeSensor = 'pressure'">
                <div class="node-line"></div>
                <div class="node-label">PRES: {{ fmt0(pressure) }} hPa</div>
              </div>
            </div>
          </div>
        </div>

        <div class="viewport-footer">
          <div class="tool-btn" @click="pingDevice">REBOOT</div>
          <div class="tool-btn" @click="clearLogs">CLEAR</div>
          <div class="tool-btn danger" @click="syncNow">SYNC</div>
        </div>
        <div class="cmd-status">
          CMD: {{ commandState.action || 'IDLE' }} | {{ commandState.status }}
          <span v-if="commandState.rttMs"> | RTT: {{ commandState.rttMs }}ms</span>
        </div>
      </main>

      <!-- Right Sidebar: Configuration -->
      <aside class="lab-config">
        <div class="pixel-card config-card">
          <div class="card-label">POLLING RATE</div>
          <input type="range" min="1" max="60" v-model="pollRate" class="pixel-slider">
          <div class="slider-val">{{ pollRate }} SECONDS</div>
        </div>

        <div class="pixel-card config-card">
          <div class="card-label">HARDWARE HEALTH</div>
          <div class="diag-sub">Link: <span :class="isOnline ? 'ok' : 'bad'">{{ isOnline ? 'ONLINE' : 'OFFLINE' }}</span></div>
          <div class="diag-sub">Temp: {{ tempStatus.label }}</div>
          <div class="diag-sub">Humidity: {{ humidityStatus.label }}</div>
          <div class="diag-sub">Soil: {{ soilStatus.label }}</div>
          <div class="diag-sub">Pressure: {{ pressureStatus.label }}</div>
        </div>

        <div class="pixel-card sensor-focus" v-if="activeSensor">
          <div class="card-label">SIGNAL: {{ activeSensor.toUpperCase() }}</div>
          <div class="focus-viz">
            <svg viewBox="0 0 200 60" class="mini-chart">
              <polyline fill="none" stroke="#4caf7d" stroke-width="2" :points="focusLinePoints" />
            </svg>
          </div>
          <div class="focus-meta">GPIO: {{ currentPinInfo.gpio }} | {{ activeSensorValue }}</div>
        </div>

        <div class="pixel-card log-card">
            <div class="card-label">LIVE LOGS</div>
            <div class="log-terminal">
              <div v-for="(log, i) in logs" :key="i" class="log-entry">
                <span class="log-ts">[{{ log.time }}]</span> {{ log.msg }}
              </div>
            </div>
          </div>
      </aside>
    </div>
  </section>
</template>

<script setup>
import { computed, ref, onMounted, onUnmounted, watch } from 'vue'
import { useMqttStore } from '@/store/mqttStore'
import { useAppStore } from '@/store/appStore'

const Mqtt = useMqttStore()
const AppStore = useAppStore()
const LIVE_TOPICS = ['620169500', '620169500_pub', '620169500_sub']

const isOnline = ref(false)
const uptime = ref('00:00:00')
const startedAt = ref(Date.now())
const temperature = ref(0)
const humidity = ref(0)
const pressure = ref(1013.25)
const soilMoisture = ref(0)
const heatIndex = ref(0)
const packetCount = ref(0)
const lastPacketAt = ref(0)
const pollRate = ref(5)
const activeSensor = ref('temp')
const logs = ref([])
const sensorSeries = ref([])
const commandState = ref({ action: '', status: 'idle', sentAt: 0, rttMs: 0 })

const toFinite = (value, fallback = 0) => {
  const n = Number(value)
  return Number.isFinite(n) ? n : fallback
}

const fmt1 = (value) => toFinite(value, 0).toFixed(1)
const fmt0 = (value) => toFinite(value, 0).toFixed(0)

const resolveSoilPercent = (raw) => {
  const direct = Number(raw?.soil ?? raw?.soil_moisture ?? raw?.soilPercent ?? raw?.soil_percent)
  if (Number.isFinite(direct)) return Math.max(0, Math.min(100, direct))

  const adc = Number(raw?.soil_adc ?? raw?.adc ?? NaN)
  if (Number.isFinite(adc)) return Math.max(0, Math.min(100, ((4095 - adc) / 4095) * 100))

  return 0
}

const normalizeReading = (raw) => {
  if (!raw || typeof raw !== 'object') return null

  return {
    timestamp: toFinite(raw.timestamp, Math.floor(Date.now() / 1000)),
    temperature: toFinite(raw.temperature, 0),
    humidity: toFinite(raw.humidity, 0),
    pressure: toFinite(raw.pressure, 1013.25),
    soil: resolveSoilPercent(raw),
  }
}

// Map your hardware pins here
const pinout = [
  { gpio: 32, label: 'Soil Moisture (ADC)', sensor: 'soil', side: 'left', index: 3 },
  { gpio: 22, label: 'SCL (BMP280)', sensor: 'pressure', side: 'right', index: 12 },
  { gpio: 21, label: 'SDA (BMP280)', sensor: 'temp', side: 'right', index: 4 },
  { gpio: 4,  label: 'DHT Sensor', sensor: 'hum', side: 'left', index: 10 },
]

const isPinActive = (side, index) => {
  const active = pinout.find(p => p.sensor === activeSensor.value)
  return active && active.side === side && active.index === index
}

const currentPinInfo = computed(() => pinout.find(p => p.sensor === activeSensor.value) || { gpio: 'N/A' })

const tempStatus = computed(() => {
  if (temperature.value < 18) return { label: 'LOW' }
  if (temperature.value > 32) return { label: 'HIGH' }
  return { label: 'OK' }
})

const humidityStatus = computed(() => {
  if (humidity.value < 35) return { label: 'LOW' }
  if (humidity.value > 75) return { label: 'HIGH' }
  return { label: 'OK' }
})

const soilStatus = computed(() => {
  if (soilMoisture.value < 35) return { label: 'DRY' }
  if (soilMoisture.value > 85) return { label: 'WET' }
  return { label: 'OPTIMAL' }
})

const pressureStatus = computed(() => {
  if (pressure.value < 1000) return { label: 'LOW' }
  if (pressure.value > 1025) return { label: 'HIGH' }
  return { label: 'NORMAL' }
})

const pushLog = (msg) => {
  logs.value.unshift({ time: new Date().toTimeString().split(' ')[0], msg })
  if (logs.value.length > 20) logs.value.splice(20)
}

const applyReading = (reading, source = 'API') => {
  const safe = normalizeReading(reading)
  if (!safe) return

  temperature.value = safe.temperature
  humidity.value = safe.humidity
  pressure.value = safe.pressure
  soilMoisture.value = safe.soil
  packetCount.value += 1
  lastPacketAt.value = safe.timestamp
  isOnline.value = true
  sensorSeries.value.push({ ...safe })
  if (sensorSeries.value.length > 30) sensorSeries.value.shift()

  if (commandState.value.status === 'sent') {
    commandState.value.status = 'ack'
    commandState.value.rttMs = Date.now() - commandState.value.sentAt
  }

  pushLog(`${source}: Received packet`)
}

const fetchLatest = async () => {
  const data = await AppStore.getLatest()
  if (data) applyReading(data, 'REST')
}

const activeSensorValue = computed(() => {
  const vals = { temp: temperature, hum: humidity, soil: soilMoisture, pressure: pressure }
  const units = { temp: '°C', hum: '%', soil: '%', pressure: ' hPa' }
  return `${fmt1(vals[activeSensor.value]?.value)}${units[activeSensor.value]}`
})

const lastSeenText = computed(() => {
  if (!lastPacketAt.value) return 'No Link'
  const delta = Math.floor(Date.now() / 1000) - lastPacketAt.value
  return `${delta}s ago`
})

const packetRate = computed(() => {
  const elapsedSec = Math.max(1, Math.floor((Date.now() - startedAt.value) / 1000))
  return Math.round((packetCount.value * 60) / elapsedSec)
})

const focusLinePoints = computed(() => {
  const values = sensorSeries.value.map(s => s[activeSensor.value === 'soil' ? 'soil' : activeSensor.value] || 0).slice(-12)
  if (values.length < 2) return '0,30 200,30'
  const min = Math.min(...values), range = Math.max(1, Math.max(...values) - min)
  return values.map((v, i) => `${(i / (values.length - 1)) * 200},${55 - ((v - min) / range) * 50}`).join(' ')
})

let t1, t2
const restartPolling = () => {
  if (t2) clearInterval(t2)
  const intervalMs = Math.max(1, Number(pollRate.value)) * 1000
  t2 = setInterval(fetchLatest, intervalMs)
}

onMounted(() => {
  Mqtt.connect()
  LIVE_TOPICS.forEach(t => Mqtt.subscribe(t))
  fetchLatest()
  t1 = setInterval(() => {
    uptime.value = new Date(Date.now() - startedAt.value).toISOString().substr(11, 8)
    isOnline.value = (Date.now() / 1000 - lastPacketAt.value) < 15
  }, 1000)
  restartPolling()
})

watch(() => Number(pollRate.value), () => {
  restartPolling()
})

watch(() => [Mqtt.payloadTopic, Mqtt.payload], ([topic, payload]) => {
  const cleanTopic = String(topic ?? '').trim()
  if (cleanTopic && LIVE_TOPICS.includes(cleanTopic)) applyReading(payload, 'MQTT')
}, { deep: true })

onUnmounted(() => {
  clearInterval(t1)
  clearInterval(t2)
  LIVE_TOPICS.forEach((topic) => Mqtt.unsubcribe(topic))
})

const sendCommand = (action, payload = {}) => {
  commandState.value = { action, status: 'sent', sentAt: Date.now(), rttMs: 0 }
  Mqtt.publish('620169500_sub', JSON.stringify({ action, ...payload, timestamp: Math.floor(Date.now() / 1000) }))
  pushLog(`CMD sent: ${action}`)
}

const pingDevice = () => sendCommand('reboot')
const syncNow = async () => {
  sendCommand('sync')
  await fetchLatest()
}
const clearLogs = () => logs.value = []
</script>

<style scoped>
.lab-screen {
  min-height: 100vh; padding-top: 64px;
  font-family: 'Press Start 2P', monospace;
  background: #0a0a0a; color: #4caf7d;
}

.pixel-overlay {
  position: absolute; inset: 0; pointer-events: none; z-index: 10;
  background: repeating-linear-gradient(0deg, rgba(0,0,0,0.1) 0, rgba(0,0,0,0.1) 1px, transparent 1px, transparent 3px);
}

.lab-layout {
  display: grid; grid-template-columns: 280px 1fr 280px;
  height: calc(100vh - 64px); gap: 10px; padding: 10px;
}

.pixel-card { background: #151515; border: 2px solid #333; padding: 12px; margin-bottom: 10px; }
.card-label { font-size: 8px; color: #666; margin-bottom: 8px; }

/* ── Schematic List ── */
.schematic-item { font-size: 7px; padding: 6px; border-left: 2px solid #333; margin-bottom: 4px; display: flex; justify-content: space-between; cursor: pointer; }
.schematic-item.active { border-color: #4caf7d; background: #4caf7d22; color: #fff; }
.pin-id { color: #888; }

/* ── ESP32 Model ── */
.lab-viewport { background: #000; border: 2px solid #4caf7d33; display: flex; flex-direction: column; position: relative; }
.twin-container { flex: 1; display: flex; align-items: center; justify-content: center; perspective: 800px; }

.esp32-model {
  width: 200px; height: 280px; background: #1b4d3e;
  border: 4px solid #14362d; border-radius: 6px;
  position: relative; transform: rotateX(15deg) rotateZ(-5deg);
  transition: transform 0.5s ease;
}

.mcu-chip {
  position: absolute; top: 90px; left: 50%; transform: translateX(-50%);
  width: 80px; height: 80px; background: #222; border: 2px solid #444;
  display: flex; align-items: center; justify-content: center; font-size: 8px; color: #666;
}

.pin-header { position: absolute; top: 40px; bottom: 20px; width: 14px; display: flex; flex-direction: column; gap: 4px; padding: 4px 0; }
.pin-header.left { left: 4px; }
.pin-header.right { right: 4px; }
.pin-dot { height: 8px; background: #333; border: 1px solid #000; transition: 0.3s; }
.pin-dot.highlight { background: #ffd700; box-shadow: 0 0 8px #ffd700; transform: scale(1.2); }

.traces-layer { position: absolute; inset: 0; pointer-events: none; z-index: 1; }
.trace-line { fill: none; stroke: #ffd700; stroke-width: 2; stroke-dasharray: 4; animation: flow 1s linear infinite; }

@keyframes flow { to { stroke-dashoffset: -8; } }

/* ── Nodes ── */
.data-node { position: absolute; cursor: pointer; z-index: 5; }
.node-line { width: 40px; height: 2px; background: #4caf7d; margin-bottom: 4px; transition: 0.3s; }
.node-label { background: #4caf7d; color: #000; font-size: 7px; padding: 4px; border: 1px solid #000; }
.data-node:hover .node-line { width: 60px; background: #fff; }

.node-temp { top: 90px; right: -80px; }
.node-hum { bottom: 80px; left: -80px; }
.node-soil { top: 50px; left: -90px; }
.node-pres { bottom: 40px; right: -95px; }

/* ── Footer ── */
.viewport-footer { display: flex; gap: 8px; padding: 12px; border-top: 1px solid #4caf7d22; }
.tool-btn { font-size: 7px; padding: 6px; border: 1px solid #4caf7d; cursor: pointer; }
.tool-btn:hover { background: #4caf7d; color: #000; }

.cmd-status {
  border-top: 1px solid #4caf7d22;
  padding: 8px 12px;
  font-size: 7px;
  color: #9ad9b1;
}

.ok { color: #4caf7d; }
.bad { color: #e05c5c; }

.log-terminal { height: 120px; background: #000; font-size: 6px; padding: 6px; overflow-y: auto; border: 1px solid #333; }
.log-ts { color: #666; }
.pixel-slider { width: 100%; accent-color: #4caf7d; }
.mini-chart { background: #000; border: 1px solid #333; margin-top: 8px; }
</style>