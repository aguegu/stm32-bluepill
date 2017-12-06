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
#include "ch_test.h"
/*
 * Blinker thread #1.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    palSetPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
    palClearPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

int main(void) {
    halInit();
    chSysInit();

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

    sdStart(&SD1, NULL);
    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       /* USART1 TX.       */
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);      /* USART1 RX.       */

    palSetPadMode(GPIOB, 5, PAL_MODE_INPUT_PULLUP);

    while (true) {
      if (palReadPad(GPIOB, 5) == 0) {
        test_execute((BaseSequentialStream *)&SD1);
      }
      chThdSleepMilliseconds(500);
    }
    return 0;
}
