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

#define LEN 16
#define CENTER 306
#define ADDRESS 0x40

#define MB_SIZE 4
#define BUFF_SIZE 256

static uint8_t buff_rx[MB_SIZE][BUFF_SIZE];
static msg_t mbfree_buffer[MB_SIZE];
static MAILBOX_DECL(mbfree, mbfree_buffer, MB_SIZE);

static msg_t mbduty_buffer[MB_SIZE];
static MAILBOX_DECL(mbduty, mbduty_buffer, MB_SIZE);

static systime_t timeout = MS2ST(200);
static mutex_t mtx_sd1;
static mutex_t mtx_servos[LEN];

BaseSequentialStream * bss = (BaseSequentialStream *)&SD1;

static THD_WORKING_AREA(waBlink, 0);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
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
static time_measurement_t t_16, t_idle;

typedef struct {
  double position;
  uint16_t span;
  uint16_t step;
  double start;
  double end;
} Servo;

static Servo servos[LEN];
static uint8_t buff_i2c[LEN * 4 + 1] = {0};

double linear(Servo * self, uint16_t step) {
  double t = (double)step / (double)self->span;
  return self->start + self->end * t - self->start * t;
}

void init_servo(Servo * self) {
  self->position = (double)CENTER;
  self->span = 0;
  self->step = 0;
  self->start = self->position;
  self->end = self->position;
}

void update(uint8_t index) {
  Servo * self = servos + index;
  chMtxLock(mtx_servos + index);
  if (self->step < self->span) {
    self->position = linear(self, self->step + 1);
    self->step++;
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
    buff_i2c[i * 4 + 3] = width & 0xff;
    buff_i2c[i * 4 + 4] = width >> 8;
  }
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmit(&I2CD1, ADDRESS, buff_i2c, LEN * 4 + 1, NULL, 0);
  i2cReleaseBus(&I2CD1);
}

static THD_WORKING_AREA(waServoDriver, 512);
static THD_FUNCTION(ServoDriver, arg) {
  (void)arg;
  chRegSetThreadName("ServoDriver");

  i2cStart(&I2CD1, &i2cfg1);
  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
  palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA

  uint8_t configs[6] = {0x00, 0x31, 0xfe, 136, 0x00, 0x21};

  i2cAcquireBus(&I2CD1);
  i2cMasterTransmit(&I2CD1, ADDRESS, configs, 2, NULL, 0);
  i2cMasterTransmit(&I2CD1, ADDRESS, configs + 2, 2, NULL, 0);
  i2cMasterTransmit(&I2CD1, ADDRESS, configs + 4, 2, NULL, 0);
  i2cReleaseBus(&I2CD1);

  chTMObjectInit(&t_16);
  chTMObjectInit(&t_idle);

  for (uint8_t i=0; i<LEN; i++) {
    init_servo(servos+i);
  }

  buff_i2c[0] = 0x06;
  transmit();

  while (true) {
    chTMStartMeasurementX(&t_16);
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
    chTMStopMeasurementX(&t_16);

    chTMStartMeasurementX(&t_idle);
    chBSemWait(&i2c_bsem);
    chTMStopMeasurementX(&t_idle);
  }
}

