import serial
import time
from struct import pack, unpack


YL = 0
YR = 1
RL = 2
RR = 3

# RL = 0
# YL = 1
# YR = 2
# RR = 3
# homes = [321, 291, 288, 289]   # 90 degree
# minimun = [-187, -170, -178, -172]
# maximun = [188, 185, 224, 204]

homes = [291, 288, 321, 289]   # 90 degree
minimun = [-170, -178, -187, -172]
maximun = [185, 224, 188, 203]



tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200, timeout=1)
print(tty.name)


def home():
    for i in range(4):
        send_width(i, homes[i])

def set_angle(index, angle):
    if angle >= 90:
        send_width(index, int(homes[index] + (angle - 90.) * maximun[index] / 90.))
    else:
        send_width(index, int(homes[index] + (90. - angle) * minimun[index] / 90.))


def send_width(index, width):
    tty.write(pack('B', 4) + pack('B', index) + pack('<H', width))
    print(tty.readline().decode())


def set_all(lst):
    for index, differ in enumerate(lst[:4]):
        set_angle(index, 90 + differ)


def updown():
    set_all([0, 0, 45, -45])
    time.sleep(2)
    set_all([0, 0, 0, 0])
    time.sleep(2)


if __name__ == '__main__':

    # set_angle(3, 0)
    # time.sleep(4)
    # set_angle(3, 180)
    # time.sleep(4)

    home()
    tempo = 0.495;

    # updown()
    set_angle(3, 80)
    set_angle(2, 100)
    time.sleep(tempo)
    set_angle(3, 70)
    set_angle(2, 110)
    time.sleep(tempo)
    set_angle(3, 60)
    set_angle(2, 120)
    time.sleep(tempo)
    set_angle(3, 50)
    set_angle(2, 130)
    time.sleep(tempo)
    set_angle(3, 40)
    set_angle(2, 140)
    time.sleep(tempo)
    set_angle(3, 30)
    set_angle(2, 150)
    time.sleep(tempo)
    set_angle(3, 20)
    set_angle(2, 160)
    time.sleep(tempo)

    home()

    tty.close()
