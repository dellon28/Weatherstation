<template>
    <section class="live-screen plant-tama-bg with-navbar">
        <div class="pixel-overlay"></div>

        <div class="content-wrapper">
            <!-- Topbar -->
            <div class="topbar pixel-card bg-card mb-4">
                <h1 class="dash-title pixel-font brand-text">LIVE SENSOR CHARTS</h1>
                <div class="status-row">
                    <span class="pixel-pill">SAMPLES: {{ sampleCount }}</span>
                    <span class="pixel-pill">UPDATED: <span class="mono-font">{{ lastUpdateText }}</span></span>
                </div>
            </div>

            <!-- Metric Cards -->
            <div class="sensor-grid mb-4">
                <article class="pixel-card metric-card metric-temp">
                    <div class="pixel-label">TEMPERATURE</div>
                    <div class="d-flex align-end">
                        <span class="pixel-value">{{ latest.temperature.toFixed(1) }}</span>
                        <span class="pixel-unit ml-1 mb-1">°C</span>
                    </div>
                </article>
                <article class="pixel-card metric-card metric-hum">
                    <div class="pixel-label">HUMIDITY</div>
                    <div class="d-flex align-end">
                        <span class="pixel-value">{{ latest.humidity.toFixed(1) }}</span>
                        <span class="pixel-unit ml-1 mb-1">%</span>
                    </div>
                </article>
                <article class="pixel-card metric-card metric-soil">
                    <div class="pixel-label">SOIL MOISTURE</div>
                    <div class="d-flex align-end">
                        <span class="pixel-value">{{ latest.soil.toFixed(1) }}</span>
                        <span class="pixel-unit ml-1 mb-1">%</span>
                    </div>
                </article>
                <article class="pixel-card metric-card metric-vpd">
                    <div class="pixel-label">VPD</div>
                    <div class="d-flex align-end">
                        <span class="pixel-value-sm">{{ latest.vpd.toFixed(2) }}</span>
                        <span class="pixel-unit ml-1 mb-1">kPa</span>
                    </div>
                </article>
            </div>

            <!-- Chart Grid -->
            <div class="chart-grid">
                <div class="pixel-card bg-card chart-box-wrap">
                    <div ref="climateChartEl" class="chart-box"></div>
                </div>
                <div class="pixel-card bg-card chart-box-wrap">
                    <div ref="rootZoneChartEl" class="chart-box"></div>
                </div>
                <div class="pixel-card bg-card chart-box-wrap chart-wide">
                    <div ref="hortiChartEl" class="chart-box"></div>
                </div>
            </div>
        </div>
    </section>
</template>

<script setup>
import { onMounted, onUnmounted, reactive, ref, watch } from 'vue'
import Highcharts from 'highcharts'
import { useAppStore } from '@/store/appStore'
import { useMqttStore } from '@/store/mqttStore'

// Load Custom Fonts once.
if (!document.querySelector('link[data-font="press-start-2p"]')) {
    const link = document.createElement('link')
    link.href = 'https://fonts.googleapis.com/css2?family=Press+Start+2P&display=swap'
    link.rel = 'stylesheet'
    link.dataset.font = 'press-start-2p'
    document.head.appendChild(link)
}

const AppStore = useAppStore()
const MqttStore = useMqttStore()
const LIVE_TOPICS = ['620169500', '620169500_pub', '620169500_sub']
const MAX_POINTS = 180
const STALE_HEARTBEAT_SECONDS = 20

const climateChartEl = ref(null)
const rootZoneChartEl = ref(null)
const hortiChartEl = ref(null)

const sampleCount = ref(0)
const lastUpdateText = ref('WAITING...')

let climateChart = null
let rootZoneChart = null
let hortiChart = null
let fallbackTimer = null
let redrawQueued = false
let lastSignature = ''

const latest = reactive({
    temperature: 0,
    humidity: 0,
    pressure: 1013.25,
    soil: 0,
    heatindex: 0,
    dewpoint: 0,
    vpd: 0,
})

const calcDewPointC = (tempC, humidityPct) => {
    const t = Number(tempC ?? 0)
    const rh = Math.max(1, Math.min(100, Number(humidityPct ?? 0)))
    const a = 17.27
    const b = 237.7
    const alpha = ((a * t) / (b + t)) + Math.log(rh / 100)
    return (b * alpha) / (a - alpha)
}