// static THD_WORKING_AREA(waBenchmark, 512);
// static THD_FUNCTION(Benchmark, arg) {
//   (void)arg;
//   chRegSetThreadName("Benchmark");
//
//
//   while (true) {
//     chTMObjectInit(&t_16);
//     chTMObjectInit(&t_idle);
//
//     for (uint8_t i=0; i<LEN; i++) {
//       servos[i].start = servos[i].position;
//       servos[i].end = 450;
//       servos[i].step = 0;
//       servos[i].span = 100;
//     }
//     chprintf(bss, "to 450, start: %d/%d\r\n", servos[0].step, servos[0].span);
//     chThdSleepMilliseconds(1000);
//     chprintf(bss, "to 450, end: %d/%d\r\n", servos[0].step, servos[0].span);
//
//     for (uint8_t i=0; i<LEN; i++) {
//       servos[i].start = servos[i].position;
//       servos[i].end = 150;
//       servos[i].step = 0;
//       servos[i].span = 200;
//     }
//     chprintf(bss, "to 150, start: %d/%d\r\n", servos[0].step, servos[0].span);
//     chThdSleepMilliseconds(2000);
//     chprintf(bss, "to 150, end: %d/%d\r\n", servos[0].step, servos[0].span);
//
//     // chThdSleepMilliseconds(1000);
//     chprintf(bss, "t_16: %d: %d, %d, %d\r\n", t_16.n, t_16.last, t_16.best, t_16.worst);
//     chprintf(bss, "t_idle: %d: %d, %d, %d\r\n", t_idle.n, t_idle.last, t_idle.best, t_idle.worst);
//     //
//
//   }
// }

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
  1000,    /* timer clock.*/
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
              memcpy(p, buff, length + 1);
              chMBPost(&mbduty, (msg_t)p, TIME_INFINITE);
            } else {
              chMtxLock(&mtx_sd1);
              rx[0] = 3;
              rx[1] = buff[1];
              rx[2] = buff[2];
              rx[3] = 0xff;
              sdWrite(&SD1, rx, 4); // busy response
              chMtxUnlock(&mtx_sd1);
            }
          }
        }
      }
    }
  }
}

static THD_WORKING_AREA(waPong, 1024);
static THD_FUNCTION(Pong, arg) {
  (void)arg;
  uint8_t *p;
  chRegSetThreadName("pong");
  uint8_t buff[BUFF_SIZE];
  systime_t tmo = MS2ST(4);
  msg_t status;

  while (true) {
    chMBFetch(&mbduty, (msg_t *)&p, TIME_INFINITE);
    memcpy(buff, p, 3);

    buff[3] = 0x00; // success
    uint8_t cursor = 4;

    for (uint8_t i = 4; i < p[0]; i += 6) {
      uint8_t index = *(uint8_t *)(p + i) % LEN;
      uint16_t width = *(uint16_t *)(p + i + 1);
      uint16_t span = *(uint16_t *)(p + i + 3);
      // uint8_t func = *(uint8_t *)(p + i + 5);
      // chprintf(bss, "%d, index: %d, width: %d, span: %d, func: %d\r\n", i, index, width, span, func);
      chMtxLock(mtx_servos + index);
      *(uint16_t *)(buff + cursor) = servos[index].step;
      servos[index].start = servos[index].position;
      servos[index].end = width;
      servos[index].span = span;
      chMtxUnlock(mtx_servos + index);
      cursor += 2;
    }

    buff[0] = cursor - 1;

    chMtxLock(&mtx_sd1);
    sdWrite(&SD1, buff, buff[0] + 1);
    chMtxUnlock(&mtx_sd1);

    chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);
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

    palSetPadMode(GPIOB, 5, PAL_MODE_OUTPUT_OPENDRAIN);
    sdStart(&SD1, NULL);

    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       // USART1 TX
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          // USART1 RX

    chBSemObjectInit(&i2c_bsem, true);

    chThdCreateStatic(waBlink, sizeof(waBlink), (NORMALPRIO + 2), Blink, NULL);
    chThdCreateStatic(waServoDriver, sizeof(waServoDriver), (NORMALPRIO + 1), ServoDriver, NULL);
    chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, NULL);
    chThdCreateStatic(waPong, sizeof(waPong), (NORMALPRIO), Pong, NULL);
    // chThdCreateStatic(waBenchmark, sizeof(waBenchmark), (NORMALPRIO), Benchmark, NULL);
    gptStart(&GPTD1, &gpt1cfg);

    gptStartContinuous(&GPTD1, 9);  // 1000 / 50 = 200Hz
    // chTMStartMeasurementX(&t_16);

    while(1) {
      chThdSleepSeconds(1);
    }
    return 0;
}
