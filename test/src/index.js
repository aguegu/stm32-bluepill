import chai from 'chai';
import 'chai/register-should';
import SerialPort from 'serialport';
import { promisify } from 'util';
import { serialportConf } from './fixings';

chai.use(require('chai-bytes'));

const range = (n) => ([...Array(n).keys()]);

const readBytesSync = (sp, txBuffer, n) => new Promise((resolve) => {
  let len = 0; const buffers = [];
  sp.on('data', function onRx(buff) {
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
  beforeEach(async function () {
    const { path, baudRate } = serialportConf;
    this.sp = new SerialPort(path, { baudRate, autoOpen: false });
    await promisify(this.sp.open).bind(this.sp)();
  });

  it('should get opened', function () {
    this.sp.isOpen.should.be.true;
  });

  it('should sequelize echo', async function () {
    const txBuffer = Buffer.alloc(3);
    txBuffer.writeUInt8(2, 0);

    // [...Array(256).keys()].forEach(i => {
    //   txBuffer.writeUInt8(i, 1);
    //   txBuffer.writeUInt8(0xff - i, 2);
    //   const rxBuffer = await readBytesSync(this.sp, txBuffer, 3);
    //   rxBuffer.should.equalBytes(txBuffer);
    // });
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

  it.skip('should be a mess', async function () {
    await Promise.all(range(256).map((i) => readBytesSync(this.sp, Buffer.from([2, i, 0xff - i]))));
  });

  afterEach(async function () {
    await promisify(this.sp.close).bind(this.sp)();
  });
});
