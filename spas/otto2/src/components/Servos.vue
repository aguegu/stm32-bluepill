<script setup>
import _ from 'lodash';
import { Buffer } from 'buffer';
import {
  ref, onMounted, computed, watch,
} from 'vue';
import Meter from './Meter.vue';

const userAgent = ref(navigator.userAgent);
const isSerialUnsupported = ref(false);
const isConnected = ref(false);

const COUNT = 4;
const servos = ref(_.times(COUNT, () => ({
  initial: 150,
  deg0: 50,
  deg90: 150,
  deg180: 250,
  degree: 150,
})));

const config = {
  baudRate: 115200,
  dataBits: 8,
  stopBits: 1,
  parity: 'none',
  flowControl: 'none',
};

const currentDegrees = computed(() => _.map(servos.value, 'degree'));

const bitPerByte = 9 + (config.parity === 'none' ? 0 : 1);

let port = null;
let reader = null;
let writer = null;
let counter = 0;
const rxBuffer = [];

const logLevels = ['debug', 'log', 'info'];
const logIndex = 0;

const log = (msg, level = 'log') => {
  // if (level === 'info') {
  //   logs.value.push(msg);
  // }
  if (logLevels.indexOf(level) >= logIndex) {
    console[level](msg); // eslint-disable-line no-console
  }
};

const readFromSerial = async () => {
  const { value, done } = await reader.read(); // eslint-disable-line no-await-in-loop
  if (done) {
    return Promise.resolve();
  }
  rxBuffer.push(...Array.from(value));
  // log(`rx: ${Buffer.from(rxBuffer).toString('hex').match(/../g).join(' ')}`, 'debug');
  return readFromSerial();
};

const reopenPort = async () => {
  await port.open(config);
  isConnected.value = true;
  // console.log(this.port); // eslint-disable-line no-console
  // console.log(this.port.getInfo()); // eslint-disable-line no-console

  // this.reader = this.port.readable.getReader();

  reader = port.readable.getReader();
  writer = port.writable.getWriter();

  readFromSerial();

  // const readEncoder = new window.TextDecoderStream();
  // this.readerPipeline = this.port.readable.pipeTo(readEncoder.writable);
  // this.reader = readEncoder.readable.getReader();
  //
  // const writeEncoder = new window.TextEncoderStream();
  // this.writerPipeline = writeEncoder.readable.pipeTo(this.port.writable);
  // this.writer = writeEncoder.writable.getWriter();
  // // this.writer = this.port.writable.getWriter();
  // this.readFromSerial();
};

const openPort = async () => {
  port = await navigator.serial.requestPort();
  return reopenPort();
};

const transmit = (payload, timeout = 36) => new Promise((resolve, reject) => {
  const txBuffer = Buffer.alloc(payload.length + 3, 0x00);
  txBuffer.writeUInt8(payload.length + 2, 0);
  txBuffer.writeUInt8(counter & 0xff, 1);
  txBuffer.writeUInt8(~counter & 0xff, 2);
  payload.copy(txBuffer, 3);

  log(`tx: ${txBuffer.toString('hex').match(/../g).join(' ')}`, 'debug');

  let recvTimeout = timeout + Math.ceil((bitPerByte * txBuffer.length * 1000) / config.baudRate);
  // console.log({ recvTimeout, timeout }); // eslint-disable-line no-console
  if (!isConnected.value) {
    reject(new Error('port closed'));
  } else {
    let start = 0;
    const span = Math.ceil((bitPerByte * 1 * 1000) / config.baudRate);
    let received = 0;

    const timer = setInterval(() => {
      const res = Buffer.from(rxBuffer);
      // console.log(res); // eslint-disable-line no-console
      if (res.length
        && res.readUInt8(0)
        && res.readUInt8(0) <= rxBuffer.length + 1
        && res.readUInt8(1) === txBuffer.readUInt8(1)
        && res.readUInt8(1) + res.readUInt8(2) === 0xff
      ) {
        clearInterval(timer);
        log(`rx: ${res.toString('hex').match(/../g).join(' ')}`, 'debug');
        resolve(rxBuffer.splice(0, rxBuffer.length + 1).slice(3));
      } else if (start > recvTimeout) {
        if (res.length > received) {
          // console.log({ txBuffer, res, recvTimeout }); // eslint-disable-line no-console
          recvTimeout += span + span;
          received = res.length;
        } else {
          clearInterval(timer);
          // log('timeout', 'ERROR');
          reject(new Error('timeout'));
        }
      }
      start += span;
    }, span);
    writer.write(txBuffer);
    counter += 1;
  }
});

const readSettings = async () => {
  const res = await transmit(Buffer.from([0x04]));
  // console.log(res); // eslint-disable-line no-console
  if (res[0] === 0x00) {
    log(`rx package: ${res.slice(1)}`, 'debug');
    _.range(COUNT).forEach((i) => {
      const offset = 1 + i * 8;
      const buff = Buffer.from(res.slice(offset, offset + 8));
      // console.log(buff); // eslint-disable-line no-console
      servos.value[i].initial = buff.readUInt16LE();
      servos.value[i].deg90 = buff.readUInt16LE(2);
      servos.value[i].deg0 = buff.readUInt16LE(4);
      servos.value[i].deg180 = buff.readUInt16LE(6);
    });
  }
};

