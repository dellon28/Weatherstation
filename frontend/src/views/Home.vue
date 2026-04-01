<template>
  <section class="home-screen">
    <div class="pixel-overlay"></div>
    <div class="status-strip">
      <span :class="wifiConnected ? 'ok' : 'bad'">WiFi {{ wifiConnected ? 'ON' : 'OFF' }}</span>
      <span :class="mqttConnected ? 'ok' : 'warn'">MQTT {{ mqttConnected ? 'OK' : 'WAIT' }}</span>
    </div>

    <div class="pixel-panel">
      <div class="inner-content">
        <div class="sprite-wrap mb-5 mb-sm-6">
          <img
            :class="['flower', plantAnimationClass]"
            src="@/assets/flower.png"
            alt="Plant mascot"
          />
        </div>

        <h1 class="title mb-2">GROWup</h1>
        <p class="subtitle mb-6">Your weather companion is ready to start.</p>

        <div class="name-panel mt-5">
          <v-text-field
            v-model="petName"
            class="pixel-input"
            label="Name your plant"
            variant="outlined"
            density="comfortable"
            maxlength="16"
            hide-details
            @keyup.enter="goToDashboard"
          />
          <v-btn class="pixel-btn mt-4" color="primary" block @click="goToDashboard">Go To Dashboard</v-btn>
        </div>
      </div>
    </div>
  </section>
</template>

<script setup>
import { computed, ref, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import bitLoveTrack from '@/assets/8 Bit Love! Happy Fun Chiptune Game Music by HeatleyBros.mp3'

const router = useRouter()
const wifiConnected = ref(true)
const mqttConnected = ref(false)
const petName = ref('')
let bgm

const plantAnimationClass = computed(() => 'bounce')

function goToDashboard() {
  const trimmed = petName.value.trim()
  if (trimmed) {
    localStorage.setItem('plantName', trimmed)
  }
  mqttConnected.value = true
  router.push('/dashboard')
}

onMounted(() => {
  bgm = new Audio(bitLoveTrack)
  bgm.loop = true
  bgm.volume = 0.35

  bgm.play().catch(() => {
    // Browser may block autoplay until user interaction.
  })
})

onUnmounted(() => {
  if (!bgm) return
  bgm.pause()
  bgm.currentTime = 0
})
</script>

<style scoped>
.home-screen {
  position: relative;
  min-height: 100vh;
  overflow: hidden;
  background:
    radial-gradient(circle at 20% 10%, #ffd08a 0, transparent 45%),
    radial-gradient(circle at 90% 85%, #ff8cb8 0, transparent 35%),
    linear-gradient(180deg, #2f223f 0%, #1e1b3a 65%, #121228 100%);
  font-family: 'Press Start 2P', 'Courier New', monospace;
}

.pixel-overlay {
  position: absolute;
  inset: 0;
  pointer-events: none;
  background-image: repeating-linear-gradient(
    0deg,
    rgba(255, 255, 255, 0.06) 0,
    rgba(255, 255, 255, 0.06) 1px,
    transparent 1px,
    transparent 3px
  );
  opacity: 0.5;
}

/* Card now fills the entire viewport */
.pixel-panel {
  position: relative;
  z-index: 1;
  min-height: 100vh;
  width: 100%;
  background: #fff3cf;
  border: 4px solid #3a2540;
  box-shadow: inset 0 0 0 4px #ffc86d;
  text-align: center;
  display: flex;
  align-items: center;
  justify-content: center;
}

/* Centers content with a comfortable max-width */
.inner-content {
  width: 100%;
  max-width: 480px;
  padding: 2rem 1.5rem;
}

.status-strip {
  position: absolute;
  top: 14px;
  right: 14px;
  z-index: 2;
  display: flex;
  flex-wrap: wrap;
  justify-content: flex-end;
  gap: 12px;
  font-weight: 700;
  font-size: 12px;
  background: rgba(255, 243, 207, 0.9);
  border: 2px solid #2c1a33;
  padding: 8px 10px;
  color: #1d1330;
}

.sprite-wrap {
  display: flex;
  justify-content: center;
}

.ok   { color: #0b7a4a; }
.warn { color: #ba5c00; }
.bad  { color: #b00020; }

.flower {
  width: min(45vw, 210px);
  height: auto;
  image-rendering: pixelated;
  image-rendering: crisp-edges;
  transform-origin: center bottom;
}

.sway   { animation: sway 3.4s ease-in-out infinite; }
.bounce { animation: bounce 1.05s ease-in-out infinite; }

.title {
  color: #2b1236;
  font-size: clamp(1.05rem, 3vw, 1.7rem);
  letter-spacing: 1px;
}

.subtitle {
  color: #503262;
  font-size: clamp(0.65rem, 1.8vw, 0.85rem);
  line-height: 1.8;
}

.name-panel {
  max-width: 360px;
  margin: 0 auto;
}

.pixel-btn {
  border-radius: 0;
  border: 3px solid #2c1a33;
  box-shadow: 4px 4px 0 #2c1a33;
  font-weight: 700;
  letter-spacing: 0.8px;
}

.pixel-input :deep(.v-field) {
  border-radius: 0;
  border: 3px solid #2c1a33;
  background: #fff9ec;
}

@keyframes sway {
  0%, 100% { transform: rotate(-4deg) translateY(0); }
  50%       { transform: rotate(4deg) translateY(-2px); }
}

@keyframes bounce {
  0%, 100% { transform: translateY(0) scale(1); }
  40%       { transform: translateY(-14px) scale(1.02); }
  70%       { transform: translateY(-4px) scale(0.98); }
}

@media (max-width: 600px) {
  .status-strip {
    top: 10px;
    right: 10px;
    font-size: 10px;
    gap: 8px;
    padding: 6px 8px;
  }
}
</style>