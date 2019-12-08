/* eslint-disable no-underscore-dangle */

import { Transform } from 'stream';

class MessageParser extends Transform {
  constructor(options = {}) {
    super(options);
    this.buffer = Buffer.alloc(0);
  }

  _transform(chunk, encoding, cb) {
    let data = Buffer.concat([this.buffer, chunk]);
    while (data[0] > 1 && data.length >= data[0] + 1) {
      if (data[1] + data[2] === 0xff) {
        this.push(data.slice(0, data[0] + 1));
        data = data.slice(data[0] + 1);
      } else {
        data = data.slice(1);
      }
    }
    this.buffer = data;
    cb();
  }

  _flush(cb) {
    // this.push(this.buffer);
    this.buffer = Buffer.alloc(0);
    cb();
  }
}

export default MessageParser;
