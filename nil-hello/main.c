#include "hal.h"
#include "ch.h"
#include "chprintf.h"

THD_WORKING_AREA(waThread1, 0);
THD_FUNCTION(Thread1, arg) {
  (void)arg;
  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN); // only 50MHz in GPIOv1

  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(50);
  }
}

THD_WORKING_AREA(waThread2, 128);
THD_FUNCTION(Thread2, arg) {
  (void)arg;
  uint8_t buff;
  BaseSequentialStream* chp = (BaseSequentialStream*) &SD1;
  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       // USART1 TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          // USART1 RX

  while (true) {
    uint8_t len = sdReadTimeout(&SD1, &buff, 1, 500);
    if (len) {
      chprintf(chp, "%02x\r\n", buff);
    } else {
      chnWrite(&SD1, (const uint8_t *)"Hello World, ", 13);
      chprintf(chp, "%u\r\n", chVTGetSystemTimeX());
    }
  }
}

THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "blinker", Thread1, NULL)
  THD_TABLE_ENTRY(waThread2, "hello", Thread2, NULL)
THD_TABLE_END

int main(void) {
  halInit();
  chSysInit();

  while(1);
  return 0;
}
