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
#include "ds3231.h"

THD_WORKING_AREA(waThread1, 32);
THD_FUNCTION(Thread1, arg) {
  (void)arg;
  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);

  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

THD_WORKING_AREA(waThread2, 128);
THD_FUNCTION(Thread2, arg) {
  (void)arg;

  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // USART TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                     // USART RX

  chnWrite(&SD1, (const uint8_t *)"Hello World!\r\n", 14);

  i2cStart(&I2CD1, &i2cfg1);
  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
  palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA

  i2c_init();
  Ds3231 tm = {&I2CD1, 0x68, {0}};
  ds3231_init(&tm);
  uint8_t now[3] = {0x56, 0x34, 0x12}; // second, minute, hour
  ds3231_setTime(&tm, now, 3);

  while (true) {
    ds3231_refresh(&tm);
    // chprintf((BaseSequentialStream *)&SD1, "%02x:%02x:%02x\r\n", tm.data[2], tm.data[1], tm.data[0]);
    chprintf((BaseSequentialStream *)&SD1, "%02x\r\n", tm.data[0]);
    chThdSleepSeconds(1);
  }
}

THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "blinker1", Thread1, NULL)
  THD_TABLE_ENTRY(waThread2, "ds3231", Thread2, NULL)
THD_TABLE_END


int main(void) {
  halInit();
  chSysInit();

  while(true);

  return 0;
}
