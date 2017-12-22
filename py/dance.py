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

        # servos[0].SetPosition(90 + 0)
        # servos[0].SetPosition(90 + 30)
        # servos[1].SetPosition(90 + 30)


        # while True:
        #
        #     for _ in range(4):
        #         dancer.moonWalkLeft(t*2)
        #     for _ in range(4):
        #         dancer.moonWalkRight(t*2)

            # dancer.oscillate([25, 25, 0, 0], [-15, 15, 0, 0], t, [0, 60, 90, 90])
            # dancer.oscillate([25, 25, 0, 0], [-15, 15, 0, 0], t, [0, 300, 90, 90])

        # dancer.home()
        delay(t * 20)

    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        dancer.home()
        host.stop()
        host.join()
