import chai from 'chai';
import 'chai/register-should';
import SerialPort from 'serialport';
import { promisify } from 'util';
import MessageParser from './MessageParser';
import { serialportConf } from './fixings';

chai.use(require('chai-bytes'));

const range = (n) => ([...Array(n).keys()]);

const readBytesSync = (sp, txBuffer, n) => new Promise((resolve) => {
  let len = 0; const buffers = [];
  // console.log(txBuffer);
  sp.on('data', function onRx(buff) {
    // console.log('in data', buff);
    buffers.push(buff);
    len += buff.length;
    if (len >= n) {
      sp.off('data', onRx);
      resolve(Buffer.concat(buffers, n));
    }
  });
  sp.write(txBuffer);
});

describe('echo', () => {
  before(async function () {
    const { path, baudRate } = serialportConf;
    this.sp = new SerialPort(path, { baudRate, autoOpen: false });
    await promisify(this.sp.open).bind(this.sp)();
  });

  it('should get opened', function () {
    this.sp.isOpen.should.be.true;
  });

  it('should half-duplex echo', async function () {
    const txBuffer = Buffer.alloc(3);
    txBuffer.writeUInt8(2, 0);
    const { sp } = this;
    await range(256).reduce(async (last, i) => {
      await last;
      txBuffer.writeUInt8(i, 1);
      txBuffer.writeUInt8(0xff - i, 2);
      const rxBuffer = await readBytesSync(sp, txBuffer, 3);
      // console.log(txBuffer, rxBuffer);
      sp.listenerCount('data').should.equal(0);
      rxBuffer.should.equalBytes(txBuffer);
    }, Promise.resolve());
  });

  it('should full-duplex echo', function (done) {
    const parser = this.sp.pipe(new MessageParser());
    const count = 256;
    let i = 0;
    parser.on('data', (message) => {
      message.should.equalBytes([2, i, 0xff - i]);
      i += 1;
      if (i === count) {
        this.sp.unpipe(parser);
        parser.end();
        done();
      }
    });
    range(count).forEach((j) => this.sp.write(Buffer.from([2, j, 0xff - j])));
  });

  it('should full-duplex echo 2', function (done) {
    const parser = this.sp.pipe(new MessageParser());
    const count = 256;
    let i = 0;
    parser.on('data', (message) => {
      message.should.equalBytes([2, i, 0xff - i]);
      i += 1;
      if (i === count) {
        this.sp.unpipe(parser);
        parser.end();
        done();
      }
    });
    range(count).forEach((j) => this.sp.write(Buffer.from([2, j, 0xff - j])));
  });

  it.skip('should be a mess', async function () {
    const rxBuffers = await Promise.all(
      range(2).map((i) => readBytesSync(this.sp, Buffer.from([2, i, 0xff - i]), 3)),
    );
    rxBuffers.forEach((rxBuffer, i) => rxBuffer.should.equalBytes([2, i, 0xff - i]));
  });

  after(async function () {
    await promisify(this.sp.close).bind(this.sp)();
  });
});
