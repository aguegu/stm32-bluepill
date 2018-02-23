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
#include "ssd1306_i2c.h"
#include <string.h>

extern uint8_t * buffer;

#define MB_SIZE 4
#define BUFF_SIZE 1030

// static uint8_t buff_rx[MB_SIZE][BUFF_SIZE];
//
// static msg_t mbfree_buffer[MB_SIZE];
// static MAILBOX_DECL(mbfree, mbfree_buffer, MB_SIZE);
//
// static msg_t mbduty_buffer[MB_SIZE];
// static MAILBOX_DECL(mbduty, mbduty_buffer, MB_SIZE);
static const uint8_t numbers[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0,
  0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0,
  0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00,
  0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff,
  0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0x03, 0x0c, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0x3f, 0xc0, 0xff, 0xff, 0x3f, 0xc0, 0xff, 0xff, 0x3f, 0xc0, 0xff, 0xff, 0x3f, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0x03, 0x00, 0x30, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

static const uint8_t symbols[] = {
  0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xdc, 0xff, 0x00, 0x00, 0xc0, 0xff, 0x00, 0x00, 0x40, 0x80, 0x00, 0x00, 0x40, 0x80, 0x00, 0x00, 0x40, 0x80, 0x00, 0x00, 0x40, 0x80,
  0x00, 0x00, 0xc0, 0xe1, 0x00, 0x00, 0x40, 0x71, 0x00, 0x00, 0xc0, 0x39, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0xe7, 0x00, 0x00, 0x80, 0xa3, 0x00, 0x00, 0xc0, 0xe1
};

static const uint16_t POLYNOMIAL = 0x131; //P(x)=x^8+x^5+x^4+1 = 100110001

static systime_t timeout = MS2ST(2);
static mutex_t mtx_sdu;

static THD_WORKING_AREA(waBlink, 0);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");
  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    systime_t time = serusbcfg.usbp->state == USB_ACTIVE ? 500 : 1000;
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(time);
  }
}

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

static THD_WORKING_AREA(waPing, 2048);
static THD_FUNCTION(Ping, arg) {
  (void)arg;
  chRegSetThreadName("ping");
  uint8_t buff[BUFF_SIZE];
  // uint8_t rx[4];
  uint16_t length;
  // uint8_t *p;
  // msg_t status;

  BaseChannel * bc = (BaseChannel *)&SDU1;

  while (true) {
    chnRead(bc, buff, 2);
    memcpy(&length, buff, 2);
    // length = (*(uint16_t *)buff) % BUFF_SIZE;
    if (length >= 2) {
      chnReadTimeout(bc, buff + 2, length, timeout);

      chMtxLock(&mtx_sdu);
      chnWrite(bc, buff, length + 2);
      chMtxUnlock(&mtx_sdu);

      // memcpy(buffer, buff + 4, 1024);
      ssd1306_display();
      // status = chMBFetch(&mbfree, (msg_t *)&p, TIME_IMMEDIATE);
            // if (status == MSG_OK) {
            //   memcpy(p, buff, length + 1);
            //   chMBPost(&mbduty, (msg_t)p, TIME_INFINITE);
            // } else {
            //   chMtxLock(&mtx_sdu);
            //   rx[0] = 3;
            //   rx[1] = buff[1];
            //   rx[2] = buff[2];
            //   rx[3] = 0xff;
            //   sdWrite(bc, rx, 4); // busy response
            //   chMtxUnlock(&mtx_sdu);
    }
  }
}

// static THD_WORKING_AREA(waPong, 1024);
// static THD_FUNCTION(Pong, arg) {
//   (void)arg;
//   uint8_t *p;
//   chRegSetThreadName("pong");
//   uint8_t buff[BUFF_SIZE];
//   systime_t tmo = MS2ST(4);
//   msg_t status;
//
//   while (true) {
//     chMBFetch(&mbduty, (msg_t *)&p, TIME_INFINITE);
//     memcpy(buff, p, 3);
//     i2cAcquireBus(&I2CD1);
//
//     if (p[0] == 5) {
//       status = i2cMasterReceiveTimeout(&I2CD1, p[3], buff + 4, p[5], tmo);
//     } else {
//       status = i2cMasterTransmitTimeout(&I2CD1, p[3],
//                               p + 6, p[0] - 5, buff + 4, p[5], tmo);
//     }
//
//     if (p[4]) {
//       chThdSleepMilliseconds(p[4]);
//     }
//
//     switch (status) {
//       case MSG_TIMEOUT:
//         buff[3] = 1;
//         buff[0] = 3;
//         break;
//       case MSG_RESET:
//         buff[3] = 2;
//         buff[0] = 3;
//         break;
//       default:
//         buff[3] = status;
//         buff[0] = p[5] + 3;
//     }
//
//     i2cReleaseBus(&I2CD1);
//
//     chMtxLock(&mtx_sdu);
//
//     sdWrite(&SD1, buff, buff[0] + 1);
//     chMtxUnlock(&mtx_sdu);
//
//     chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);
//   }
// }

