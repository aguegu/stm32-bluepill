#include "ch.h"
#include "hal.h"

static THD_WORKING_AREA(waThread1, 0);
static THD_FUNCTION(Thread1, arg) {
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
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

  while (true) {
    chThdSleepSeconds(1);
  }

  return 0;
}
