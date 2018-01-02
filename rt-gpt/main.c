/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"
#include "ch.h"
#include "chprintf.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "tinycrypt/aes.h"

#define LEN 16
// #define WIDTH_MID 306
// #define WIDTH_MIN 115
// #define WIDTH_MAX 510
#define ADDRESS_PCA9685 0x40
#define ADDRESS_AT24C02 0x50
#define UUID ((uint8_t *)0x1FFFF7E8)
#define LICENSE_ADDRESS 0xf0

#define MB_SIZE 4
#define BUFF_SIZE 256

#define POSTFIX 0x9868ee46
#define VERSION 1

static const uint8_t aes_key[16] = {
  0x02, 0xc8, 0x69, 0x40, 0xec, 0x17, 0xe0, 0xf8, 0xbd, 0xaa, 0xfd, 0x2b, 0xa4, 0x1c, 0xa8, 0x78
};

static uint16_t width_init[16] = {306};
static uint16_t width_mid[16] = {306};
static uint16_t width_min[16] = {102};
static uint16_t width_max[16] = {510};


static uint8_t buff_rx[MB_SIZE][BUFF_SIZE];
static msg_t mbfree_buffer[MB_SIZE];
static MAILBOX_DECL(mbfree, mbfree_buffer, MB_SIZE);

static msg_t mbduty_buffer[MB_SIZE];
static MAILBOX_DECL(mbduty, mbduty_buffer, MB_SIZE);

static systime_t timeout = MS2ST(200);
static mutex_t mtx_sd1;
static mutex_t mtx_servos[LEN];
static uint8_t licensed = 0x00;
static uint16_t cnt = 0;

BaseSequentialStream * bss = (BaseSequentialStream *)&SD1;
const double PI = acos(-1);

static THD_WORKING_AREA(waBlink, 0);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(500);
  }
}

static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

static binary_semaphore_t i2c_bsem;

typedef struct {
  double position;
  uint16_t span;
  uint16_t step;
  double start;
  double end;
  int16_t amplitude;
  int16_t phase;
  double (*curve)(double r);
  // uint8_t curve;
} Servo;

static Servo servos[LEN];
static uint8_t buff_i2c[LEN * 4 + 1] = {0};

double easeLinear(double t) {
  return t;
}

double easeSineIn(double t) {
  return sin((t - 1) * PI / 2) + 1;
}

double easeSineOut(double t) {
  return sin(t * PI / 2);
}

double easeSineInOut(double t) {
  return 0.5 * (1 - cos(t * PI));
}

double easeQuadIn(double t) {
  return t * t;
}

double easeQuadOut(double t) {
  return t * (2 - t);
}

double easeQuadInOut(double t) {
  if (t < 0.5) {
    return 2 * t * t;
  }
  return (-2 * t * t) + (4 * t) - 1;
}

double easeCubicIn(double t) {
  return t * t * t;
}

double easeCubicOut(double t) {
  return (t - 1) * (t - 1) * (t - 1) + 1;
}

double easeCubicInOut(double t) {
  if (t < 0.5) {
    return 4 * t * t * t;
  }
  double p = 2 * t - 2;
  return 0.5 * p * p * p + 1;
}

double easeQuarticIn(double t) {
  return t * t * t * t;
}

double easeQuarticOut(double t) {
  return (t - 1) * (t - 1) * (t - 1) * (1 - t) + 1;
}

double easeQuarticInOut(double t) {
  if (t < 0.5) {
    return 8 * t * t * t * t;
  }
  double p = t - 1;
  return 1 - 8 * p * p * p * p;
}

double easeExponentialIn(double t) {
  if (t == 0.) {
    return 0.;
  }
  return exp(10 * (t - 1));
}

double easeExponentialOut(double t) {
  if (t == 1.) {
    return 1.;
  }
  return 1 - exp(-10 * t);
}

double easeExponentialInOut(double t) {
  if (t == 0. || t == 1.) {
    return t;
  }

  if (t < 0.5) {
    return 0.5 * exp(20 * t - 10);
  }
  return 1 - 0.5 * exp(10 - 20 * t);
}

double easeCircularIn(double t) {
  return 1 - sqrt(1 - t * t);
}

double easeCircularOut(double t) {
  return sqrt((2 - t) * t);
}

