import serial
from serial.threaded import Protocol
from datetime import datetime, timedelta
import threading
import time
from struct import pack, unpack
import easing

from serialdaemon import SerialDaemon, toHex


class FramedPacket(Protocol):
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
        super(FramedPacket, self).connection_lost(exc)

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
        # pass
        # print('rx:', toHex(packet), datetime.now())
        print('rx:', toHex(packet))


# def at24c(slave_address, extra_wait, rxlength, txbuff):
#     return bytes([slave_address, extra_wait, rxlength]) + bytes(txbuff)

homes = [291, 288, 321, 289]   # 90 degree
minimun = [-170, -178, -187, -172]
maximun = [185, 224, 188, 203]

if __name__ == '__main__':
    tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200, timeout=1)
    positions = [300] * 16

    t = SerialDaemon(tty, FramedPacket)
    t.start()
    t.connect()

    def init():
        t.write(bytes([0x40, 0, 0] + [0, 0x31]))
        t.write(bytes([0x40, 0, 0] + [0xfe, 136]))
        t.write(bytes([0x40, 0, 0] + [0, 0x21]))

    def middle(width=300):
        for i in range(16):
            t.write(bytes([0x40, 0, 0] + [6 + 4 * i, 0, 0, width & 0xff, width >> 8]))
            positions[i] = width

    def width(i, width):
        t.write(bytes([0x40, 0, 0] + [8 + 4 * i, width & 0xff, width >> 8]))
        positions[i] = width

    def all(width):
        t.write(bytes([0x40, 0, 0] + [6] + [0, 0, width & 0xff, width >> 8] * 16))
        for i in range(16):
            positions[i] = width

    def home():
        for i, v in enumerate(homes):
            width(i, v)

    def angle(index, angle):
        if angle >= 90:
            width(index, int(homes[index] + (angle - 90.) * maximun[index] / 90.))
        else:
            width(index, int(homes[index] + (90. - angle) * minimun[index] / 90.))

    def move(index, w, span):
        steps = [x / 100 for x in range(0, int(span * 100))]
        print(steps)

        curve = easing.BounceEaseOut(positions[index], w, span)
        raw = list(map(curve.ease, steps))
        print(raw)
        result = list(map(round, raw))
        print(result)
        for x in result[:-1]:
            width(index, x)
            time.sleep(0.010 - 0.0001)

        width(index, w)
    try:
        init()
        # home()
        # width(15, 150)
        s1 = datetime.now()

        all(150)

        s2 = datetime.now()
        print(s2-s1)
        time.sleep(1)
        # tempo = 0.495;
        i = 300
        step = 1
        k = 0

        start = datetime.now()
        end = start + timedelta(seconds=1)

        # while datetime.now() < end:
        #     k += 1
        #     # width(15, i)
        #     # all(i)
        #     i += step
        #     if i > 450 or i < 150:
        #         step = -step
        #     # time.sleep(0.025)
        #     time.sleep(0.050 - 0.0001)
        # print(k)
        while True:
            move(15, 450, 2)
            time.sleep(1)
            move(15, 150, 2)
            time.sleep(1)


        # updown()
        # angle(3, 80)
        # angle(2, 100)
        # time.sleep(tempo)
        # angle(3, 70)
        # angle(2, 110)
        # time.sleep(tempo)
        # angle(3, 60)
        # angle(2, 120)
        # time.sleep(tempo)
        # angle(3, 50)
        # angle(2, 130)
        # time.sleep(tempo)
        # angle(3, 40)
        # angle(2, 140)
        # time.sleep(tempo)
        # angle(3, 30)
        # angle(2, 150)
        # time.sleep(tempo)
        # angle(3, 20)
        # angle(2, 160)
        # time.sleep(tempo)

        # home()
        # all(300)

        time.sleep(1)
    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.close()
