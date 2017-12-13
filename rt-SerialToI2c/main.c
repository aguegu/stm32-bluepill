#include "hal.h"
#include "ch.h"

#define MB_SIZE 4
// #define BUFF_SIZE 1

// static uint8_t buffs[MB_SIZE][BUFF_SIZE];

static msg_t mb_buffer[MB_SIZE];
static MAILBOX_DECL(mb, mb_buffer, MB_SIZE);


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

static THD_WORKING_AREA(waPing, 128);
static THD_FUNCTION(Ping, arg) {
  (void)arg;

  chRegSetThreadName("ping");
  uint8_t buff;

  while (true) {
    sdRead(&SD1, &buff, 1);
    chMBPost(&mb, (msg_t)buff, TIME_INFINITE);
  }
}

static THD_WORKING_AREA(waPong, 128);
static THD_FUNCTION(Pong, arg) {
  (void)arg;
  msg_t msg;
  chRegSetThreadName("pong");

  while (true) {
    chMBFetch(&mb, &msg, TIME_INFINITE);
    uint8_t buff = (uint8_t)msg;
    sdWrite(&SD1, &buff, 1);
  }
}

int main(void) {
    halInit();
    chSysInit();

    chMBObjectInit(&mb, mb_buffer, MB_SIZE);

    sdStart(&SD1, NULL);
    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       // USART1 TX
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          // USART1 RX

    chThdCreateStatic(waBlink, sizeof(waBlink), (NORMALPRIO + 1), Blink, NULL);
    chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO - 1), Ping, NULL);
    chThdCreateStatic(waPong, sizeof(waPong), (NORMALPRIO - 1), Pong, NULL);

    while (true) {
      chThdSleepSeconds(1);
    }
    return 0;
}
