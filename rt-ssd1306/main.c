#include "hal.h"
#include "string.h"
#include "stdlib.h"

#define WIDTH		128
#define HEIGHT  32
#define LEN (WIDTH * HEIGHT >> 3)
#define LEN_RAW (LEN + 3)

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(500);
  }
}

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

static const uint8_t font[30] = {
  0x3f, 0x21, 0x3f,
  0x22, 0x3f, 0x20,
  0x3d, 0x25, 0x27,
  0x25, 0x25, 0x3f,
  0x0f, 0x08, 0x3f,
  0x27, 0x25, 0x3d,
  0x3f, 0x25, 0x3d,
  0x01, 0x01, 0x3f,
  0x3f, 0x25, 0x3f,
  0x2f, 0x29, 0x3f,
};

void init(I2CDriver *i2cp) {
  msg_t status = MSG_OK;

  i2cAcquireBus(i2cp);

  uint8_t s[27] = {0x00, 0xae, 0xd5, 0x80, 0xa8, 31, 0x40, 0x8d, 0x14, 0x20, 0x00, 0xa0, 0xc0, 0xda, 0x02, 0x81, 0x8f, 0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0x2e, 0xaf, 0x00, 0x10};

  status = i2cMasterTransmit(i2cp, 0x3c,
                          s, 27, NULL, 0);
  osalDbgCheck(MSG_OK == status);
  i2cReleaseBus(i2cp);
}

uint8_t raw[LEN_RAW];
uint8_t *buffer = raw + 3;

void display(I2CDriver *i2cp) {
  i2cMasterTransmit(i2cp, 0x3c,
                        raw, LEN_RAW, NULL, 0);
  i2cReleaseBus(i2cp);
}

void clear(void) {
  memset(buffer, 0x00, LEN);
}

void plot(uint8_t x, uint8_t y) {
  buffer[(y >> 3) * WIDTH + x] |= 1 << (y & 0x07);
}

void line(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  int16_t tmp;

  if (steep) {
    tmp = x0; x0 = y0; y0 = tmp;
    tmp = x1; x1 = y1; y1 = tmp;
  }

  if (x0 > x1) {
    tmp = x0; x0 = x1; x1 = tmp;
    tmp = y0; y0 = y1; y1 = tmp;
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);
  int16_t err = dx >> 1;
  int16_t ystep = y0 < y1 ? 1 : -1;

  for (; x0<=x1; x0++) {
    if (steep) {
      plot(y0, x0);
    } else {
      plot(x0, y0);
    }
    err -= dy;
    if (err < 0) {
        y0 += ystep;
        err += dx;
    }
  }
}

void frame(void) {
  for (uint16_t i=0; i<8; i++) {
    uint16_t offset = (i << 6) + (i & 0x01) * 28;
    memcpy(buffer + offset, font + i * 3, 3);
    buffer[offset + 5] = 0x28;
  }
}

void printVal(uint16_t index, uint16_t val) {
  uint8_t st = 32;
  uint16_t offset = (index << 6) + (index & 0x01) * 28;
  do {
    uint8_t x = val % 10;
    memcpy(buffer + offset + st, font + x * 3, 3);
    val /= 10;
    st -= 4;
  } while(val);
}

int main(void) {
    halInit();
    chSysInit();

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

    i2cStart(&I2CD2, &i2cfg);
    palSetPadMode(GPIOB, 10, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
    palSetPadMode(GPIOB, 11, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA

    init(&I2CD2);
    raw[0] = 0x80;
    raw[1] = 0xb0;
    raw[2] = 0x40;

    clear();
    frame();

    for (uint8_t i=0; i<8; i++) {
      printVal(i, i * 7313);
    }

    display(&I2CD2);

    while (true) {

      chThdSleepMilliseconds(500);

    }
    return 0;
}
