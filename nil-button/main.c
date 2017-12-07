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
#include "chprintf.h"

THD_WORKING_AREA(waThread1, 32);
THD_FUNCTION(Thread1, arg) {
  (void)arg;
  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);

  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

THD_WORKING_AREA(waThread2, 128);
THD_FUNCTION(Thread2, arg) {
  (void)arg;
  /*
   * Activates the serial driver 1 using the driver default configuration.
   * PA9 and PA10 are routed to USART1.
   */
  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       /* USART1 TX. */
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          /* USART1 RX. */

  /* Welcome message.*/
  chnWrite(&SD1, (const uint8_t *)"Hello World!\r\n", 14);

  /* Waiting for button push and activation of the test suite.*/
  palSetPadMode(GPIOB, 5, PAL_MODE_INPUT_PULLUP);

  while (true) {
    if (palReadPad(GPIOB, 5) == 0) {
      test_execute((BaseSequentialStream *)&SD1);
    }
    chThdSleepMilliseconds(500);
  }
}


THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "blinker1", Thread1, NULL)
  THD_TABLE_ENTRY(wa_test_support, "test_support", test_support, (void *)&nil.threads[2])
  THD_TABLE_ENTRY(waThread2, "tester", Thread2, NULL)
THD_TABLE_END


int main(void) {
  halInit();
  chSysInit();

  while(true);

  return 0;
}
