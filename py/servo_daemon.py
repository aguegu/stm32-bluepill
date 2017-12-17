import threading
import time
import easing
from datetime import datetime


class Servo():
    def __init__(self, position):
        self.position = position
        self.span = 0
        self.func = None
        self.step = 0

    def update(self):
        if self.step < self.span:
            self.position = self.func(self.step + 1)
            self.step += 1

        if self.step == self.span:
            self.func = None
            self.step = 0
            self.span = 0

    def curve(self, easing_factory, to, span):
        self.func = easing_factory(self.position, to, span).ease
        self.span = span


class Bus(threading.Thread):
    def __init__(self, length):
        super(Bus, self).__init__()
        self.length = length
        self.servos = [Servo(0.) for _ in range(self.length)]
        self.alive = True
        self.scale = 10
        self.sleep = 1.0 / self.scale - 0.005

    def update(self):
        for x in self.servos:
            x.update()

    def run(self):
        print(' '.join(['%+2.4f' % x.position for x in self.servos]))

        while self.alive:
            if [x.span for x in self.servos] != [0] * self.length:
                print(' '.join('%d/%d' % (x.step, x.span) for x in self.servos))
                self.update()
                print(datetime.now().strftime('%H:%M:%S.%f'), ' '.join(['%+2.4f' % x.position for x in self.servos]))
            time.sleep(self.sleep)

    def stop(self):
        self.alive = False
        self.join()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()

    def curve(self, index, func, to, seconds):
        self.servos[index].curve(func, to, seconds * self.scale)


if __name__ == '__main__':
    t = Bus(4)
    try:
        t.start()
        time.sleep(0.2)
        t.curve(1, easing.Linear, 4, 2)
        time.sleep(0.3)
        t.curve(2, easing.ElasticEaseIn, 3, 2)

        time.sleep(4)
    except (KeyboardInterrupt, SystemExit):
        print('exit')
    finally:
        t.stop()
