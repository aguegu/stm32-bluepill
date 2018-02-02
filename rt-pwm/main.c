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

static PWMConfig pwmcfg = {
  1000000,
  20000,
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0,
#if STM32_PWM_USE_ADVANCED
  0
#endif
};

int main(void) {
    halInit();
    chSysInit();

    // pull-up resisitor inplace, otherwise use pushpull mode
    palSetPadMode(GPIOA, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOA, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 0, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 1, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);

    palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 8, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);

    palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);

    pwmStart(&PWMD3, &pwmcfg);
    pwmStart(&PWMD4, &pwmcfg);

    pwmEnableChannel(&PWMD3, 0, 1500);
    pwmEnableChannel(&PWMD3, 1, 1500);
    pwmEnableChannel(&PWMD3, 2, 1500);
    pwmEnableChannel(&PWMD3, 3, 1500);

    pwmEnableChannel(&PWMD4, 0, 1500);
    pwmEnableChannel(&PWMD4, 1, 1500);
    pwmEnableChannel(&PWMD4, 2, 1500);
    pwmEnableChannel(&PWMD4, 3, 1500);


    int16_t w = 1500;
    int16_t step = 100;

    while(1) {
        pwmEnableChannel(&PWMD4, 0, w);
        pwmEnableChannel(&PWMD4, 1, w);
        pwmEnableChannel(&PWMD4, 2, w);
        pwmEnableChannel(&PWMD4, 3, w);

        pwmEnableChannel(&PWMD3, 0, w);
        pwmEnableChannel(&PWMD3, 1, w);
        pwmEnableChannel(&PWMD3, 2, w);
        pwmEnableChannel(&PWMD3, 3, w);

        w += step;

        if (w > 2400 || w < 600) {
          step = -step;
        }

        palTogglePad(GPIOC, GPIOC_LED);
        chThdSleepMilliseconds(50);
    }
    return 0;
}
