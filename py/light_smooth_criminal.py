import serial
import time
from struct import unpack, pack
from light_otto import Otto, OttoServo, delay
from light_daemon import BusDaemon, RecvProtocol


if __name__ == '__main__':
    try:
        tty = serial.Serial('/dev/cu.SLAB_USBtoUART', 115200)
        host = BusDaemon(tty, RecvProtocol)
        host.start()
        host.connect()

        rx = host.write(bytes([0x04]), True)
        data = unpack('<' + 'H' * 16 * 4, rx[4:])
        mids = [data[i * 4 + 1] for i in range(4)]

        servos = [
            OttoServo(host, 'YL', 0, mids[0]),
            OttoServo(host, 'YR', 1, mids[1]),
            OttoServo(host, 'RL', 2, mids[2]),
            OttoServo(host, 'RR', 3, mids[3]),
        ]

        t = 495

        dancer = Otto(servos, t)

        delay(t)

        dancer.primera_parte()
        dancer.segunda_parte()

        for _ in range(4):
            dancer.moonWalkLeft(t*2)

        for _ in range(4):
            dancer.moonWalkRight(t*2)

        for _ in range(4):
            dancer.moonWalkLeft(t*2)

        for _ in range(4):
            dancer.moonWalkRight(t*2)

        dancer.primera_parte()
        dancer.crusaito(t*8)
        dancer.crusaito(t*7)

        for _ in range(16):
            dancer.flapping(t / 4)
            delay(3*t/4)

        for _ in range(4):
            dancer.moonWalkRight(t * 2)

        for _ in range(4):
            dancer.moonWalkLeft(t * 2)

        for _ in range(4):
            dancer.moonWalkRight(t * 2)

        for _ in range(4):
            dancer.moonWalkLeft(t * 2)

        dancer.drunk(t * 4)
        dancer.drunk(t * 4)
        dancer.drunk(t * 4)
        dancer.drunk(t * 4)
        dancer.kickLeft(t)
        dancer.kickRight(t)
        dancer.drunk(t * 8)
        dancer.drunk(t * 4)
        dancer.drunk(t / 2)

        delay(t * 4)

        dancer.drunk(t/2)

        delay(t * 4)
        dancer.walk(t * 2)
        dancer.walk(t * 2)
        dancer.backyard(t * 2)
        dancer.backyard(t * 2)
        dancer.goingUp(t * 2)
        dancer.goingUp(t * 1)
        dancer.noGravity(t * 2)
        dancer.crusaito(t * 2)
        dancer.crusaito(t * 8)
        dancer.crusaito(t * 2)
        dancer.crusaito(t * 8)
        dancer.crusaito(t * 2)
        dancer.crusaito(t * 3)

        delay(t)

        dancer.primera_parte()
        for _ in range(32):
            dancer.flapping(t / 2)
            delay(t / 2)

        dancer.home()
    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        host.stop()
        host.join()
