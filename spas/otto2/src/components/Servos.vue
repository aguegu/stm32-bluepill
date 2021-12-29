<script setup>
import { ref } from 'vue';

const count = ref(0);
</script>

<template>
  <n-card title="Servos">
    <n-space>
      <n-space justify="space-between">
        <n-tag size="large">
          #0
        </n-tag>
        <n-input-number v-model:value="count" auto-size style="margin-left: 2em;" />
        <n-button>0&deg;</n-button>
        <n-input-number v-model:value="count" auto-size style="margin-left: 2em;" />
        <n-button>90&deg;</n-button>
        <n-input-number v-model:value="count" auto-size style="margin-left: 2em;" />
        <n-button>180&deg;</n-button>
      </n-space>
    </n-space>

    <n-button @click="count++">
      count is: {{ count }}
    </n-button>
  </n-card>
</template>
