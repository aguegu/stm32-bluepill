#include "ch.h"
#include "hal.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"

static THD_WORKING_AREA(waBlinker, 0);
static THD_FUNCTION(Blinker, arg) {
  (void)arg;
  chRegSetThreadName("blinker");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN); // only 50MHz in GPIOv1
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(500);
  }
}

int main(void) {
  halInit();
  chSysInit();

  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);       // USART1 TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                          // USART1 RX

  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO+1, Blinker, NULL);

  while (true) {
    test_execute((BaseSequentialStream *)&SD1, &rt_test_suite);
    test_execute((BaseSequentialStream *)&SD1, &oslib_test_suite);
    chThdSleepSeconds(10);
  }

  return 0;
}
