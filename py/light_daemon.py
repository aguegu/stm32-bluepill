import serial
from serial.threaded import Protocol
from struct import unpack, pack
import time
import threading
from functools import reduce


def move(index, width, span, curve=0):
    return pack('<BHHB', index, width, span, curve)


def oscillate(index, amplitude, span, phase=0):
    return pack('<BhHh', index, amplitude, span, phase)


def toHex(bs):
    return ' '.join('0x%02x' % c for c in bs)


CURVES = [
    'Linear',
    'SineIn',
    'SineOut',
    'SineInOut',
    'QuadIn',
    'QuadOut',
    'QuadInOut',
    'CubicIn',
    'CubicOut',
    'CubicInOut',
    'QuarticIn',
    'QuarticOut',
    'QuarticInOut',
    'ExponentialIn',
    'ExponentialOut',
    'ExponentialInOut',
    'CircularIn',
    'CircularOut',
    'CircularInOut',
    'BackIn',
    'BackOut',
    'BackInOut',
    'ElasticIn',
    'ElasticOut',
    'ElasticInOut',
    'BounceIn',
    'BounceOut',
    'BounceInOut',
]

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

curves = [
    'Linear',
    'SineIn',
    'SineOut',
    'SineInOut',
    'QuadIn',
    'QuadOut',
    'QuadInOut',
    'CubicIn',
    'CubicOut',
    'CubicInOut',
    'QuarticIn',
    'QuarticOut',
    'QuarticInOut',
    'ExponentialIn',
    'ExponentialOut',
    'ExponentialInOut',
    'CircularIn',
    'CircularOut',
    'CircularInOut',
    'BackIn',
    'BackOut',
    'BackInOut',
    'ElasticIn',
    'ElasticOut',
    'ElasticInOut',
    'BounceIn',
    'BounceOut',
    'BounceInOut',
]


class OttoServo():
    def __init__(self, host, name, pin, home):
        self.host = host
        self.name = name
        self.pin = pin
        self.home = home

    def get_width(self, angle):
        return round(self.home + (angle - 90) * 34.0 / 15.)

    def SetPosition(self, angle, millis=0, curve=0):
        width = self.get_width(angle)
        self.host.write(bytes([0x01]) + move(self.pin, width, 1, curve))

    def oscillate(self, amplitude, phase, millis=0):
        # print(amplitude, phase, millis)
        # self.host.oscillate(self.pin, amplitude, phase, millis * 0.001)
        self.host.write(bytes([0x02]) + move(self.pin, width, 1, curve))


def home(servos):
    for x in servos:
        x.SetPosition(90)
    time.sleep(0.01)


def delay(millis):
    time.sleep(millis * 0.001)


if __name__ == '__main__':
    try:
        tty = serial.Serial('/dev/cu.SLAB_USBtoUART', 115200)
        t = BusDaemon(tty, RecvProtocol)
        t.start()
        t.connect()

        servos = [
            OttoServo(t, 'RR', 3, 289),
            OttoServo(t, 'RL', 2, 315),
            OttoServo(t, 'YR', 1, 282),
            OttoServo(t, 'YL', 0, 291),
        ]

        func = bytes([0x01])

        # t.write(func + move(0, 374, 1))
        # time.sleep(0.01)

        curve = CURVES.index('BounceOut')
        s = 2

        tempo = 485
        # home(servos)

        # self.home()
        # delay(tempo)
        # servos[0].SetPosition(80)
        # servos[1].SetPosition(100)
        # delay(tempo)
        # servos[0].SetPosition(70)
        # servos[1].SetPosition(110)
        # delay(tempo)
        # servos[0].SetPosition(60)
        # servos[1].SetPosition(120)
        # delay(tempo)
        # servos[0].SetPosition(50)
        # servos[1].SetPosition(130)
        # delay(tempo)
        # servos[0].SetPosition(40)
        # servos[1].SetPosition(140)
        # delay(tempo)
        # servos[0].SetPosition(30)
        # servos[1].SetPosition(150)
        # delay(tempo)
        # servos[0].SetPosition(20)
        # servos[1].SetPosition(160)
        # delay(tempo)

        # home(servos)
        # time.sleep(0.01)

        # t.write(bytes([0x02]) + oscillate(0, 30, s*100))
        # time.sleep(s)

        # t.write(bytes([0x02]) + oscillate(0, 60, s*100) + oscillate(1, -60, s*100))
        # time.sleep(s)

        while True:
            # payload = [move(i, 170, s * 100, curve) for i in range(16)]
            # t.write(func + reduce(lambda c, x: c + x, payload, bytearray()))
            # time.sleep(s)
            # payload = [move(i, 442, s * 100, curve) for i in range(16)]
            # t.write(func + reduce(lambda c, x: c + x, payload, bytearray()))
            # time.sleep(s)
            payload = [oscillate(i, i * 17, s * 100) for i in range(16)]
            t.write(reduce(lambda c, x: c + x, payload, bytearray.fromhex('02')))
            time.sleep(s)
            # break


        # t.write(func + move(0, 450, 100) + move(15, 150, 100))
        # time.sleep(1)
    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.stop()
        t.join()
