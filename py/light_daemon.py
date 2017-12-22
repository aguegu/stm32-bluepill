import serial
from serial.threaded import Protocol
from struct import unpack, pack
import time
import threading


def move(index, width, span, curve=0):
    return pack('<BHHB', index, width, span, curve)


def toHex(bs):
    return ' '.join('0x%02x' % c for c in bs)


class RecvProtocol(Protocol):
    def __init__(self):
        self.packet = bytearray()
        self.in_packet = False
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport

    def connection_lost(self, exc):
        self.transport = None
        self.in_packet = False
        self.pointer = 0
        self.length = 0
        del self.packet[:]
        super(RecvProtocol, self).connection_lost(exc)

    def data_received(self, data):
        # print(toHex(data))
        for byte in serial.iterbytes(data):
            if self.in_packet:
                self.packet.extend(byte)
                self.pointer += 1
            else:
                self.length = unpack('B', byte)[0]
                self.packet.extend(byte)
                self.pointer = 0
                self.in_packet = True

            if self.pointer == self.length:
                self.in_packet = False
                self.handle_packet(bytes(self.packet))  # make read-only copy
                del self.packet[:]

    def handle_packet(self, packet):
        if packet[3]:
            print(packet[3])
        # print('rx:', toHex(packet), datetime.now())
        print('rx:', toHex(packet))


class BusDaemon(threading.Thread):
    def __init__(self, tty, recv_protocol):
        super(BusDaemon, self).__init__()
        self.tty = tty
        self.receiver = recv_protocol()
        self.alive = False
        self._lock = threading.Lock()
        self._connected = threading.Event()
        self.uid = 0

    def stop(self):
        self.alive = False

    def run(self):
        self.tty.timeout = 1
        self._connected.wait()

        error = None
        while self.alive and self.tty.is_open:
            try:
                data = self.tty.read(self.tty.in_waiting or 1)
            except serial.SerialException as e:
                error = e
                break
            else:
                if data:
                    self.receiver.data_received(data)

        self.alive = False
        self.receiver.connection_lost(error)
        self.receiver = None
        self.tty.close()

    def connect(self):
        self.receiver.connection_made(self)
        self.alive = True
        self._connected.set()

    def write(self, data):
        with self._lock:
            tx = bytes([len(data) + 2, self.uid, 0xff - self.uid]) + data
            # print('tx:', toHex(tx), datetime.now())
            print('tx:', toHex(tx))
            self.tty.write(tx)
            self.uid += 1
            self.uid &= 0xff


if __name__ == '__main__':
    try:
        tty = serial.Serial('/dev/cu.SLAB_USBtoUART', 115200)
        t = BusDaemon(tty, RecvProtocol)
        t.start()
        t.connect()

        func = bytes([0x01])

        t.write(func + move(0, 306, 100))
        t.write(func + move(1, 306, 100))
        t.write(func + move(2, 306, 100))
        t.write(func + move(3, 306, 100))

        t.write(func + move(4, 102, 100))
        t.write(func + move(5, 102, 100))
        t.write(func + move(6, 102, 100))
        t.write(func + move(7, 102, 100))

        t.write(func + move(8, 510, 100))
        t.write(func + move(9, 510, 100))
        t.write(func + move(10, 510, 100))
        t.write(func + move(11, 510, 100))

        t.write(func + move(12, 500, 100))
        t.write(func + move(13, 500, 100))
        t.write(func + move(14, 500, 100))
        t.write(func + move(15, 500, 100))

        # while True:


        # t.write(func + move(0, 450, 100) + move(15, 150, 100))
        time.sleep(1)
    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.stop()
        t.join()
