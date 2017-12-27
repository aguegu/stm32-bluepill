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
        # print('rx:', toHex(packet), datetime.now())
        print('rx:', toHex(packet))


def at24c(slave_address, extra_wait, rxlength, txbuff):
    return bytes([slave_address, extra_wait, rxlength]) + bytes(txbuff)


if __name__ == '__main__':
    tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200, timeout=1)
    print(tty.name)

    t = SerialDaemon(tty, FramedPacket)
    t.start()
    t.connect()

    try:
        # while True:
        t.write(at24c(0x57, 0, 128, [0, 0]))
        # t.write(at24c(0x57, 5, 0, [0xc] + [0] * 7))
        #
        # t.write(at24c(0x57, 0, 128, [0]))
        # t.write(at24c(0x57, 5, 0, [0] + [0xff] * 128))
        #
        # t.write(at24c(0x57, 5, 0, [0] + list(range(16))))
        # t.write(at24c(0x57, 0, 16, [0]))

        # time.sleep(1)
        #
        # txbuff = bytes([0x50, 5, 0]) #+ bytes([0x00])
        # t.write(txbuff)
        # time.sleep(1)
        #
        # txbuff = bytes([0x50, 0, 0]) + bytes([0x12, 0x00])
        # t.write(txbuff)
        # time.sleep(1)
        #
        # txbuff = bytes([0x50, 5, 0]) #+ bytes([0x00])
        # t.write(txbuff)
        # time.sleep(1)
        #
        # txbuff = bytes([0x50, 0, 1]) + bytes([0x34, 0x00])
        # t.write(txbuff)
        # time.sleep(1)
        # #
        # txbuff = bytes([0x50, 5, 0]) #+ bytes([0x00])
        # t.write(txbuff)
        # time.sleep(1)
        # t.write(bytes([0x50, 0, 0]) + bytes(range(1, 20)))
        # time.sleep(1)
        t.write(at24c(0x50, 0, 128, [0]))
        time.sleep(1)

        # txbuff = bytes([0x50, 8, 18]) #+ bytes([0x00])
        # t.write(txbuff)
        # time.sleep(1)


        # txbuff = bytes([0x68, 2, 0]) #+ bytes([0x00])
        # t.write(txbuff)
        # time.sleep(1)


    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.close()
