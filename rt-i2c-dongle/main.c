#include "hal.h"
#include "ch.h"
#include <string.h>

#define MB_SIZE 4
#define BUFF_SIZE 256

static uint8_t buff_rx[MB_SIZE][BUFF_SIZE];

static msg_t mbfree_buffer[MB_SIZE];
static MAILBOX_DECL(mbfree, mbfree_buffer, MB_SIZE);

static msg_t mbduty_buffer[MB_SIZE];
static MAILBOX_DECL(mbduty, mbduty_buffer, MB_SIZE);

static systime_t timeout = MS2ST(200);
static mutex_t mtx_sd1;

static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

static THD_WORKING_AREA(waBlink, 0);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

static THD_WORKING_AREA(waPing, 512);
static THD_FUNCTION(Ping, arg) {
  (void)arg;
  chRegSetThreadName("ping");
  uint8_t buff[BUFF_SIZE];
  uint8_t rx[4];;
  uint8_t length;
  uint8_t *p;
  msg_t status;

  while (true) {
    sdRead(&SD1, buff, 1);
    length = buff[0];
    if (length >= 2) {
      sdRead(&SD1, buff + 1, 2);
      if (buff[1] + buff[2] == 0xff) {
        if (length == 2) {
          chMtxLock(&mtx_sd1);
          sdWrite(&SD1, buff, 3);
          chMtxUnlock(&mtx_sd1);
        } else {
          uint8_t len = sdReadTimeout(&SD1, buff + 3, length - 2, timeout);
          if (len == length - 2) {
            status = chMBFetch(&mbfree, (msg_t *)&p, TIME_IMMEDIATE);
            if (status == MSG_OK) {
              memcpy(p, buff, length + 1);
              chMBPost(&mbduty, (msg_t)p, TIME_INFINITE);
            } else {
              chMtxLock(&mtx_sd1);
              rx[0] = 3;
              rx[1] = buff[1];
              rx[2] = buff[2];
              rx[3] = 0xff;
              sdWrite(&SD1, rx, 4); // busy response
              chMtxUnlock(&mtx_sd1);
            }
          }
        }
      }
    }
  }
}

static THD_WORKING_AREA(waPong, 1024);
static THD_FUNCTION(Pong, arg) {
  (void)arg;
  uint8_t *p;
  // uint8_t length;
  chRegSetThreadName("pong");
  uint8_t buff[BUFF_SIZE];
  // systime_t tmo = MS2ST(4);
  // uint8_t addr = 0x00;

  while (true) {
    chMBFetch(&mbduty, (msg_t *)&p, TIME_INFINITE);
    memcpy(buff, p, 3);
    i2cAcquireBus(&I2CD1);
    // p: length, uid, ~uid, slave_address,
    // extra_wait, rxbytes, txbuf

    // 06 00 ff 50 00
    // 10 00
    if (p[0] == 5) {
      i2cMasterReceive(&I2CD1, p[3], buff + 3, p[5]);
    } else {
      i2cMasterTransmit(&I2CD1, p[3],
                              p + 6, p[0] - 5, buff + 3, p[5]);
    }

    if (p[4]) {
      chThdSleepMilliseconds(p[4]);
    }

    // osalDbgCheck(MSG_OK == status);
    i2cReleaseBus(&I2CD1);

    chMtxLock(&mtx_sd1);
    buff[0] = p[5] + 2;
    sdWrite(&SD1, buff, buff[0] + 1);
    chMtxUnlock(&mtx_sd1);

    chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);
  }
}

int main(void) {
    halInit();
    chSysInit();

    chMBObjectInit(&mbfree, mbfree_buffer, MB_SIZE);
    chMBObjectInit(&mbduty, mbduty_buffer, MB_SIZE);

    chMtxObjectInit(&mtx_sd1);

    for (uint8_t i = 0; i < MB_SIZE; i++)
      chMBPost(&mbfree, (msg_t)(*(buff_rx + i)), TIME_INFINITE);

    sdStart(&SD1, NULL);
    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       // USART1 TX
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          // USART1 RX

    i2cStart(&I2CD1, &i2cfg1);
    palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
    palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA

    chThdCreateStatic(waBlink, sizeof(waBlink), (NORMALPRIO + 1), Blink, NULL);
    chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, NULL);
    chThdCreateStatic(waPong, sizeof(waPong), (NORMALPRIO), Pong, NULL);

    while (true) {
      chThdSleepSeconds(1);
    }
    return 0;
}
