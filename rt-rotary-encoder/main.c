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
#include "ssd1306_i2c.h"
#include "stdlib.h"

int16_t speed_l = 0;
int16_t speed_r = 0;


static THD_WORKING_AREA(waServoDriver, 512);
static THD_FUNCTION(ServoDriver, arg) {
  (void)arg;
  chRegSetThreadName("ServoDriver");

  ssd1306_init(&I2CD2);
  ssd1306_clear();

  while (true) {
    ssd1306_putnumber(0, speed_l);
    ssd1306_putnumber(1, speed_r);

    ssd1306_display();
    chThdSleepMilliseconds(50);
  }
}

static THD_WORKING_AREA(waMotorL, 128);
static THD_FUNCTION(MotorL, arg) {
  (void)arg;
  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 13, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 14, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 15, PAL_MODE_OUTPUT_PUSHPULL);
  uint8_t i = 0;

  while(true) {
    if (speed_l) {
      palSetPad(GPIOB, (i % 4) + 12);
      palSetPad(GPIOB, ((i + 1) % 4) + 12);
      palClearPad(GPIOB, ((i + 2) % 4) + 12);
      palClearPad(GPIOB, ((i + 3) % 4) + 12);
      if (speed_l > 0) {
        i++;
      }
      if (speed_l < 0) {
        i--;
      }
      chThdSleepMicroseconds(abs(speed_l) << 8);
    } else {
      chThdSleepMilliseconds(50);
    }
  }
}

static THD_WORKING_AREA(waMotorR, 128);
static THD_FUNCTION(MotorR, arg) {
  (void)arg;
  palSetPadMode(GPIOB, 6, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 7, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 8, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 9, PAL_MODE_OUTPUT_PUSHPULL);

  uint8_t i = 0;

  while (true) {
    if (speed_r) {
      palSetPad(GPIOB, (i % 4) + 6);
      palSetPad(GPIOB, ((i + 1) % 4) + 6);
      palClearPad(GPIOB, ((i + 2) % 4) + 6);
      palClearPad(GPIOB, ((i + 3) % 4) + 6);

      if (speed_r > 0) {
        i++;
      }
      if (speed_r < 0) {
        i--;
      }
      chThdSleepMicroseconds(abs(speed_r) << 8);
    } else {
      chThdSleepMilliseconds(50);
    }
  }
}

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

int main(void) {
    halInit();
    chSysInit();

    palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
    palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 2, PAL_MODE_INPUT_PULLDOWN);

    palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 5, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_PULLDOWN);

    palSetPadMode(GPIOB, 10, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
    palSetPadMode(GPIOB, 11, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA
    i2cStart(&I2CD2, &i2cfg);

    // palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // USART1 TX
    // palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                     // USART1 RX
    // sdStart(&SD1, NULL);

    uint8_t cnt_l_increase = 0;
    uint8_t cnt_l_decrease = 0;
    uint8_t released_l = 1;

    uint8_t cnt_r_increase = 0;
    uint8_t cnt_r_decrease = 0;
    uint8_t released_r = 1;

    uint8_t CNT = 0x02;

    chThdCreateStatic(waServoDriver, sizeof(waServoDriver), (NORMALPRIO), ServoDriver, NULL);
    chThdCreateStatic(waMotorL, sizeof(waMotorL), (NORMALPRIO), MotorL, NULL);
    chThdCreateStatic(waMotorR, sizeof(waMotorR), (NORMALPRIO), MotorR, NULL);

    while(1) {
      if (palReadPad(GPIOA, 2) == 1) {
        if (released_l) {
          cnt_l_increase++;
        }
      } else if (palReadPad(GPIOA, 1) == 1) {
        palTogglePad(GPIOC, GPIOC_LED);
        if (released_l) {
          cnt_l_decrease++;
        }
      } else {
        released_l = 1;
        cnt_l_increase = 0;
        cnt_l_decrease = 0;
      }

      if (palReadPad(GPIOA, 4) == 1) {
        if (released_r) {
          cnt_r_increase++;
        }
      } else if (palReadPad(GPIOA, 5) == 1) {
        palTogglePad(GPIOC, GPIOC_LED);
        if (released_r) {
          cnt_r_decrease++;
        }
      } else {
        released_r = 1;
        cnt_r_increase = 0;
        cnt_r_decrease = 0;
      }

      if (released_l) {
        if (cnt_l_increase > CNT) {
          if (speed_l < 127) {
            speed_l ++;
          }
          cnt_l_increase = 0;
          released_l = 0;
        } else if (cnt_l_decrease > CNT) {
          if (speed_l > -127) {
            speed_l --;
          }
          cnt_l_decrease = 0;
          released_l = 0;
        }
      }

      if (released_r) {
        if (cnt_r_increase > CNT) {
          if (speed_r < 127) {
            speed_r ++;
          }
          cnt_r_increase = 0;
          released_r = 0;
        } else if (cnt_r_decrease > CNT) {
          if (speed_r > -127) {
            speed_r --;
          }
          cnt_r_decrease = 0;
          released_r = 0;
        }
      }
      chThdSleepMilliseconds(2);
    }
    return 0;
}