double easeCircularInOut(double t) {
  if (t < 0.5) {
    return 0.5 * (1 - sqrt(1 - 4 * t * t));
  }
  return 0.5 * (sqrt((3 - 2 * t) * (2 * t - 1)) + 1);
}

double easeBackIn(double t) {
  return t * t * t - t * sin(t * PI);
}

double easeBackOut(double t) {
  double p = 1.0 - t;
  return 1 - (p * p * p - p * sin(p * PI));
}

double easeBackInOut(double t) {
  double p;
  if (t < 0.5) {
    p = 2 * t;
    return 0.5 * (p * p * p - p * sin(p * PI));
  }

  p = 1 - (2 * t - 1);
  return 0.5 * (1 - (p * p * p - p * sin(p * PI))) + 0.5;
}

double easeElasticIn(double t) {
  return sin(13 * PI / 2 * t) * exp(10 * (t - 1));
}

double easeElasticOut(double t) {
  return sin(-13 * PI / 2 * (t + 1)) * exp(-10 * t) + 1;
}

double easeElasticInOut(double t) {
  if (t < 0.5) {
    return 0.5 * sin(13 * PI / 2 * (2 * t)) * exp(10 * ((2 * t) - 1));
  }
  return 0.5 * (sin(-13 * PI / 2 * ((2 * t - 1) + 1)) * exp(-10 * (2 * t - 1)) + 2);
}

double easeBounceOut(double t) {
  if (t < 4.0 / 11.0) {
    return 121.0 * t * t / 16.0;
  } else if (t < 8.0 / 11.0) {
    return (363.0 / 40.0 * t * t) - (99.0 / 10.0 * t) + 17.0 / 5.0;
  } else if (t < 9.0 / 10.0) {
    return (4356.0 / 361.0 * t * t) - (35442.0 / 1805.0 * t) + 16061.0 / 1805.0;
  }
  return (54.0 / 5.0 * t * t) - (513.0 / 25.0 * t) + 268.0 / 25.0;
}

double easeBounceIn(double t) {
  return 1 - easeBounceOut(1 - t);
}

double easeBounceInOut(double t) {
  if (t < 0.5) {
    return 0.5 * easeBounceIn (2 * t);
  }
  return 0.5 * easeBounceOut(2 * t - 1) + 0.5;
}

double oscillate(double t) {
  return t; // fake func
}

double (*EASING[28])(double) = {
  easeLinear,
  easeSineIn,
  easeSineOut,
  easeSineInOut,
  easeQuadIn,
  easeQuadOut,
  easeQuadInOut,
  easeCubicIn,
  easeCubicOut,
  easeCubicInOut,
  easeQuarticIn,
  easeQuarticOut,
  easeQuarticInOut,
  easeExponentialIn,
  easeExponentialOut,
  easeExponentialInOut,
  easeCircularIn,
  easeCircularOut,
  easeCircularInOut,
  easeBackIn,
  easeBackOut,
  easeBackInOut,
  easeElasticIn,
  easeElasticOut,
  easeElasticInOut,
  easeBounceIn,
  easeBounceOut,
  easeBounceInOut,
};

void init_servo(Servo * self, uint8_t index) {
  self->position = (double)width_init[index];
  self->span = 0;
  self->step = 0;
  self->start = self->position;
  self->end = self->position;
  self->curve = easeLinear;
}

double calc_position(Servo * self, uint16_t step) {
  double t = (double)step / (double)self->span;
  double width;
  if (self->curve == oscillate) {
    width = self->start + self->amplitude * sin(t * 2 * PI + self->phase * PI / 180);
  } else {
    double r = self->curve(t);
    width = self->start + self->end * r - self->start * r;
  }
  return width;
}

void update(uint8_t index) {
  Servo * self = servos + index;
  chMtxLock(mtx_servos + index);
  if (self->step < self->span) {
    self->position = calc_position(self, ++self->step);
  }

  if (self->step >= self->span) {
    self->step = 0;
    self->span = 0;
  }
  chMtxUnlock(mtx_servos + index);
}

void transmit(void) {
  uint16_t width;
  for (uint8_t i=0; i<LEN; i++) {
    width = (uint16_t)(servos[i].position + 0.5);
    if (width > width_max[i]) {
      width = width_max[i];
    }
    if (width < width_min[i]) {
      width = width_min[i];
    }
    buff_i2c[i * 4 + 3] = width & 0xff;
    buff_i2c[i * 4 + 4] = width >> 8;
  }
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmit(&I2CD1, ADDRESS_PCA9685, buff_i2c, LEN * 4 + 1, NULL, 0);
  i2cReleaseBus(&I2CD1);
}

