// Composables
import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  { path: '/', name: 'Home', component: () => import('@/views/Home.vue'), meta:{ transition: 'fade'} }, 
  { path: '/dashboard', name: 'Dashboard', component: () => import('@/views/Dashboard.vue'), meta:{ transition: 'fade'} },
  { path: '/Analysis', name: 'Analysis', component: () => import('@/views/Analysis.vue'), meta:{ transition: 'fade'} },
  { path: '/Control', name: 'Control', component: () => import('@/views/Control.vue'), meta:{ transition: 'fade'} }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

export default router