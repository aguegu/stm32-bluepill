<script setup>
import { zhCN } from 'naive-ui';
import Servos from './components/Servos.vue';
</script>

<template>
  <n-config-provider :locale="zhCN">
    <n-layout>
      <n-layout-header>
        <n-page-header>
          <template #title>
            <n-h1>OTTO Servo Calibrator</n-h1>
          </template>
        </n-page-header>
      </n-layout-header>
      <n-layout-content>
        <Servos />
      </n-layout-content>
    </n-layout>
  </n-config-provider>
</template>
