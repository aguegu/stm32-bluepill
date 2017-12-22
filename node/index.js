import SerialPort from 'serialport';

const command = (index, width, span, curve=0) => {
  const payload = Buffer.alloc(6);

  payload.writeUInt8(index, 0);
  payload.writeUInt16LE(width, 1);
  payload.writeUInt16LE(span, 3);
  payload.writeUInt8(curve, 5);

  return payload;
}

const main = async () => {
  const ports = await SerialPort.list();
  console.log(ports);

  const tty = new SerialPort('/dev/tty.SLAB_USBtoUART', { baudRate: 115200 });

  tty.on('data', console.log);

  tty.on('open', () => {
    let uid = 0;

    const func = 1;

    const payload = command(2, 206, 100);
    const header = Buffer.alloc(4);
    header.writeUInt8(payload.length + 2, 0);
    header.writeUInt8(uid, 1);
    header.writeUInt8(0xff - uid, 2);
    header.writeUInt8(func, 3);
    const tx = Buffer.concat([header, payload]);
    console.log(tx);
    tty.write(tx);

    setTimeout(() => {
      tty.off('data');
      tty.close();
    }, 1000);
  });
};




main();
