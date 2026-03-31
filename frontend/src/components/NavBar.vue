<template>
  <v-app-bar 
    class="solid-navbar" 
    color="primary" 
    density="comfortable" 
  >
    <v-toolbar-title class="font-weight-bold">Weather Station</v-toolbar-title>
    <v-spacer />

    <v-btn
      v-for="item in navItems"
      :key="item.name"
      variant="text"
      :active="route.name === item.name"
      @click="goTo(item.name)"
    >
      {{ item.label }}
    </v-btn>
  </v-app-bar>
</template>

<script setup>
import { useRoute, useRouter } from 'vue-router';

const router = useRouter();
const route = useRoute();

const navItems = [
  { name: 'Dashboard', label: 'Dashboard' },
  { name: 'Analysis', label: 'Analysis' },
  { name: 'Control', label: 'Control' },
];

const goTo = (routeName) => {
  if (route.name !== routeName) {
    router.push({ name: routeName });
  }
};
</script>

<style scoped>
.solid-navbar {
  background-color: rgb(var(--v-theme-primary)) !important;
}
</style>