import serial
from serial.threaded import Protocol
from datetime import datetime
import threading
import time
from struct import pack, unpack

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
        rx_at = datetime.now()
        print('rx:', toHex(packet), rx_at)


if __name__ == '__main__':
    tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200, timeout=1)
    print(tty.name)

    t = SerialDaemon(tty, FramedPacket)
    t.start()
    t.connect()

    counter = 0
    try:
        while counter < 4:
            t.write(bytes([counter & 0xff] * counter))
            counter += 1
            time.sleep(0.01)
        time.sleep(3)
    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.close()