static uint8_t PCA9685_CONF[6] = {0x00, 0x31, 0xfe, 136, 0x00, 0x21};

static THD_WORKING_AREA(waServoDriver, 512);
static THD_FUNCTION(ServoDriver, arg) {
  (void)arg;
  chRegSetThreadName("ServoDriver");

  i2cAcquireBus(&I2CD1);
  i2cMasterTransmit(&I2CD1, ADDRESS_PCA9685, PCA9685_CONF, 2, NULL, 0);
  i2cMasterTransmit(&I2CD1, ADDRESS_PCA9685, PCA9685_CONF + 2, 2, NULL, 0);
  i2cMasterTransmit(&I2CD1, ADDRESS_PCA9685, PCA9685_CONF + 4, 2, NULL, 0);
  i2cReleaseBus(&I2CD1);

  for (uint8_t i=0; i<LEN; i++) {
    init_servo(servos+i, i);
  }

  buff_i2c[0] = 0x06;
  transmit();

  while (true) {
    bool isUpdated = false;
    for (uint8_t i=0; i<LEN; i++) {
      if (servos[i].span) {
        update(i);
        isUpdated = true;
      }
    }
    if (isUpdated) {
      transmit();
    }
    chBSemWait(&i2c_bsem);
  }
}

static void gpt1cb(GPTDriver *gptp)
{
  (void)gptp;
  chSysLockFromISR();
  chBSemSignalI(&i2c_bsem);
  chSysUnlockFromISR();
  palTogglePad(GPIOB, 5);
}

static GPTConfig gpt1cfg =
{
  10000,    /* timer clock.*/
  gpt1cb,        /* Timer callback.*/
  0,
  0
};


static THD_WORKING_AREA(waPing, 512);
static THD_FUNCTION(Ping, arg) {
  (void)arg;
  chRegSetThreadName("ping");
  uint8_t buff[BUFF_SIZE];
  uint8_t rx[4];;
  uint8_t length;
  uint8_t *p;
  msg_t status;

  while (true) {
    sdRead(&SD1, buff, 1);
    length = buff[0];
    if (length >= 2) {
      sdRead(&SD1, buff + 1, 2);
      if (buff[1] + buff[2] == 0xff) {
        if (length == 2) {
          chMtxLock(&mtx_sd1);
          sdWrite(&SD1, buff, 3);
          chMtxUnlock(&mtx_sd1);
        } else {
          uint8_t len = sdReadTimeout(&SD1, buff + 3, length - 2, timeout);
          if (len == length - 2) {
            status = chMBFetch(&mbfree, (msg_t *)&p, TIME_IMMEDIATE);
            if (status == MSG_OK) {
              if (cnt > 0x1000 && (cnt & 0x03) == 0) {
                chMtxLock(&mtx_sd1);
                rx[0] = 3;
                rx[1] = buff[1];
                rx[2] = buff[2];
                rx[3] = 0xfe; // no license response
                sdWrite(&SD1, rx, 4);
                chMtxUnlock(&mtx_sd1);
              } else {
                memcpy(p, buff, length + 1);
                chMBPost(&mbduty, (msg_t)p, TIME_INFINITE);
              }
              cnt++;
            } else {
              chMtxLock(&mtx_sd1);
              rx[0] = 3;
              rx[1] = buff[1];
              rx[2] = buff[2];
              rx[3] = 0xff; // busy response
              sdWrite(&SD1, rx, 4);
              chMtxUnlock(&mtx_sd1);
            }
          }
        }
      }
    }
  }
}

void softreset(void) {
  *(uint32_t *)(0xE000ED0CUL) = 0x05FA0000UL | (*(uint32_t *)(0xE000ED0CUL) & 0x0700) | 0x04;
}

