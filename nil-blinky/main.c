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

THD_WORKING_AREA(waThread1, 128);
THD_FUNCTION(Thread1, arg) {

  (void)arg;

  while (true) {
    palSetPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
    palClearPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "blinker1", Thread1, NULL)
THD_TABLE_END


int main(void) {
    halInit();
    chSysInit();

    while(1) {
    }
    return 0;
}
