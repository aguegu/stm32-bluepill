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

static uint8_t buff[BUFF_SIZE];

static THD_WORKING_AREA(waPing, 256);
static THD_FUNCTION(Ping, arg) {
  (void)arg;
  chRegSetThreadName("ping");

  uint8_t length;
  uint8_t *p;
  msg_t status;

  while (true) {
    uint8_t len = sdReadTimeout(&SD1, buff, 1, timeout);
    length = buff[0];
    if (len == 1 && length > 0) {
      len = sdReadTimeout(&SD1, buff + 1, length, timeout);
      if (len == length) {
        status = chMBFetch(&mbfree, (msg_t *)&p, TIME_IMMEDIATE);
        if (status == MSG_OK) {
          memcpy(p, buff, length);
          chMBPost(&mbduty, (msg_t)p, TIME_INFINITE);
        }
      }
    }
  }
}

static THD_WORKING_AREA(waPong, 128);
static THD_FUNCTION(Pong, arg) {
  (void)arg;
  void *p;
  uint8_t length;
  chRegSetThreadName("pong");

  while (true) {
    chMBFetch(&mbduty, (msg_t *)&p, TIME_INFINITE);
    chThdSleepMilliseconds(20);
    length = *(uint8_t *)p;
    sdWrite(&SD1, p, length + 1);
    chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);
  }
}

int main(void) {
    halInit();
    chSysInit();

    chMBObjectInit(&mbfree, mbfree_buffer, MB_SIZE);
    chMBObjectInit(&mbduty, mbduty_buffer, MB_SIZE);

    for (uint8_t i = 0; i < MB_SIZE; i++)
      chMBPost(&mbfree, (msg_t)(*(buff_rx + i)), TIME_INFINITE);

    sdStart(&SD1, NULL);
    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       // USART1 TX
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          // USART1 RX

    chThdCreateStatic(waBlink, sizeof(waBlink), (NORMALPRIO + 1), Blink, NULL);
    chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, NULL);
    chThdCreateStatic(waPong, sizeof(waPong), (NORMALPRIO), Pong, NULL);

    while (true) {
      chThdSleepSeconds(1);
    }
    return 0;
}
