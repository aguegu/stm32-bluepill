#include "hal.h"
#include "ch.h"

#define MB_SIZE 8
#define BUFF_SIZE 256

static uint8_t buff_rx[MB_SIZE][BUFF_SIZE];
static uint8_t length_rx[MB_SIZE];
static msg_t mb1_buffer[MB_SIZE];
static MAILBOX_DECL(mb1, mb1_buffer, MB_SIZE);
// static systime_t timeout = MS2ST(4);

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

static THD_WORKING_AREA(waPing, 64);
static THD_FUNCTION(Ping, arg) {
  (void)arg;

  chRegSetThreadName("ping");
  msg_t i = 0;

  while (true) {
    sdRead(&SD1, length_rx + i, 1);
    if (length_rx[i]) {
      // uint8_t len = sdReadTimeout(&SD1, *(buff_rx + i), length_rx[i], TIME_INFINITE);
      uint8_t len = sdRead(&SD1, *(buff_rx + i), length_rx[i]);
      if (len == length_rx[i]) {
        chMBPost(&mb1, i, TIME_INFINITE);
        if (++i == MB_SIZE) {
          i = 0;
        }
      }
    }
  }
}

static THD_WORKING_AREA(waPong, 64);
static THD_FUNCTION(Pong, arg) {
  (void)arg;
  msg_t i;
  chRegSetThreadName("pong");

  while (true) {
    chMBFetch(&mb1, &i, TIME_INFINITE);
    sdWrite(&SD1, length_rx + i, 1);
    sdWrite(&SD1, *(buff_rx + i), length_rx[i]);
  }
}

int main(void) {
    halInit();
    chSysInit();

    chMBObjectInit(&mb1, mb1_buffer, MB_SIZE);

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
