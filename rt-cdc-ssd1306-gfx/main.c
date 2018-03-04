#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "usbcfg.h"
#include "ssd1306_i2c.h"
#include <string.h>

extern uint8_t * buffer;
static mutex_t mtx_sdu;

static THD_WORKING_AREA(waBlink, 0);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");
  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    systime_t time = serusbcfg.usbp->state == USB_ACTIVE ? 250 : 1000;
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(time);
  }
}

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

void point(uint8_t x, uint8_t y, bool b) {
  if (x >= WIDTH || y >= HEIGHT) {
    return;
  }
  uint16_t i = x * BYTES_PER_COL + (y >> 3);
  if (b) {
    buffer[i] |= 1 << (y & 0x07);
  } else {
    buffer[i] &= ~(1 << (y & 0x07));
  }
}

void area_quick(uint8_t x, uint8_t y, uint8_t * p, uint8_t len, uint8_t bytes_in_col) {
  uint8_t cols = len / bytes_in_col;
  for (uint8_t i=0; i<cols; i++) {
    memcpy(buffer + x * BYTES_PER_COL + y, p, bytes_in_col);
    p += bytes_in_col;
    x++;
  }
}

void area(uint8_t x, uint8_t y, uint8_t * p, uint8_t len, uint8_t mask) {
  if (x >= WIDTH || y > HEIGHT) {
    return;
  }

  uint8_t bytes_in_col = (mask + 7) >> 3;
  uint8_t cols = len / bytes_in_col;

  uint8_t x2 = x + cols;
  if (x2 > WIDTH) {
    x2 = WIDTH;
  }

  uint8_t y2 = y + mask;
  if (y2 > HEIGHT) {
    y2 = HEIGHT;
  }

  for (uint8_t i = x; i < x2; i++) {
    for (uint8_t j = y; j < y2; j++) {
      point(i, j, p[(i-x) * bytes_in_col + ((j-y) >> 3)] & (1 << ((j-y) & 0x07)));
    }
  }
}

uint8_t font[] = {
  // 0x80, 0x07, 0x7c, 0x00, 0x18, 0x00, 0x60, 0x00, 0x60, 0x00, 0x98, 0x07, 0x7c, 0x00
  // 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
  0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03
};

uint8_t block[] = {
  0x07, 0x07, 0x07
};

uint8_t reverse(uint8_t a) {
  return ((a & 0x1)  << 7) | ((a & 0x2)  << 5) |
         ((a & 0x4)  << 3) | ((a & 0x8)  << 1) |
         ((a & 0x10) >> 1) | ((a & 0x20) >> 3) |
         ((a & 0x40) >> 5) | ((a & 0x80) >> 7);
}

int main(void) {
  halInit();
  chSysInit();

  sduObjectInit(&SDU1);
  chMtxObjectInit(&mtx_sdu);

  sduStart(&SDU1, &serusbcfg);

  palSetPadMode(GPIOB, 10, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
  palSetPadMode(GPIOB, 11, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA
  i2cStart(&I2CD2, &i2cfg);


  // ssd1306_display();

  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  ssd1306_init(&I2CD2);
  ssd1306_clear();

  chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO, Blink, NULL);

  // BaseChannel * bc = (BaseChannel *)&SDU1;

  // memset(buffer, 0xff, BUFFER_LEN);
  // ssd1306_display();

  // uint16_t i = 0;
  // bool status = true;

  for (uint8_t i=0; i<4; i++) {
    // area_quick(12 * i, i, font, 14, 2);
    area(8 * i, i, font, 27, 18);
  }

  // for (uint16_t i=0; i < 128; i+=8) {
  //   for (uint16_t j=0; j < 64; j+=8) {
  //     area(i, j, font + (256 - 84 + j / 8 * 21 + i / 6) * 5, 5, 8);
  //   }

  //   area(i, j, font + (256 - 84 + j / 8 * 21 + i / 6) * 5, 5, 8);
  //   ssd1306_display();
  // }


  // for (uint16_t i=0; i < 128; i+=4) {
  //   for (uint16_t j=0; j < 64; j+=4) {
  //     area(i, j, block, 3, 3);
  //   }
  // }
  ssd1306_display();
  // }

  while (true) {
    // point(i / HEIGHT, i % HEIGHT, status);
    // ssd1306_display();
    // // i++;
    // if (i >= BUFFER_LEN << 3) {
    //   i = 0;
    //   status = !status;
    // }
    chThdSleepMicroseconds(10);
  }
}
