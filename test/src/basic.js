import chai from 'chai'; // eslint-disable-line no-unused-vars
import 'chai/register-should';
import SerialPort from 'serialport';
import { serialportConf } from './fixings';

describe.skip('list', () => {
  it('should list ports', async () => {
    const ports = await SerialPort.list();
    const { path } = serialportConf;
    ports.map((d) => d.path).should.include(path);
  });
});
