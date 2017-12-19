import serial
import time
from struct import pack, unpack
from servo_daemon import ServoDaemon
from otto import OttoServo, Otto, delay


if __name__ == '__main__':

    tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200)
    host = ServoDaemon(tty)
    servos = [
        OttoServo(host, 'RR', 3, 289, -172, 203),
        OttoServo(host, 'RL', 2, 321, -187, 188),
        OttoServo(host, 'YR', 1, 288, -178, 224),
        OttoServo(host, 'YL', 0, 291, -170, 185),
    ]
    host.start()
    host.connect()

    t = 495
    dancer = Otto(servos, t)

    try:
        dancer.home()
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
