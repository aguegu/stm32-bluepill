import serial
from serial.threaded import Protocol
from struct import unpack, pack
import time
import threading
import queue
from functools import reduce
from Crypto.Cipher import AES


def move(index, width, span, curve=0):
    return pack('<BHHB', index, width, span, curve)


def oscillate(index, amplitude, span, phase=0):
    return pack('<BhHh', index, amplitude, span, phase)


def initialize(index, init, mid, minimum, maximum):
    return pack('<BHHHH', index, init, mid, minimum, maximum)


def toHex(bs):
    return ' '.join('0x%02x' % c for c in bs)


SECRET = bytes([0x02, 0xc8, 0x69, 0x40, 0xec, 0x17, 0xe0, 0xf8, 0xbd, 0xaa, 0xfd, 0x2b, 0xa4, 0x1c, 0xa8, 0x78])
POSTFIX = [
    0x00000000,
    0x9868ee46
]

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
        # with self.transport.cache_lock:
        #     self.transport.cache = bytes(packet)
        self.transport.q.put(bytes(packet))
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
        self.cache = bytes()
        self.uid_mark = None
        # self._sync = threading.Event()
        self._package_received = threading.Event()
        self._package_synced = threading.Event()

        # self.rx = bytes()
        # self.cache_lock = threading.Lock()
        # self.packet_received = threading.Event()
        self.q = queue.Queue()

    def stop(self):
        self.alive = False

    def run(self):
        self.tty.timeout = 0.1
        self._connected.wait()

        error = None
        while (self.alive) and self.tty.is_open:
            try:
                while True:
                    self.cache = self.q.get_nowait()
                    if self.uid_mark is not None and self.cache[1] == self.uid_mark:
                        self._package_received.set()
                        self._package_synced.wait()
                        self._package_synced.clear()
                    self.q.task_done()
            except queue.Empty:
                pass

            try:
                data = self.tty.read(self.tty.in_waiting or 1)
            except serial.SerialException as e:
                error = e
                break
            else:
                if data: # bytes([0]), b'0' is True too
                    self.receiver.data_received(data)

        self.alive = False
        self.receiver.connection_lost(error)
        self.receiver = None
        self.tty.close()

    def connect(self):
        self.receiver.connection_made(self)
        self.alive = True
        self._connected.set()

    def write(self, data, sync=False):
        tx = bytes([len(data) + 2, self.uid, 0xff - self.uid]) + data
        # print('tx:', toHex(tx), datetime.now())
        print('tx:', toHex(tx))
        with self._lock:
            self.tty.write(tx)

        uid = self.uid
        self.uid += 1
        self.uid &= 0xff

        if sync:
            self.uid_mark = uid

            self._package_received.wait()
            self._package_received.clear()

            rx = bytes(self.cache)
            self.uid_mark = None
            self._package_synced.set()

            return rx
        return uid


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


# def home(servos):
#     for x in servos:
#         x.SetPosition(90)
#     time.sleep(0.01)
#
#
# def delay(millis):
#     time.sleep(millis * 0.001)


# def moveup():
    # tempo = 485
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


if __name__ == '__main__':
    try:
        tty = serial.Serial('/dev/cu.SLAB_USBtoUART', 115200)
        t = BusDaemon(tty, RecvProtocol)
        t.start()
        t.connect()

        licensed = t.write(bytes([0xf3]), True)[-2]
        if not licensed:
        # if True:
            version = t.write(bytes([0xf3]), True)[-1]
            sid = t.write(bytes([0xf0]), True)[4:]
            cipher = AES.new(SECRET)
            license = cipher.encrypt(sid + pack('<I', POSTFIX[version]))
            # license = bytes(range(16))    # fake license
            t.write(bytes([0xf2]) + license + bytes([sum(license) & 0xff]), True)
            time.sleep(0.1) # for reset
            t.write(bytes([0xf3]), True)


        # init range
        # payload = [initialize(i, 306, 306, 50, 600) for i in range(16)]
        # t.write(reduce(lambda c, x: c + x, payload, bytearray.fromhex('03')))


        # t.write(bytes([0x02]) + oscillate(0, 30, s*100))
        # time.sleep(s)

        # t.write(bytes([0x02]) + oscillate(0, 60, s*100) + oscillate(1, -60, s*100))
        # time.sleep(s)

        while True:
            # payload = [move(i, 170, s * 100, curve) for i in range(16)]
            # t.write(func + reduce(lambda c, x: c + x, payload, bytearray()))
            #
            # payload = [move(i, 442, s * 100, curve) for i in range(16)]
            # t.write(func + reduce(lambda c, x: c + x, payload, bytearray()))
            #
            # payload = [oscillate(i, i * 17, s * 100) for i in range(16)]
            # t.write(reduce(lambda c, x: c + x, payload, bytearray.fromhex('02')))

            # rx = t.write(bytes([0xf1]), True)

            # print(uid)
            # t.write(bytes([0xf2]) + bytes([0xbe] * 16), True)
            # t.write(bytes([0xf1]))
            # time.sleep(s)
            break


        # t.write(func + move(0, 450, 100) + move(15, 150, 100))
        # time.sleep(1)
        time.sleep(0.01)
    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.stop()
        t.join()
