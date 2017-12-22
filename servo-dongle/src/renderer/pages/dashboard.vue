<template lang="html">
  <main>
    <el-form ref="form" :model="form" label-width="40px">
      <el-form-item label="Port">
        <el-select v-model="form.port" style="width: calc(100% - 80px);" size="small">
          <el-option v-for="item in ports" :key="item.value" :label="item.comName" :value="item.comName">
          </el-option>
        </el-select>
        <el-button type="danger" size="small" @click.prevent="disconnect" v-if="isConnect">Close</el-button>
        <el-button type="primary" size="small" @click.prevent="connect" v-else>Open</el-button>
      </el-form-item>
      <el-form-item label="">
        <el-button type="primary" size="small" @click.prevent="send">Send</el-button>
      </el-form-item>
    </el-form>
    <pre>{{ $data }}</pre>
  </main>
</template>

<script>
import SerialPort from 'serialport';
import _ from 'lodash';

export default {
  data() {
    return {
      form: {
        port: '',
      },
      header: {
        uid: 0,
      },
      ports: [],
      isConnect: false,
      tty: null,
    };
  },
  methods: {
    hello() {
      const { app } = this.$electron.remote;
      this.form.name = app.hello().hello;
    },
    async listPort() {
      const ports = await SerialPort.list();
      this.ports = ports.filter(d => !_.isUndefined(d.locationId));
      if (this.ports.length) {
        this.form.port = this.ports[0].comName;
      }
    },
    connect() {
      // this.tty = new SerialPort(this.form.port, {
      //   baudRate: 115200,
      // });
      // this.tty.on('open', () => {
      //   this.isConnect = true;
      // });
      // this.tty.on('close', () => {
      //   this.isConnect = false;
      // });
      // this.tty.on('data', console.log); // eslint-disable-line no-console
      // this.tty.on('readable', () => {
      //   console.log('Data:', this.tty.read()); // eslint-disable-line no-console
      // });
    },
    disconnect() {
      this.tty.close();
      this.tty = null;
    },
    send() {
      const payload = Buffer.from([0x01, 0x00, 0x32, 0x01, 0x64, 0x00, 0x00]);
      const header = Buffer.alloc(3);
      header.writeUInt8(payload.length + 3, 0);
      header.writeUInt8(this.header.uid, 1);
      header.writeUInt8(0xff - this.header.uid, 2);
      const tx = Buffer.concat([header, payload]);
      this.tty.write(tx);
      console.log('Data:', this.tty.read()); // eslint-disable-line no-console
      this.header.uid += 1;
      this.header.uid %= 256;
    },
  },
  mounted() {
    this.hello();
    this.listPort();

    this.tty = new SerialPort('/dev/tty.SLAB_USBtoUART', {
      baudRate: 115200,
    });
    this.tty.on('open', () => {
      this.isConnect = true;
    });
    this.tty.on('close', () => {
      this.isConnect = false;
    });
    this.tty.on('data', console.log); // eslint-disable-line no-console
    this.tty.on('readable', () => {
      console.log('Data:', this.tty.read()); // eslint-disable-line no-console
    });
  },
};
</script>

<style lang="css">
</style>
