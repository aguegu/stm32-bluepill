<script setup>
import _ from 'lodash';
import { ref } from 'vue';

const COUNT = 4;
const servos = ref(_.times(COUNT, () => ({
  deg0: 500,
  deg90: 1500,
  deg180: 2500,
  degree: 1500,
})));
</script>

<template>
  <n-card title="Servos">
    <n-space v-for="servo, i in servos" :key="i" vertical>
      <n-space style="padding-bottom: 1em;">
        <n-tag size="large" style="width: 2.5em;">#{{ i }}</n-tag>
        <n-grid :cols="4" :x-gap="6">
          <n-gi>
            <n-space>
              <n-input-number v-model:value="servo.deg0" :min="1" />
              <n-button secondary type="primary" @click="servo.deg0 = servo.degree">0&deg;</n-button>
            </n-space>
          </n-gi>
          <n-gi>
            <n-space>
              <n-input-number v-model:value="servo.deg90" :min="1" />
              <n-button secondary type="primary" @click="servo.deg90 = servo.degree">90&deg;</n-button>
            </n-space>
          </n-gi>
          <n-gi>
            <n-space>
              <n-input-number v-model:value="servo.deg180" :min="1" />
              <n-button secondary type="primary" @click="servo.deg180 = servo.degree">180&deg;</n-button>
            </n-space>
          </n-gi>
          <n-gi style="padding-top: 0.5em;">
            <n-slider v-model:value="servo.degree" :min="0" :max="3000" />
          </n-gi>
        </n-grid>
      </n-space>
    </n-space>

    <pre>{{ servos }}</pre>
  </n-card>
</template>
