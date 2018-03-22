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

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "usbcfg.h"
#include <string.h>

#define BUFF_SIZE 256

static mutex_t mtx_sdu;

static THD_WORKING_AREA(waBlink, 0);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");
  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  systime_t span;

  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    span = serusbcfg.usbp->state == USB_ACTIVE ? 250 : 1000;
    chThdSleepMilliseconds(span);
  }
}

static THD_WORKING_AREA(waPing, 512);
static THD_FUNCTION(Ping, arg) {
  (void)arg;
  chRegSetThreadName("ping");
  static systime_t timeout = MS2ST(2);
  uint8_t buff[BUFF_SIZE];

  BaseChannel * bc = (BaseChannel *)&SDU1;

  while (true) {
    chnRead(bc, buff, 1);
    uint8_t length = buff[0];

    if (length) {
      chnReadTimeout(bc, buff + 1, length, timeout);
    }

    chMtxLock(&mtx_sdu);
    chnWrite(bc, buff, length + 1);
    chMtxUnlock(&mtx_sdu);
  }
}

int main(void) {
  halInit();
  chSysInit();

  chMtxObjectInit(&mtx_sdu);

  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO + 1, Blink, NULL);
  chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, NULL);

  while (true) {
    chThdSleepMilliseconds(2000);
  }
}