static THD_WORKING_AREA(waPong, 1024);
static THD_FUNCTION(Pong, arg) {
  (void)arg;
  uint8_t *p;
  chRegSetThreadName("pong");
  uint8_t buff[BUFF_SIZE];
  uint8_t flag_reset = 0;

  while (true) {
    chMBFetch(&mbduty, (msg_t *)&p, TIME_INFINITE);
    memcpy(buff, p, 3);

    buff[3] = 0x00; // success
    uint8_t cursor = 4;

    if (p[3] == 0x01) { // curve
      for (uint8_t i = 4; i < p[0]; i += 6) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        uint16_t width = *(uint16_t *)(p + i + 1);
        uint16_t span = *(uint16_t *)(p + i + 3);
        uint8_t curve = *(uint8_t *)(p + i + 5);

        // *(uint16_t *)(buff + cursor) = servos[index].step;
        // cursor += 2;
        chMtxLock(mtx_servos + index);
        servos[index].start = servos[index].position;
        servos[index].end = width;
        servos[index].span = span;
        servos[index].curve = EASING[curve];
        chMtxUnlock(mtx_servos + index);
      }

      // buff[0] = cursor - 1;
      buff[0] = 3;
    }

    if (p[3] == 0x02) { // oscillate
      for (uint8_t i = 4; i < p[0]; i += 7) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        int16_t amplitude = *(int16_t *)(p + i + 1);
        uint16_t span = *(uint16_t *)(p + i + 3);
        int16_t phase = *(int16_t *)(p + i + 5);

        // *(uint16_t *)(buff + cursor) = servos[index].amplitude;
        // cursor += 2;
        chMtxLock(mtx_servos + index);
        servos[index].phase = phase;
        servos[index].amplitude = amplitude;
        servos[index].start = servos[index].position - amplitude * sin(phase * PI / 180.);
        servos[index].span = span;
        servos[index].curve = oscillate;
        chMtxUnlock(mtx_servos + index);
      }

      // buff[0] = cursor - 1;
      buff[0] = 3;
    }

    if (p[3] == 0x03) { // write init/mid/min/max
      for (uint8_t i = 4; i < p[0]; i += 9) {
        uint8_t tx[9] = {0};
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        width_init[index] = *(uint16_t *)(p + i + 1);
        width_mid[index] = *(uint16_t *)(p + i + 3);
        width_min[index] = *(uint16_t *)(p + i + 5);
        width_max[index] = *(uint16_t *)(p + i + 7);

        tx[0] = index * 8;
        memcpy(tx + 1, p + i + 1, 8);
        i2cAcquireBus(&I2CD1);
        i2cMasterTransmit(&I2CD1, ADDRESS_AT24C02, tx, 9, NULL, 0);
        chThdSleepMilliseconds(5);
        i2cReleaseBus(&I2CD1);
      }
      buff[0] = 3;
    }

    if (p[3] == 0x04) { // read init/mid/min/max
      for (uint8_t i = 0; i < LEN; i++) {
        *(uint16_t *)(buff + cursor) = width_init[i];
        *(uint16_t *)(buff + cursor + 2) = width_mid[i];
        *(uint16_t *)(buff + cursor + 4) = width_min[i];
        *(uint16_t *)(buff + cursor + 6) = width_max[i];
        cursor += 8;
      }
      buff[0] = 8 * LEN + 3;
    }

    if (p[3] == 0xf0) { // read chip serial
      memcpy(buff + cursor, UUID, 12);
      buff[0] = 12 + 3;
    }

    if (p[3] == 0xf1) { // read license
      uint8_t tx[1] = {LICENSE_ADDRESS};
      i2cAcquireBus(&I2CD1);
      i2cMasterTransmit(&I2CD1, ADDRESS_AT24C02, tx, 1, buff + cursor, 16);
      i2cReleaseBus(&I2CD1);
      buff[0] = 16 + 3;
    }

    if (p[3] == 0xf2) { // write license
      uint8_t tx[9] = {0};
      if (p[0] == 20) {
        uint8_t sum = 0;
        for (uint8_t i=4; i<20; i++) {
          sum += p[i];
        }
        if (sum == p[20]) {
          memcpy(tx + 1, p + 4, 8);
          tx[0] = LICENSE_ADDRESS;
          i2cAcquireBus(&I2CD1);
          i2cMasterTransmit(&I2CD1, ADDRESS_AT24C02, tx, 9, NULL, 0);
          chThdSleepMilliseconds(5);
          i2cReleaseBus(&I2CD1);
          memcpy(tx + 1, p + 4 + 8, 8);
          tx[0] = LICENSE_ADDRESS + 8;
          i2cAcquireBus(&I2CD1);
          i2cMasterTransmit(&I2CD1, ADDRESS_AT24C02, tx, 9, NULL, 0);
          chThdSleepMilliseconds(5);
          i2cReleaseBus(&I2CD1);
          flag_reset = 1;
        } else {
          buff[3] = 0xff; // failure
        }
      } else {
        buff[3] = 0xff; // failure
      }
      buff[0] = 3;
    }

    if (p[3] == 0xf3) { // read licensed and version
      buff[4] = licensed;
      buff[5] = VERSION;
      buff[0] = 5;
    }

    if (p[3] == 0xf4 && p[0] == 3) { // reset
      flag_reset = 1;
      buff[0] = 3;
    }

    chMtxLock(&mtx_sd1);
    sdWrite(&SD1, buff, buff[0] + 1);
    chMtxUnlock(&mtx_sd1);

    chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);

    if (flag_reset) { // reset
      chThdSleepMilliseconds(1);
      flag_reset = 0;
      softreset();
    }
  }
}


