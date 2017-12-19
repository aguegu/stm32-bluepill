import serial
from struct import unpack
import time

tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 9600)

while True:
    tty.write(bytes([0x55]))
    distance = tty.read(2)
    print(distance, unpack('>H', distance))
    time.sleep(1)