const closePort = async () => {
  await writer.close();
  await writer.releaseLock();
  await reader.cancel();
  await reader.releaseLock();
  await port.close();
  isConnected.value = false;
};

watch(currentDegrees, (newVal, oldVal) => {
  const changed = newVal.map((x, i) => (x === oldVal[i] ? undefined : x));
  const index = _.findIndex(changed, (x) => !_.isUndefined(x));
  // console.log(index, newVal[index]); // eslint-disable-line no-console
  const txPkg = Buffer.alloc(6, 0x00);
  txPkg.writeUInt8(0x01, 0);
  txPkg.writeUInt8(index, 1);
  txPkg.writeUInt16LE(newVal[index], 2);
  txPkg.writeUInt16LE(1, 4);
  txPkg.writeUInt8(0, 5);
  // console.log(txPkg); // eslint-disable-line no-console
  if (isConnected.value) {
    transmit(txPkg);
  }
});

onMounted(async () => {
  isSerialUnsupported.value = !navigator.serial;
});

</script>

<template>
  <n-card>
    <n-alert v-if="isSerialUnsupported" title="Please try Chrome" type="warning" style="margin-bottom: 2em;">
      This browser "{{ userAgent }}" does not support <a href="https://developer.mozilla.org/en-US/docs/Web/API/Serial" class="warning--text">Web Serial API</a> yet.
    </n-alert>
    <n-space style="margin-bottom: 1em;">
      <n-button type="primary" :disabled="isConnected" @click="openPort">Open Serial Port</n-button>
      <n-button v-if="isConnected" type="primary" @click="closePort">Disconnect</n-button>
      <n-button v-if="isConnected" type="primary" @click="readSettings">Read</n-button>
    </n-space>
    <n-space v-for="servo, i in servos" :key="i" vertical>
      <n-space style="padding-bottom: 1em;">
        <n-tag size="large" style="width: 2.5em;">#{{ i }}</n-tag>
        <n-input-group>
          <n-input-group-label>Initial</n-input-group-label>
          <n-input-number v-model:value="servo.initial" :min="servo.deg0" :max="servo.deg180" :style="{ width: '6em' }" />
          <n-button secondary type="error" @click="servo.initial = servo.degree">Set</n-button>
          <n-button secondary type="primary" @click="servo.degree = servo.initial">Go</n-button>
        </n-input-group>
        <n-input-group>
          <n-input-group-label>0&deg;</n-input-group-label>
          <n-input-number v-model:value="servo.deg0" :min="50" :max="servo.deg90" :style="{ width: '6em' }" />
          <n-button secondary type="error" @click="servo.deg0 = servo.degree">Set</n-button>
          <n-button secondary type="primary" @click="servo.degree = servo.deg0">Go</n-button>
        </n-input-group>
        <n-input-group>
          <n-input-group-label>90&deg;</n-input-group-label>
          <n-input-number v-model:value="servo.deg90" :min="servo.deg0" :max="servo.deg180" :style="{ width: '6em' }" />
          <n-button secondary type="error" @click="servo.deg90 = servo.degree">Set</n-button>
          <n-button secondary type="primary" @click="servo.degree = servo.deg90">Go</n-button>
        </n-input-group>
        <n-input-group>
          <n-input-group-label>180&deg;</n-input-group-label>
          <n-input-number v-model:value="servo.deg180" :min="servo.deg90" :max="600" :style="{ width: '6em' }" />
          <n-button secondary type="error" @click="servo.deg180 = servo.degree">Set</n-button>
          <n-button secondary type="primary" @click="servo.degree = servo.deg180">Go</n-button>
        </n-input-group>
        <n-grid :cols="3" :x-gap="2">
          <n-gi style="padding-top: 0.5em;">
            <n-slider v-model:value="servo.degree" :min="50" :max="600" />
          </n-gi>
          <n-gi>
            <svg :width="100" :height="25" xmlns="http://www.w3.org/2000/svg">
              <!-- <circle :cx="center.x" :cy="center.y" :r="radius" fill="yellow" /> -->
              <!-- <line :x1="radius" :y1="radius" :x2="pointer.x" :y2="pointer.y" style="stroke:rgb(255,0,0);stroke-width:2" /> -->
              <line :x1="0" :y1="25" :x2="5" :y2="25" style="stroke:blue;" />
              <line :x1="5" :y1="0" :x2="servo.degree / 10 + 5" :y2="0" style="stroke:blue;" />
              <line :x1="servo.degree / 10 + 5" :y1="25" :x2="100" :y2="25" style="stroke:blue;" />
              <line :x1="5" :y1="0" :x2="5" :y2="25" style="stroke:blue;" />
              <line :x1="servo.degree / 10 + 5" :y1="0" :x2="servo.degree / 10 + 5" :y2="25" style="stroke:blue;" />
            </svg>
          </n-gi>
          <n-gi>
            <Meter :angle="(servo.degree - servo.deg0) / (servo.deg180 - servo.deg0) * 180" />
          </n-gi>
        </n-grid>
      </n-space>
    </n-space>
    <!-- <pre>{{ currentDegrees }}</pre> -->
    <!-- <pre>{{ servos }}</pre> -->
  </n-card>
</template>
