from struct import unpack, pack
import time


def delay(millis):
    time.sleep(millis * 0.001)


class OttoServo():
    def __init__(self, host, name, pin, home=306):
        self.host = host
        self.name = name
        self.pin = pin
        self.home = home

    def get_width(self, angle):
        return round(self.home + (angle - 90) * 2)

    def SetPosition(self, angle, millis=0, curve=0):
        # width = self.get_width(angle)
        # self.host.write(bytes([0x01]) + OttoServo.move(self.pin, width, max(1, int(millis / 10)), curve))
        self.host.write(bytes([0x05]) + OttoServo.move(self.pin, angle, max(1, int(millis / 10)), curve))

    def oscillate(self, amplitude, millis=0, phase=0):
        # print(amplitude, phase, millis)
        # payload = OttoServo.oscil(self.pin, int(amplitude * 34.0 / 15.), int(millis / 10), phase)
        payload = OttoServo.oscil(self.pin, int(amplitude), int(millis / 10), phase)
        # print(payload)
        self.host.write(bytes([0x06]) + payload)
        # self.host.write(bytes([0x02]) + OttoServo.move(self.pin, width, 1, curve))

    @classmethod
    def move(cls, index, width, span, curve=0):
        return pack('<BHHB', index, width, span, curve)

    @classmethod
    def oscil(cls, index, amplitude, span, phase=0):
        return pack('<BhHh', index, amplitude, span, phase)


