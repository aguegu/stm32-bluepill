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

int main(void) {
    halInit();
    chSysInit();

    palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
    palSetPadMode(GPIOA, 0, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 1, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 2, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 3, PAL_MODE_OUTPUT_PUSHPULL);

    uint16_t i = 0;
    uint16_t c = 64 * 32;
    // 64: gear ratio, 32: full phase ration
    // gear ratio: 32 / 9 * 22 / 11 * 26 / 9 * 31 / 10
    // = (32 * 22 * 26 * 31) / (9 * 11 * 9 * 10) = 25792 / 405

    while(1) {
        palTogglePad(GPIOC, GPIOC_LED);

        palSetPad(GPIOA, i % 4);
        palSetPad(GPIOA, (i + 1) % 4);
        palClearPad(GPIOA, (i + 2) % 4);
        palClearPad(GPIOA, (i + 3) % 4);

        chThdSleepMilliseconds(1);

        i++;

        if (i == c) {
          chThdSleepSeconds(1);
          i = 0;
        }
    }

    while(1);

    return 0;
}
