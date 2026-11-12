<script setup>
import { ref, computed } from 'vue';

const radius = ref(100);
const angle = ref(90);

const center = computed(() => ({
  x: radius.value,
  y: radius.value,
}));

const pointer = computed(() => ({
  x: center.value.x - radius.value * Math.cos((angle.value * Math.PI) / 180),
  y: center.value.y - radius.value * Math.sin((angle.value * Math.PI) / 180),
}));

</script>

<template>
  <div>
    <input v-model.number="angle" type="range" min="0" max="180"><span>{{ angle }}</span>
    <div>
      <svg :width="radius * 2" :height="radius * 2" xmlns="http://www.w3.org/2000/svg">
        <circle :cx="center.x" :cy="center.y" :r="radius" fill="yellow" />
        <line :x1="radius" :y1="radius" :x2="pointer.x" :y2="pointer.y" style="stroke:rgb(255,0,0);stroke-width:2" />
      </svg>
    </div>
    <pre>{{ center }}</pre>
    <pre>{{ pointer }}</pre>
  </div>
</template>
