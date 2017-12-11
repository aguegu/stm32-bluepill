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
// #include "chprintf.h"

static THD_WORKING_AREA(waBlink, 128);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

static THD_WORKING_AREA(waEcho, 128);
static THD_FUNCTION(Echo, arg) {
  (void)arg;
  uint8_t buff;
  chRegSetThreadName("echo");
  // BaseSequentialStream* chp = (BaseSequentialStream*) &SD1;
  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       // USART1 TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          // USART1 RX

  while (true) {
    sdRead(&SD1, &buff, 1);
    sdWrite(&SD1, &buff, 1);
    // uint8_t len = sdReadTimeout(&SD1, &buff, 1, 500);
    // if (len) {
    //   chprintf(chp, "%02x\r\n", buff);
    // } else {
    //   chnWrite(&SD1, (const uint8_t *)"Hello World, ", 13);
    //   chprintf(chp, "%u\r\n", chVTGetSystemTimeX());
    // }
  }
}


int main(void) {
    halInit();
    chSysInit();

    chThdCreateStatic(waBlink, sizeof(waBlink), (NORMALPRIO + 2), Blink, NULL);
    chThdCreateStatic(waEcho, sizeof(waEcho), (NORMALPRIO - 1), Echo, NULL);

    while (true) {
      chThdSleepSeconds(1);
    }
    return 0;
}