const calcVpdKPa = (tempC, humidityPct) => {
    const t = Number(tempC ?? 0)
    const rh = Math.max(0, Math.min(100, Number(humidityPct ?? 0)))
    const svp = 0.6108 * Math.exp((17.27 * t) / (t + 237.3))
    return Math.max(0, svp * (1 - rh / 100))
}

const resolveSoilPercent = (raw) => {
    const direct = Number(raw?.soil ?? raw?.soil_percent ?? raw?.soilPercent ?? raw?.soil_moisture)
    if (Number.isFinite(direct)) return Math.max(0, Math.min(100, direct))

    const adc = Number(raw?.soil_adc ?? raw?.adc ?? NaN)
    if (Number.isFinite(adc)) return Math.max(0, Math.min(100, ((4095 - adc) / 4095) * 100))

    return 0
}

const normalizeReading = (raw) => {
    if (!raw || typeof raw !== 'object') return null

    const timestamp = Number(raw.timestamp ?? Math.floor(Date.now() / 1000))
    const temperature = Number(raw.temperature ?? 0)
    const humidity = Number(raw.humidity ?? 0)
    const pressure = Number(raw.pressure ?? 1013.25)
    const heatindex = Number(raw.heatindex ?? raw.heat_index ?? temperature)
    const soil = resolveSoilPercent(raw)
    const dewpoint = calcDewPointC(temperature, humidity)
    const vpd = calcVpdKPa(temperature, humidity)

    return {
        timestamp: Number.isFinite(timestamp) ? timestamp : Math.floor(Date.now() / 1000),
        temperature: Number.isFinite(temperature) ? temperature : 0,
        humidity: Number.isFinite(humidity) ? humidity : 0,
        pressure: Number.isFinite(pressure) ? pressure : 1013.25,
        heatindex: Number.isFinite(heatindex) ? heatindex : 0,
        soil: Number.isFinite(soil) ? soil : 0,
        dewpoint: Number.isFinite(dewpoint) ? dewpoint : 0,
        vpd: Number.isFinite(vpd) ? vpd : 0,
    }
}

const fetchLatestAndPush = async () => {
    const latestRow = await AppStore.getLatest()
    if (!latestRow || typeof latestRow !== 'object') return
    const reading = normalizeReading(latestRow)
    if (!reading) return

    const nowSec = Math.floor(Date.now() / 1000)
    if ((nowSec - reading.timestamp) >= STALE_HEARTBEAT_SECONDS) {
        reading.timestamp = nowSec
    }

    pushToCharts(reading)
}

// Global Highcharts theme adapted for PlantTama
const getChartTheme = () => ({
    chart: {
        backgroundColor: 'transparent',
        style: { fontFamily: "'Courier New', monospace" },
        spacingTop: 24,
        spacingRight: 12,
        spacingLeft: 10,
        spacingBottom: 20,
    },
    title: {
        style: { color: '#3A2540', fontFamily: "'Press Start 2P', cursive", fontSize: '10px' },
        align: 'left',
        margin: 14,
        y: 12,
    },
    subtitle: {
        style: { color: '#7a6a8a', fontSize: '10px' },
        align: 'left',
        y: 34,
    },
    xAxis: {
        type: 'datetime',
        labels: { style: { color: '#3A2540', fontWeight: 'bold' } },
        lineColor: '#3A2540',
        tickColor: '#3A2540',
    },
    yAxis: {
        labels: { style: { color: '#3A2540', fontWeight: 'bold' } },
        gridLineColor: 'rgba(58, 37, 64, 0.1)',
    },
    legend: {
        itemStyle: {
            color: '#3A2540',
            fontFamily: "'Press Start 2P', cursive",
            fontSize: '7px',
        },
        layout: 'horizontal',
        align: 'center',
        verticalAlign: 'bottom',
        itemDistance: 10,
        margin: 16,
        padding: 8,
        maxHeight: 60,
    },
    plotOptions: {
        series: {
            animation: false,
            marker: { enabled: true, radius: 2, symbol: 'circle' },
            lineWidth: 2,
        },
    },
    credits: { enabled: false },
})

