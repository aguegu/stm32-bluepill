import chai from 'chai'; // eslint-disable-line no-unused-vars
import 'chai/register-should';
import SerialPort from 'serialport';
import { promisify } from 'util';
import { serialportConf } from './fixings';

describe('echo', () => {
  before(async function () {
    const { path, baudRate } = serialportConf;
    this.sp = new SerialPort(path, { baudRate, autoOpen: false });
    await promisify(this.sp.open).bind(this.sp)();
  });

  it('should get opened', function () {
    this.sp.isOpen.should.be.true;
  });

  after(async function () {
    await promisify(this.sp.close).bind(this.sp)();
  });
});
