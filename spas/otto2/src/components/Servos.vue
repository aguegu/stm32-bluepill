<script setup>
import _ from 'lodash';
import { Buffer } from 'buffer';
import { ref, onMounted } from 'vue';

const userAgent = ref(navigator.userAgent);
const isSerialUnsupported = ref(false);
const isConnected = ref(false);

const COUNT = 4;
const servos = ref(_.times(COUNT, () => ({
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
  log(`rx: ${Buffer.from(rxBuffer).toString('hex').match(/../g).join(' ')}`, 'debug');
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

const transmit = (payload, timeout = 12) => new Promise((resolve, reject) => {
  const txBuffer = Buffer.alloc(payload.length + 3, 0x00);
  txBuffer.writeUInt8(payload.length + 2, 0);
  txBuffer.writeUInt8(counter & 0xff, 1);
  txBuffer.writeUInt8(~counter & 0xff, 2);
  payload.copy(txBuffer, 3);

  log(`tx: ${txBuffer.toString('hex').match(/../g).join(' ')}`, 'debug');

  let recvTimeout = timeout + Math.ceil((10 * txBuffer.length * 1000) / config.baudRate);
  // console.log({ recvTimeout, timeout }); // eslint-disable-line no-console
  if (!isConnected.value) {
    reject(new Error('port closed'));
  } else {
    rxBuffer.splice(0, rxBuffer.length); // clear rxBuffer
    let start = 0;
    const span = Math.ceil((11 * 2 * 1000) / config.baudRate);
    let received = 0;

    const timer = setInterval(() => {
      const res = Buffer.from(rxBuffer);
      // console.log(res); // eslint-disable-line no-console
      if (res.length && res.readUInt8(res.length - 1) === 0x16
          && res.slice(0, 3).equals(Buffer.from([0x46, 0xb9, 0x68]))
          && res.readUInt16BE(3) + 2 === res.length
          && res.slice(2, -3).reduce((s, c) => s + c) === res.slice(-3, -1).readUInt16BE(0)
      ) {
        clearInterval(timer);
        log(`rx: ${res.toString('hex').match(/../g).join(' ')}`, 'debug');
        resolve(res);
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

const readSettings = async () => transmit(Buffer.from([0x04]));

const closePort = async () => {
  await writer.close();
  await writer.releaseLock();
  await reader.cancel();
  await reader.releaseLock();
  await port.close();
  isConnected.value = false;
};

onMounted(async () => {
  isSerialUnsupported.value = !navigator.serial;
});

</script>

<template>
  <n-card>
    <n-alert v-if="isSerialUnsupported" title="Please try Chrome" type="warning" style="margin-bottom: 2em;">
      This browser "{{ userAgent }}" does not support <a href="https://developer.mozilla.org/en-US/docs/Web/API/Serial" class="warning--text">Web Serial API</a> yet.
    </n-alert>
    <n-button type="primary" @click="openPort">Open Serial Port</n-button>
    <n-button v-if="isConnected" type="primary" @click="closePort">Disconnect</n-button>
    <n-button type="primary" :disabled="!isConnected" @click="readSettings">Read</n-button>
    <n-space v-for="servo, i in servos" :key="i" vertical>
      <n-space style="padding-bottom: 1em;">
        <n-tag size="large" style="width: 2.5em;">#{{ i }}</n-tag>
        <n-input-group>
          <n-input-number v-model:value="servo.deg0" :min="50" :max="servo.deg90" :style="{ width: '8em' }" />
          <n-button secondary type="primary" @click="servo.deg0 = servo.degree">0&deg;</n-button>
        </n-input-group>
        <n-input-group>
          <n-input-number v-model:value="servo.deg90" :min="servo.deg0" :max="servo.deg180" :style="{ width: '8em' }" />
          <n-button secondary type="primary" @click="servo.deg90 = servo.degree">90&deg;</n-button>
        </n-input-group>
        <n-input-group>
          <n-input-number v-model:value="servo.deg180" :min="servo.deg90" :max="600" :style="{ width: '8em' }" />
          <n-button secondary type="primary" @click="servo.deg180 = servo.degree">180&deg;</n-button>
        </n-input-group>
        <n-grid :cols="3" :x-gap="6">
          <n-gi style="padding-top: 0.5em;">
            <n-slider v-model:value="servo.degree" :min="50" :max="600" />
          </n-gi>
          <n-gi>
            <svg :width="250" :height="25" xmlns="http://www.w3.org/2000/svg">
              <!-- <circle :cx="center.x" :cy="center.y" :r="radius" fill="yellow" /> -->
              <!-- <line :x1="radius" :y1="radius" :x2="pointer.x" :y2="pointer.y" style="stroke:rgb(255,0,0);stroke-width:2" /> -->
              <line :x1="0" :y1="25" :x2="5" :y2="25" style="stroke:blue;" />
              <line :x1="5" :y1="0" :x2="servo.degree / 10 + 5" :y2="0" style="stroke:blue;" />
              <line :x1="servo.degree / 10 + 5" :y1="25" :x2="250" :y2="25" style="stroke:blue;" />
              <line :x1="5" :y1="0" :x2="5" :y2="25" style="stroke:blue;" />
              <line :x1="servo.degree / 10 + 5" :y1="0" :x2="servo.degree / 10 + 5" :y2="25" style="stroke:blue;" />
            </svg>
          </n-gi>
        </n-grid>
      </n-space>
    </n-space>

    <pre>{{ servos }}</pre>
  </n-card>
</template>
