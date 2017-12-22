import threading
from serial.threaded import Protocol
import time
import serial
from struct import unpack
import easing
from datetime import datetime
from functools import reduce
import math


def toHex(bs):
    return ' '.join('%02x' % c for c in bs)


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
        # print('rx:', toHex(packet))


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
            # print('tx:', toHex(tx))
            self.tty.write(tx)
            self.uid += 1
            self.uid &= 0xff


class Servo():
    def __init__(self, position):
        self.position = position
        self.span = 0
        self.func = None
        self.step = 0

    def update(self):
        if self.step < self.span:
            self.position = round(self.func(self.step + 1))
            self.step += 1
            # print(self.position)
            if self.position < 150:
                raise ValueError

        if self.step >= self.span:
            self.func = None
            self.step = 0
            self.span = 0

    def curve(self, to, span, func):
        self.func = func(self.position, to, span).ease
        self.span = span

    def oscillate(self, amplitude, phase, span):
        offset = self.position - amplitude * math.sin(phase * math.pi / 180)
        # print(amplitude, phase, span)
        self.func = lambda step: offset + amplitude * math.sin(step * 2 * math.pi / span + phase * math.pi / 180)
        self.span = span


class ServoDaemon(threading.Thread):
    def __init__(self, tty, length=16, freq=100, downtime=0.002):
        super(ServoDaemon, self).__init__()
        self.length = length
        self.servos = [Servo(300) for _ in range(self.length)]
        self.alive = True
        self.bus = BusDaemon(tty, RecvProtocol)
        self.freq = freq
        self.sleep = 1.0 / self.freq - downtime
        self._connected = threading.Event()

    def update(self):
        for x in self.servos:
            x.update()

    def connect(self):
        self.bus.start()
        self.bus.connect()
        self.bus.write(bytes([0x40, 0, 0] + [0, 0x31]))
        self.bus.write(bytes([0x40, 0, 0] + [0xfe, 136]))
        self.bus.write(bytes([0x40, 0, 0] + [0, 0x21]))
        self.transmit()
        time.sleep(1)
        self._connected.set()

    def transmit(self):
        vals = [[0, 0, x.position & 0xff, x.position >> 8] for x in self.servos]
        payload = reduce(lambda c, x: c + x, vals, [])
        self.bus.write(bytes([0x40, 0, 0] + [6] + payload))

    def run(self):
        self._connected.wait()

        while self.alive:
            if any([x.span for x in self.servos]):
                # print(' '.join('%d/%d' % (x.step, x.span) for x in self.servos))
                self.update()
                self.transmit()
                # print(datetime.now().strftime('%H:%M:%S.%f'), ' '.join(['%03d' % x.position for x in self.servos]))
            time.sleep(self.sleep)

    def stop(self):
        self.alive = False
        self.bus.stop()
        self.bus.join()

    def curve(self, index, to, seconds=0, func=easing.Linear):
        if seconds == 0:
            self.servos[index].curve(to, 1, func)
        else:
            self.servos[index].curve(to, round(seconds * self.freq), func)

    def oscillate(self, index, amplitude, phase, seconds=0):
        # print('servo Daemon', index, amplitude, phase, seconds)
        if seconds == 0:
            return
        else:
            self.servos[index].oscillate(amplitude, phase, round(seconds * self.freq))


if __name__ == '__main__':
    try:
        tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200)
        t = ServoDaemon(tty)
        t.start()
        t.connect()
        t.curve(15, 150, 0)
        time.sleep(1)
        # while True:
        t.curve(15, 450, 1.5)
        time.sleep(1.6)
        t.curve(15, 150, 1.5)
        time.sleep(1.6)

    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.stop()
        t.join()