const initCharts = () => {
    climateChart = Highcharts.chart(climateChartEl.value, Highcharts.merge(getChartTheme(), {
        chart: { type: 'spline' },
        title: { text: 'CLIMATE TRENDS' },
        yAxis: [{ title: { text: '°C / %', style: { color: '#3A2540', fontFamily: "'Press Start 2P', cursive", fontSize: '8px' } } }],
        series: [
            { name: 'Temperature (°C)', data: [], color: '#E05C5C' },
            { name: 'Humidity (%)', data: [], color: '#5B9EF0' },
            { name: 'Heat Index (°C)', data: [], color: '#E0A020' },
        ],
    }))

    rootZoneChart = Highcharts.chart(rootZoneChartEl.value, Highcharts.merge(getChartTheme(), {
        chart: { type: 'areaspline' },
        title: { text: 'ROOT ZONE & PRESSURE' },
        yAxis: [
            { title: { text: 'SOIL MOISTURE %', style: { color: '#3A2540', fontFamily: "'Press Start 2P', cursive", fontSize: '8px' } }, max: 100, min: 0 },
            { title: { text: 'PRESSURE hPa', style: { color: '#3A2540', fontFamily: "'Press Start 2P', cursive", fontSize: '8px' } }, opposite: true },
        ],
        series: [
            { name: 'Soil Moisture (%)', data: [], color: '#4CAF7D', fillOpacity: 0.2 },
            { name: 'Pressure (hPa)', data: [], color: '#9B6ECF', yAxis: 1 },
        ],
    }))

    hortiChart = Highcharts.chart(hortiChartEl.value, Highcharts.merge(getChartTheme(), {
        chart: { type: 'line' },
        title: { text: 'HORTICULTURE METRICS' },
        subtitle: { text: 'Derived from live sensor data' },
        yAxis: [
            { title: { text: 'VPD (kPa)', style: { color: '#3A2540', fontFamily: "'Press Start 2P', cursive", fontSize: '8px' } } },
            { title: { text: 'DEW POINT (°C)', style: { color: '#3A2540', fontFamily: "'Press Start 2P', cursive", fontSize: '8px' } }, opposite: true },
        ],
        series: [
            { name: 'VPD (kPa)', data: [], color: '#FFC86D' },
            { name: 'Dew Point (°C)', data: [], color: '#5B9EF0', yAxis: 1 },
        ],
    }))
}

const seedChartsIfEmpty = () => {
    if (!climateChart || climateChart.series[0].data.length > 0) return

    const nowMs = Date.now()
    const seed = {
        timestamp: Math.floor(nowMs / 1000),
        temperature: 0,
        humidity: 0,
        pressure: 1013.25,
        heatindex: 0,
        soil: 0,
        dewpoint: 0,
        vpd: 0,
    }
    pushToCharts(seed)
}

const queueRedraw = () => {
    if (redrawQueued) return
    redrawQueued = true
    requestAnimationFrame(() => {
        climateChart?.redraw()
        rootZoneChart?.redraw()
        hortiChart?.redraw()
        redrawQueued = false
    })
}

const pushToCharts = (reading, shouldRedraw = true) => {
    if (!climateChart || !rootZoneChart || !hortiChart) return

    const signature = `${reading.timestamp}|${reading.temperature}|${reading.humidity}|${reading.pressure}|${reading.soil}|${reading.heatindex}`
    if (signature === lastSignature) return
    lastSignature = signature

    const t = reading.timestamp * 1000
    const shift = climateChart.series[0].data.length >= MAX_POINTS

    climateChart.series[0].addPoint([t, reading.temperature], false, shift)
    climateChart.series[1].addPoint([t, reading.humidity], false, shift)
    climateChart.series[2].addPoint([t, reading.heatindex], false, shift)

    rootZoneChart.series[0].addPoint([t, reading.soil], false, shift)
    rootZoneChart.series[1].addPoint([t, reading.pressure], false, shift)

    hortiChart.series[0].addPoint([t, reading.vpd], false, shift)
    hortiChart.series[1].addPoint([t, reading.dewpoint], false, shift)

    if (shouldRedraw) queueRedraw()

    latest.temperature = reading.temperature
    latest.humidity = reading.humidity
    latest.pressure = reading.pressure
    latest.soil = reading.soil
    latest.heatindex = reading.heatindex
    latest.dewpoint = reading.dewpoint
    latest.vpd = reading.vpd
    sampleCount.value += 1
    lastUpdateText.value = new Date(t).toLocaleTimeString()
}

const loadInitialHistory = async () => {
    const end = Math.floor(Date.now() / 1000)
    const start = end - (2 * 60 * 60)
    const rows = await AppStore.getAllInRange(start, end)

    if (!Array.isArray(rows)) return

    const ordered = [...rows].sort((a, b) => Number(a.timestamp ?? 0) - Number(b.timestamp ?? 0))
    for (const row of ordered) {
        const reading = normalizeReading(row)
        if (!reading) continue
        pushToCharts(reading, false)
    }

    queueRedraw()
}