class Otto():
    def __init__(self, servos, tempo):
        self.servos = servos
        self.tempo = tempo

    def home(self):
        for x in self.servos:
            x.SetPosition(90)

    def goingUp(self, tempo):
        self.home()
        delay(tempo)
        self.servos[0].SetPosition(80)
        self.servos[1].SetPosition(100)
        delay(tempo)
        self.servos[0].SetPosition(70)
        self.servos[1].SetPosition(110)
        delay(tempo)
        self.servos[0].SetPosition(60)
        self.servos[1].SetPosition(120)
        delay(tempo)
        self.servos[0].SetPosition(50)
        self.servos[1].SetPosition(130)
        delay(tempo)
        self.servos[0].SetPosition(40)
        self.servos[1].SetPosition(140)
        delay(tempo)
        self.servos[0].SetPosition(30)
        self.servos[1].SetPosition(150)
        delay(tempo)
        self.servos[0].SetPosition(20)
        self.servos[1].SetPosition(160)
        delay(tempo)

    def kickLeft(self, tempo):
        self.home()
        delay(tempo)
        self.servos[0].SetPosition(50)
        self.servos[1].SetPosition(70)
        delay(tempo)
        self.servos[0].SetPosition(80)
        self.servos[1].SetPosition(70)
        delay(tempo/4)
        self.servos[0].SetPosition(30)
        self.servos[1].SetPosition(70)
        delay(tempo/4)
        self.servos[0].SetPosition(80)
        self.servos[1].SetPosition(70)
        delay(tempo/4)
        self.servos[0].SetPosition(30)
        self.servos[1].SetPosition(70)
        delay(tempo/4)
        self.servos[0].SetPosition(80)
        self.servos[1].SetPosition(70)
        delay(tempo)

    def kickRight(self, tempo):
        self.home()
        delay(tempo)
        self.servos[0].SetPosition(110)
        self.servos[1].SetPosition(130)
        delay(tempo)
        self.servos[0].SetPosition(110)
        self.servos[1].SetPosition(100)
        delay(tempo/4)
        self.servos[0].SetPosition(110)
        self.servos[1].SetPosition(150)
        delay(tempo/4)
        self.servos[0].SetPosition(110)
        self.servos[1].SetPosition(80)
        delay(tempo/4)
        self.servos[0].SetPosition(110)
        self.servos[1].SetPosition(150)
        delay(tempo/4)
        self.servos[0].SetPosition(110)
        self.servos[1].SetPosition(100)
        delay(tempo)

    def lateral_fuerte(self, side, tempo):
        self.home()
        if side:
            self.servos[0].SetPosition(40)
        else:
            self.servos[1].SetPosition(140)

        delay(tempo/2)

        self.servos[0].SetPosition(90)
        self.servos[1].SetPosition(90)
        delay(tempo/2)

    def moveNServos(self, ts, newPosition):
        for i, x in enumerate(newPosition):
            self.servos[i].SetPosition(x, ts)
        delay(ts)

    def noGravity(self, tempo):
        move1 = [120, 140, 90, 90]
        move2 = [140, 140, 90, 90]
        move3 = [120, 140, 90, 90]
        move4 = [90, 90, 90, 90]

        self.home()
        self.moveNServos(tempo*2, move1)
        self.moveNServos(tempo*2, move2)
        delay(tempo*2)
        self.moveNServos(tempo*2, move3)
        self.moveNServos(tempo*2, move4)

    def oscillate(self, amplitude, offset, tempo, phase):
        for i, x in enumerate(self.servos):
            x.SetPosition(90 + offset[i])
            x.oscillate(amplitude[i], tempo, phase[i])
        delay(tempo)

    def primera_parte(self):
        move1 = [60, 120, 90, 90]
        move2 = [90, 90, 90, 90]
        move3 = [40, 140, 90, 90]

        for _ in range(3):
            for _ in range(3):
                self.lateral_fuerte(1, self.tempo/2)
                self.lateral_fuerte(0, self.tempo/4)
                self.lateral_fuerte(1, self.tempo/4)

        self.home()
        self.moveNServos(self.tempo * 0.4, move1)
        self.moveNServos(self.tempo * 0.4, move2)
        delay(self.tempo * 1.2)

        for _ in range(2):
            self.lateral_fuerte(1, self.tempo/2)
            self.lateral_fuerte(0, self.tempo/4)
            self.lateral_fuerte(1, self.tempo/4)
            delay(self.tempo)

        self.home()
        self.crusaito(self.tempo * 1.4)
        self.moveNServos(self.tempo * 1, move3)
        delay(2 * self.tempo)
        self.home()
        delay(2 * self.tempo)

    def segunda_parte(self):
        move1 = [90, 90, 80, 100]
        move2 = [90, 90, 100, 80]
        move3 = [90, 90, 80, 100]
        move4 = [90, 90, 100, 80]
        move5 = [40, 140, 80, 100]
        move6 = [40, 140, 100, 80]
        move7 = [90, 90, 80, 100]
        move8 = [90, 90, 100, 80]
        move9 = [40, 140, 80, 100]
        move10 = [40, 140, 100, 80]
        move11 = [90, 90, 80, 100]
        move12 = [90, 90, 100, 80]

        for _ in range(7):
            for _ in range(3):
                self.moveNServos(self.tempo * 0.15, move1)
                self.moveNServos(self.tempo * 0.15, move2)
                self.moveNServos(self.tempo * 0.15, move3)
                self.moveNServos(self.tempo * 0.15, move4)

            self.moveNServos(self.tempo * 0.15, move5)
            self.moveNServos(self.tempo * 0.15, move6)
            self.moveNServos(self.tempo * 0.15, move7)
            self.moveNServos(self.tempo * 0.15, move8)

        for _ in range(3):
            self.moveNServos(self.tempo * 0.15, move9)
            self.moveNServos(self.tempo * 0.15, move10)
            self.moveNServos(self.tempo * 0.15, move11)
            self.moveNServos(self.tempo * 0.15, move12)

    def walk(self, tempo):
        self.oscillate([15, 15, 30, 30], [0, 0, 0, 0], tempo, [0, 0, 90, 90])

    def walk2(self, tempo):
        self.oscillate([15, 15, 30, 30], [0, 0, 0, 0], tempo, [180, 180, 270, 270])

    def run(self, tempo):
        self.oscillate([10, 10, 10, 10], [0, 0, 0, 0], tempo, [0, 0, 90, 90])

    def backyard(self, tempo):
        self.oscillate([15, 15, 30, 30], [0, 0, 0, 0], tempo, [0, 0, -90, -90])

    def turnLeft(self, tempo):
        self.oscillate([20, 20, 10, 30], [0, 0, 0, 0], tempo, [0, 0, 90, 90])

    def turnRight(self, tempo):
        self.oscillate([20, 20, 30, 10], [0, 0, 0, 0], tempo, [0, 0, 90, 90])

    def moonWalkLeft(self, tempo):
        self.oscillate([25, 25, 0, 0], [-15, 15, 0, 0], tempo, [120, 180, 90, 90])

    def moonWalkRight(self, tempo):
        self.oscillate([25, 25, 0, 0], [-15, 15, 0, 0], tempo, [0, 300, 90, 90])

    def crusaito(self, tempo):
        self.oscillate([25, 25, 30, 30], [-15, 15, 0, 0], tempo, [0, 300, 90, 90])

    def swig(self, tempo):
        self.oscillate([25, 25, 0, 0], [-15, 15, 0, 0], tempo, [0, 0, 90, 90])

    def upDown(self, tempo):
        self.oscillate([25, 25, 0, 0], [-15, 15, 0, 0], tempo, [180, 0, 270, 270])

    def flapping(self, tempo):
        self.oscillate([15, 15, 8, 8], [-15, 15, 0, 0], tempo, [0, 180, 90, -90])

    def drunk(self, tempo):
        move1 = [60, 70, 90, 90]
        move2 = [110, 120, 90, 90]
        move3 = [60, 70, 90, 90]
        move4 = [110, 120, 90, 90]

        self.moveNServos(tempo*0.235, move1)
        self.moveNServos(tempo*0.235, move2)
        self.moveNServos(tempo*0.235, move3)
        self.moveNServos(tempo*0.235, move4)
