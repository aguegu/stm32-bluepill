import serial
from struct import pack, unpack
from enum import IntEnum, auto
import time


class Curve(IntEnum):
    Linear = 0
    SineIn = auto()
    SineOut = auto()
    SineInOut = auto()
    QuadIn = auto()
    QuadOut = auto()
    QuadInOut = auto()
    CubicIn = auto()
    CubicOut = auto()
    CubicInOut = auto()
    QuarticIn = auto()
    QuarticOut = auto()
    QuarticInOut = auto()
    ExponentialIn = auto()
    ExponentialOut = auto()
    ExponentialInOut = auto()
    CircularIn = auto()
    CircularOut = auto()
    CircularInOut = auto()
    BackIn = auto()
    BackOut = auto()
    BackInOut = auto()
    ElasticIn = auto()
    ElasticOut = auto()
    ElasticInOut = auto()
    BounceIn = auto()
    BounceOut = auto()
    BounceInOut = auto()


class OttoServoV1():
    uid = 0

    def __init__(self, tty, index):
        self.tty = tty
        self.index = index

    def moveTo(self, angle, millis=0, curve=Curve.Linear):
        self.transmit(bytes([0x05]) + OttoServoV1.move(self.index, angle, max(1, int(millis / 10)), int(curve)))

    def oscillate(self, amplitude, millis=0, phase=0):
        self.transmit(bytes([0x06]) + OttoServoV1.oscil(self.index, int(amplitude), int(millis / 10), phase))


    def transmit(self, payload):
        tx = bytes([len(payload) + 2, OttoServoV1.uid, 0xff - OttoServoV1.uid]) + payload
        self.tty.reset_input_buffer()
        self.tty.write(tx)
        print(OttoServoV1.hex(tx))

        OttoServoV1.uid += 1
        OttoServoV1.uid &= 0xff

    @classmethod
    def hex(cls, bs):
        return ' '.join('%02x' % c for c in bs)

    @classmethod
    def move(cls, index, width, span, curve=0):
        return pack('<BHHB', index, width, span, curve)

    @classmethod
    def oscil(cls, index, amplitude, span, phase=0):
        return pack('<BhHh', index, amplitude, span, phase)


if __name__ == '__main__':
    tty = serial.Serial('/dev/cu.SLAB_USBtoUART', 115200)
    RR = OttoServoV1(tty, 0)
    RL = OttoServoV1(tty, 1)
    YR = OttoServoV1(tty, 2)
    YL = OttoServoV1(tty, 3)

    servos = [RR, RL, YR, YL]

    t = 1000
    extra = 100

    x = 60
    y = 33

    def home():
        for s in servos:
            s.moveTo(90, 10)
        time.sleep(0.01)

    def delay(millis):
        time.sleep(millis / 1000)


    home()

    while True:
        # L Step Forward
        RR.oscillate(15, t, 270)
        RL.oscillate(30, t, 270)

        YR.moveTo(90 - 50, t)
        YL.moveTo(90 - 50, t)

        delay(t + extra)

        # R Step Forward
        RR.oscillate(30, t, 90)
        RL.oscillate(15, t, 90)

        YR.moveTo(90 + 50, t)
        YL.moveTo(90 + 50, t)

        delay(t + extra)

        home()

        # Turn Right
        RR.oscillate(15, t, 270)
        RL.oscillate(30, t, 270)

        YR.oscillate(30, t, 0)
        YL.oscillate(15, t, 270)

        delay(t + extra)

        # Turn Left
        RR.oscillate(30, t, 90)
        RL.oscillate(15, t, 90)

        YR.oscillate(15, t, 90)
        YL.oscillate(30, t, 180)
        delay(t + extra)

        # Lean Left
        RR.oscillate(30, t, 90)
        RL.oscillate(15, t, 90)
        delay(t + extra)

        # Lean Right
        RR.oscillate(15, t, 270)
        RL.oscillate(30, t, 270)
        delay(t + extra)

        # kick right
        RR.moveTo(90 - 60, t / 2)
        RL.moveTo(90 - 30, t / 2)
        delay(t / 2)

        RR.moveTo(90 - 30, 10)
        delay(t / 16)
        RR.moveTo(90 - 60, 10)
        delay(t / 16)
        RR.moveTo(90 - 30, 10)
        delay(t / 16)

        RR.moveTo(90 - 60, 10)
        delay(t / 16)

        RR.moveTo(90, t / 2)
        RL.moveTo(90, t / 2)
        delay(t / 2)

        # kick left
        RR.moveTo(90 + 30, t / 2)
        RL.moveTo(90 + 60, t / 2)
        delay(t / 2)

        RL.moveTo(90 + 30, 10)
        delay(t / 16)
        RL.moveTo(90 + 60, 10)
        delay(t / 16)
        RL.moveTo(90 + 30, 10)
        delay(t / 16)

        RL.moveTo(90 + 60, 10)
        delay(t / 16)

        RR.moveTo(90, t / 2)
        RL.moveTo(90, t / 2)
        delay(t / 2)


# def SetPosition(self, angle, millis=0, curve=0):
#     # width = self.get_width(angle)
#     # self.host.write(bytes([0x01]) + OttoServo.move(self.pin, width, max(1, int(millis / 10)), curve))
#     self.host.write(bytes([0x05]) + OttoServo.move(self.pin, angle, max(1, int(millis / 10)), curve))
#
# def oscillate(self, amplitude, millis=0, phase=0):
#     # print(amplitude, phase, millis)
#     # payload = OttoServo.oscil(self.pin, int(amplitude * 34.0 / 15.), int(millis / 10), phase)
#     payload = OttoServo.oscil(self.pin, int(amplitude), int(millis / 10), phase)
#     # print(payload)
#     self.host.write(bytes([0x06]) + payload)
#     # self.host.write(bytes([0x02]) + OttoServo.move(self.pin, width, 1, curve))
#
# def move(cls, index, width, span, curve=0):
#     return pack('<BHHB', index, width, span, curve)
#
# @classmethod
# def oscil(cls, index, amplitude, span, phase=0):
#     return pack('<BhHh', index, amplitude, span, phase)