watch(
    () => [MqttStore.payloadTopic, MqttStore.payload],
    ([topic, payload]) => {
        const cleanTopic = String(topic ?? '').trim()
        if (!cleanTopic || !LIVE_TOPICS.includes(cleanTopic)) return
        const reading = normalizeReading(payload)
        if (!reading) return
        pushToCharts(reading)
    },
    { deep: true }
)

onMounted(async () => {
    initCharts()
    seedChartsIfEmpty()
    await loadInitialHistory()
    await fetchLatestAndPush()
    MqttStore.connect()
    LIVE_TOPICS.forEach((topic) => MqttStore.subscribe(topic))
    fallbackTimer = setInterval(fetchLatestAndPush, 12000)
})

onUnmounted(() => {
    if (fallbackTimer) {
        clearInterval(fallbackTimer)
        fallbackTimer = null
    }
    MqttStore.unsubcribeAll()
    climateChart?.destroy()
    rootZoneChart?.destroy()
    hortiChart?.destroy()
})
</script>

<style scoped>
.with-navbar {
    padding-top: 74px;
}

.live-screen {
    position: relative;
    min-height: 100vh;
    padding: 16px;
    background-color: #FFF3CF !important;
    overflow-x: hidden;
}

.pixel-overlay {
    position: absolute;
    inset: 0;
    pointer-events: none;
    background-image: repeating-linear-gradient(
        0deg,
        rgba(58, 37, 64, 0.04) 0,
        rgba(58, 37, 64, 0.04) 1px,
        transparent 1px,
        transparent 3px
    );
    opacity: 0.8;
    z-index: 0;
}

.content-wrapper {
    position: relative;
    z-index: 1;
    max-width: 1400px;
    margin: 0 auto;
}

.pixel-font { font-family: 'Press Start 2P', cursive; }
.mono-font { font-family: 'Courier New', monospace; font-weight: bold; }
.brand-text { color: #3A2540; }
.bg-card { background-color: #FFF9EC !important; }

.pixel-card {
    border: 3px solid #3A2540 !important;
    box-shadow: 4px 4px 0 rgba(0, 0, 0, 0.1) !important;
    border-radius: 0 !important;
    padding: 20px;
}

.topbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 16px;
    padding: 20px 24px;
}

.dash-title {
    margin: 0;
    font-size: 1.4rem;
    letter-spacing: 1px;
}

.status-row {
    display: flex;
    gap: 12px;
    flex-wrap: wrap;
}

.pixel-pill {
    background: #FFF3CF;
    border: 2px solid #3A2540;
    padding: 8px 12px;
    font-family: 'Press Start 2P', cursive;
    font-size: 0.6rem;
    color: #3A2540;
    box-shadow: 2px 2px 0 rgba(0,0,0,0.1);
}

.sensor-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 16px;
}

.metric-card {
    display: flex;
    flex-direction: column;
    justify-content: space-between;
}

.pixel-label {
    font-family: 'Press Start 2P', cursive;
    font-size: 0.65rem;
    color: rgba(255, 255, 255, 0.9);
    margin-bottom: 12px;
    line-height: 1.4;
}

.pixel-value {
    font-family: 'Press Start 2P', cursive;
    font-size: 2.2rem;
    color: white;
    line-height: 1;
}

.pixel-value-sm {
    font-family: 'Press Start 2P', cursive;
    font-size: 1.8rem;
    color: white;
    line-height: 1;
}

.pixel-unit {
    font-family: 'Courier New', monospace;
    font-size: 1.2rem;
    font-weight: 900;
    color: rgba(255, 255, 255, 0.8);
}

.metric-temp { background-color: #E05C5C !important; }
.metric-hum  { background-color: #5B9EF0 !important; }
.metric-soil { background-color: #4CAF7D !important; }
.metric-vpd  { background-color: #FFC86D !important; }

.metric-vpd .pixel-label,
.metric-vpd .pixel-value-sm,
.metric-vpd .pixel-unit {
    color: #3A2540 !important;
}

.chart-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 16px;
}

.chart-box-wrap {
    padding: 16px;
}

.chart-box {
    min-height: 340px;
    width: 100%;
}

.chart-wide {
    grid-column: span 2;
}

@media (max-width: 1024px) {
    .chart-grid {
        grid-template-columns: 1fr;
    }
    .chart-wide {
        grid-column: span 1;
    }
}
</style>