void putNumber(float y) {
  ssd1306_clear();
  int16_t x = (int16_t)(y * 100);
  uint8_t j = 0;
  int16_t neg = 0;

  if (x < 0) {
    x = -x;
    neg = 1;
  }

  do {
    uint8_t i = x % 10;
    memcpy(buffer + BUFFER_LEN - 72 - 80 * j - 40 - (j < 2 ? 0 : 16), numbers + 72 * i, 72);
    x /= 10;
    j++;
  } while (x || j < 3);

  buffer[BUFFER_LEN - 160 - 40 - 8 + 3] = 0xc0;
  buffer[BUFFER_LEN - 160 - 40 - 4 + 3] = 0xc0;

  if (neg) {
    for (uint8_t i = 0; i < 16; i++) {
      buffer[BUFFER_LEN - 72 - 80 * j - 56 + 1 + 4 * i] = 0x0c;
    }
  }
}


bool checkCrc(uint8_t *data, uint8_t len, uint8_t checksum) {
  uint8_t crc = 0;

  for (uint8_t i = 0; i < len; ++i) {
    crc ^= (data[i]);
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ POLYNOMIAL;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return (crc == checksum);
}

int main(void) {
  halInit();
  chSysInit();

  sduObjectInit(&SDU1);
  chMtxObjectInit(&mtx_sdu);

  sduStart(&SDU1, &serusbcfg);

  palSetPadMode(GPIOB, 10, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
  palSetPadMode(GPIOB, 11, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA
  i2cStart(&I2CD2, &i2cfg);

  ssd1306_init(&I2CD2);
  ssd1306_clear();
  // ssd1306_display();

  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO, Blink, NULL);
  chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, NULL);

  uint8_t s[1] = {0xe3};
  uint8_t result[3];
  float t;

  BaseChannel * bc = (BaseChannel *)&SDU1;

  // int16_t x = 54243;
  while (true) {
    s[0] = 0xe3;
    i2cAcquireBus(&I2CD2);
    i2cMasterTransmit(&I2CD2, 0x40,
                            s, 1, result, 3);
    i2cReleaseBus(&I2CD2);

    if (checkCrc(result, 2, result[2]) && (result[1] & 0x02) == 0) {
      chMtxLock(&mtx_sdu);
      chnWrite(bc, result, 3);
      chMtxUnlock(&mtx_sdu);

      t = (((int16_t)result[0] << 8) + result[1]) & 0xfffc;
      putNumber(t * 175.72 / 65536 - 46.85);

      memcpy(buffer + BUFFER_LEN - 32, symbols, 32);

      buffer[3] = 0x01;
      buffer[7] = 0x01;
      buffer[11] = 0xff;
      buffer[15] = 0x01;
      buffer[19] = 0x01;

      ssd1306_display();
    }

    chThdSleepMilliseconds(2000);

    s[0] = 0xe5;
    i2cAcquireBus(&I2CD2);
    i2cMasterTransmit(&I2CD2, 0x40,
                            s, 1, result, 3);

    i2cReleaseBus(&I2CD2);

    if (checkCrc(result, 2, result[2]) && (result[1] & 0x02)) {
      chMtxLock(&mtx_sdu);
      chnWrite(bc, result, 3);
      chMtxUnlock(&mtx_sdu);

      t = (((int16_t)result[0] << 8) + result[1]) & 0xfffc;

      putNumber(t * 125.0 / 65536 - 6);
      memcpy(buffer + BUFFER_LEN - 32, symbols + 32, 32);

      buffer[3] = 0xff;
      buffer[7] = 0x11;
      buffer[11] = 0x31;
      buffer[15] = 0x51;
      buffer[19] = 0x8e;

      buffer[27] = 0xff;
      buffer[31] = 0x08;
      buffer[35] = 0x08;
      buffer[39] = 0x08;
      buffer[43] = 0xff;

      ssd1306_display();
    }
    chThdSleepMilliseconds(2000);
  }
}
