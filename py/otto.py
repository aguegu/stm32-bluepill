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

    def SetPosition(self, angle, seconds=0, func=easing.Linear):
        width = self.get_width(angle)
        self.host.curve(self.pin, width, seconds, func)

YL = 0
YR = 1
RL = 2
RR = 3

# servo[0].attach(PIN_RR);
# servo[1].attach(PIN_RL);
# servo[2].attach(PIN_YR);
# servo[3].attach(PIN_YL);

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

    def moveNServos(time, newPosition):
        for i, x in enumerate(newPosition):
            servo[i].SetPosition(x)
        delay(time)

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
        # crusaito(1, t*1.4);
        moveNServos(t*1, move3)
        delay(2*t)
        home()
        # while(millis()<(pause+t*4));
        delay(2 * t)

    try:
        home()
        # goingUp()
        kickLeft(t)
        kickRight(t)

        primera_parte()

        home()
        delay(1)

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

# tty = serial.Serial('/dev/tty.SLAB_USBtoUART', 115200)
# print(tty.name)
#
#
# def home():
#     for i in range(4):
#         send_width(i, homes[i])
#
# def set_angle(index, angle):
#     if angle >= 90:
#         send_width(index, int(homes[index] + (angle - 90.) * maximun[index] / 90.))
#     else:
#         send_width(index, int(homes[index] + (90. - angle) * minimun[index] / 90.))
#
#
# def send_width(index, width):
#     tty.write(pack('B', 4) + pack('B', index) + pack('<H', width))
#     print(tty.readline().decode())
#
#
# def set_all(lst):
#     for index, differ in enumerate(lst[:4]):
#         set_angle(index, 90 + differ)
#
#
# def updown():
#     set_all([0, 0, 45, -45])
#     time.sleep(2)
#     set_all([0, 0, 0, 0])
#     time.sleep(2)
#
#
# if __name__ == '__main__':
#
#     # set_angle(3, 0)
#     # time.sleep(4)
#     # set_angle(3, 180)
#     # time.sleep(4)
#
#     home()
#     tempo = 0.495;
#
#     # updown()
#     set_angle(3, 80)
#     set_angle(2, 100)
#     time.sleep(tempo)
#     set_angle(3, 70)
#     set_angle(2, 110)
#     time.sleep(tempo)
#     set_angle(3, 60)
#     set_angle(2, 120)
#     time.sleep(tempo)
#     set_angle(3, 50)
#     set_angle(2, 130)
#     time.sleep(tempo)
#     set_angle(3, 40)
#     set_angle(2, 140)
#     time.sleep(tempo)
#     set_angle(3, 30)
#     set_angle(2, 150)
#     time.sleep(tempo)
#     set_angle(3, 20)
#     set_angle(2, 160)
#     time.sleep(tempo)
#
#     home()
#
#     tty.close()
