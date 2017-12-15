import threading
from datetime import datetime


def toHex(bs):
    return ' '.join('%02x' % c for c in bs)


class SerialDaemon(threading.Thread):
    def __init__(self, serial_instance, protocol_factory):
        super(SerialDaemon, self).__init__()
        self.daemon = True
        self.serial = serial_instance
        self.protocol_factory = protocol_factory
        self.alive = True
        self._lock = threading.Lock()
        self._connection_made = threading.Event()
        self.protocol = None
        self.uid = 0

    def stop(self):
        self.alive = False
        self.join(2)

    def run(self):
        self.serial.timeout = 1
        self.protocol = self.protocol_factory()
        self.protocol.connection_made(self)

        error = None
        self._connection_made.set()
        while self.alive and self.serial.is_open:
            try:
                data = self.serial.read(self.serial.in_waiting or 1)
            except serial.SerialException as e:
                error = e
                break
            else:
                if data:
                    self.protocol.data_received(data)

        self.alive = False
        self.protocol.connection_lost(error)
        self.protocol = None

    def write(self, data):
        with self._lock:
            tx = bytes([len(data) + 2, self.uid, 0xff - self.uid]) + data
            # print('tx:', toHex(tx), datetime.now())
            print('tx:', toHex(tx))
            self.serial.write(tx)
            self.uid += 1
            self.uid &= 0xff

    def close(self):
        with self._lock:
            self.stop()
            self.serial.close()

    def connect(self):
        if self.alive:
            self._connection_made.wait()
            if not self.alive:
                raise RuntimeError('connection_lost already called')
            return (self, self.protocol)
        else:
            raise RuntimeError('already stopped')

    def __enter__(self):    # with
        self.start()
        self._connection_made.wait()
        if not self.alive:
            raise RuntimeError('connection_lost already called')
        return self.protocol

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
