import serial
import time
from struct import pack, unpack
from servo_daemon import ServoDaemon
import easing


class OttoServo():
    def __init__(self, host, name, pin, home, minimun, maximun):
        self.host = host
        self.name = name
        self.pin = pin
        self.home = home
        self.minimun = minimun
        self.maximun = maximun

    def get_width(self, angle):
        if angle >= 90:
            return int(self.home + (angle - 90.) * self.maximun / 90.)
        return int(self.home + (90. - angle) * self.minimun / 90.)

    def SetPosition(self, angle, millis=0, func=easing.Linear):
        width = self.get_width(angle)
        self.host.curve(self.pin, width, millis * 0.001, func)

    def oscillate(self, amplitude, phase, millis=0):
        # print(amplitude, phase, millis)
        self.host.oscillate(self.pin, amplitude, phase, millis * 0.001)


def delay(millis):
    time.sleep(millis * 0.001)


if __name__ == '__main__':

    tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200)
    host = ServoDaemon(tty)
    servo = [
        OttoServo(host, 'RR', 3, 289, -172, 203),
        OttoServo(host, 'RL', 2, 321, -187, 188),
        OttoServo(host, 'YR', 1, 288, -178, 224),
        OttoServo(host, 'YL', 0, 291, -170, 185),
    ]
    host.start()
    host.connect()

    t = 495
    # tmp = 0.495

    def home():
        for x in servo:
            x.SetPosition(90)

    def goingUp(tempo):
        home()
        delay(tempo)
        servo[0].SetPosition(80)
        servo[1].SetPosition(100)
        delay(tempo)
        servo[0].SetPosition(70)
        servo[1].SetPosition(110)
        delay(tempo)
        servo[0].SetPosition(60)
        servo[1].SetPosition(120)
        delay(tempo)
        servo[0].SetPosition(50)
        servo[1].SetPosition(130)
        delay(tempo)
        servo[0].SetPosition(40)
        servo[1].SetPosition(140)
        delay(tempo)
        servo[0].SetPosition(30)
        servo[1].SetPosition(150)
        delay(tempo)
        servo[0].SetPosition(20)
        servo[1].SetPosition(160)
        delay(tempo)

    def kickLeft(tempo):
        home()
        delay(tempo)
        servo[0].SetPosition(50)
        servo[1].SetPosition(70)
        delay(tempo)
        servo[0].SetPosition(80)
        servo[1].SetPosition(70)
        delay(tempo/4)
        servo[0].SetPosition(30)
        servo[1].SetPosition(70)
        delay(tempo/4)
        servo[0].SetPosition(80)
        servo[1].SetPosition(70)
        delay(tempo/4)
        servo[0].SetPosition(30)
        servo[1].SetPosition(70)
        delay(tempo/4)
        servo[0].SetPosition(80)
        servo[1].SetPosition(70)
        delay(tempo)

    def kickRight(tempo):
        home();
        delay(tempo)
        servo[0].SetPosition(110)
        servo[1].SetPosition(130)
        delay(tempo)
        servo[0].SetPosition(110)
        servo[1].SetPosition(100)
        delay(tempo/4)
        servo[0].SetPosition(110)
        servo[1].SetPosition(150)
        delay(tempo/4)
        servo[0].SetPosition(110)
        servo[1].SetPosition(80)
        delay(tempo/4)
        servo[0].SetPosition(110)
        servo[1].SetPosition(150)
        delay(tempo/4)
        servo[0].SetPosition(110)
        servo[1].SetPosition(100)
        delay(tempo)

    def lateral_fuerte(side, tempo):
        home()
        if side:
            servo[0].SetPosition(40)
        else:
            servo[1].SetPosition(140)

        delay(tempo/2)

        servo[0].SetPosition(90)
        servo[1].SetPosition(90)
        delay(tempo/2)

    def moveNServos(ts, newPosition):
        for i, x in enumerate(newPosition):
            servo[i].SetPosition(x, ts)
        delay(ts)

    def noGravity(tempo):
        move1 = [120, 140, 90, 90]
        move2 = [140, 140, 90, 90]
        move3 = [120, 140, 90, 90]
        move4 = [90, 90, 90, 90]

        home()
        moveNServos(tempo*2, move1)
        moveNServos(tempo*2, move2)
        delay(tempo*2)
        moveNServos(tempo*2, move3)
        moveNServos(tempo*2, move4)

    def oscillate(amplitude, offset, tempo, phase):
        for i, x in enumerate(servo):
            x.SetPosition(90 + offset[i])
            x.oscillate(amplitude[i], phase[i], tempo)
        delay(tempo)

    def crusaito(steps, tempo):
        A = [25, 25, 30, 30]
        O = [- 15, 15, 0, 0]
        # phase_diff = [DEG2RAD(0), DEG2RAD(180 + 120), DEG2RAD(90), DEG2RAD(90)]
        phase_diff = [0] * 4
        for _ in range(steps):
            oscillate(A, O, t, phase_diff)

    def primera_parte():
        move1 = [60, 120, 90, 90]
        move2 = [90, 90, 90, 90]
        move3 = [40, 140, 90, 90]

        for _ in range(3):
            for _ in range(3):
                lateral_fuerte(1, t/2);
                lateral_fuerte(0, t/4);
                lateral_fuerte(1, t/4);



        # pause=millis();
        home()
        moveNServos(t*0.4, move1);
        moveNServos(t*0.4, move2);
        delay(t * 1.2)

        # while(millis()<(pause+t*2));
        # }
        #
        for _ in range(2):
            lateral_fuerte(1, t/2)
            lateral_fuerte(0, t/4)
            lateral_fuerte(1, t/4)
            delay(t)
        # }
        #
        # pause=millis();
        home()
        crusaito(1, t*1.4);
        moveNServos(t*1, move3)
        delay(2*t)
        home()
        # while(millis()<(pause+t*4));
        delay(2 * t)

    try:
        home()
        delay(t)
        # goingUp()
        # kickLeft(t)
        # kickRight(t)

        # primera_parte()
        #
        # noGravity(t)
        # crusaito(1, t*1.4)

        # servo[0].SetPosition(90-15)
        # servo[1].SetPosition(90+15)
        # servo[0].oscillate(25, 180, 2*t)
        # servo[1].oscillate(25, 0, 2*t)
        # delay(2*t)

        home()
    #     int A[4]= {15, 15, 30, 30}
    # int O[4] = {0, 0, 0, 0};
    # double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90)};
        while True:
            oscillate([15, 15, 30, 30], [0, 0, 0, 0], t, [0, 0, 90, 90])
            oscillate([15, 15, 30, 30], [0, 0, 0, 0], t, [180, 180, 270, 270])

        home()
        delay(t)

        # t.curve(15, 150, 0, easing.Linear)
        # time.sleep(1)
        # # while True:
        # t.curve(15, 450, 1.5, easing.BounceEaseOut)
        # time.sleep(1.6)
        # t.curve(15, 150, 1.5, easing.BounceEaseOut)
        # time.sleep(1.6)

    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        host.stop()
        host.join()