int main (void) {
    halInit();
    chSysInit();

    chMBObjectInit(&mbfree, mbfree_buffer, MB_SIZE);
    chMBObjectInit(&mbduty, mbduty_buffer, MB_SIZE);

    chMtxObjectInit(&mtx_sd1);
    for (uint8_t i = 0; i < LEN; i++) {
      chMtxObjectInit(mtx_servos + i);
    }

    for (uint8_t i = 0; i < MB_SIZE; i++)
      chMBPost(&mbfree, (msg_t)(*(buff_rx + i)), TIME_INFINITE);

    palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
    // palClearPad(GPIOC, GPIOC_LED);
    // chThdSleepMilliseconds(3000);
    // palSetPad(GPIOC, GPIOC_LED);

    palSetPadMode(GPIOB, 5, PAL_MODE_OUTPUT_OPENDRAIN);
    sdStart(&SD1, NULL);

    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // USART1 TX
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                     // USART1 RX

    i2cStart(&I2CD1, &i2cfg1);
    palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
    palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA

    chBSemObjectInit(&i2c_bsem, true);

    uint8_t tx[1] = {0};

    //
    uint8_t *rx;
    rx = (uint8_t *)malloc(8 * LEN);
    i2cMasterTransmit(&I2CD1, ADDRESS_AT24C02, tx, 1, rx, 8 * LEN);
    for (uint8_t i=0; i<LEN; i++) {
      uint8_t offset = i * 8;
      width_init[i] = *(uint16_t *)(rx + offset);
      width_mid[i] = *(uint16_t *)(rx + offset + 2);
      width_min[i] = *(uint16_t *)(rx + offset + 4);
      width_max[i] = *(uint16_t *)(rx + offset + 6);
    }
    free(rx);

    // license verification
    struct tc_aes_key_sched_struct s;
    tc_aes128_set_encrypt_key(&s, aes_key);

    uint8_t * license, * decrypted, * sid;
    license = (uint8_t *)malloc(16);
    decrypted = (uint8_t *)malloc(16);
    sid = (uint8_t *)malloc(16);

    tx[0] = LICENSE_ADDRESS;
    i2cAcquireBus(&I2CD1);
    i2cMasterTransmit(&I2CD1, ADDRESS_AT24C02, tx, 1, license, 16);
    i2cReleaseBus(&I2CD1);

    memcpy(sid, UUID, 12);
    *(uint32_t *)(sid + 12) = POSTFIX;

    tc_aes_decrypt(decrypted, license, &s);

    if (!memcmp(decrypted, sid, 16)) {
      licensed = 0x01;
    }

    free(license);
    free(decrypted);
    free(sid);

    // start routine
    chThdCreateStatic(waBlink, sizeof(waBlink), (NORMALPRIO + 2), Blink, NULL);
    chThdCreateStatic(waServoDriver, sizeof(waServoDriver), (NORMALPRIO + 1), ServoDriver, NULL);
    chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, NULL);
    chThdCreateStatic(waPong, sizeof(waPong), (NORMALPRIO), Pong, NULL);
    gptStart(&GPTD1, &gpt1cfg);

    gptStartContinuous(&GPTD1, 98);  // 1000 / 50 = 200Hz

    while(1) {
      chThdSleepSeconds(1);
    }
    return 0;
}
