#include "ch.h"
#include "hal.h"

static PWMConfig pwmcfg = {
  1000000,
  20000,
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0,
#if STM32_PWM_USE_ADVANCED
  0
#endif
};

static THD_WORKING_AREA(waThread1, 0);
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN); // only 50MHz in GPIOv1
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

int main(void) {
  halInit();
  chSysInit();
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

  palSetPadMode(GPIOA, 6, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOA, 7, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 0, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 1, PAL_MODE_STM32_ALTERNATE_PUSHPULL);

  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 8, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 9, PAL_MODE_STM32_ALTERNATE_PUSHPULL);

  pwmStart(&PWMD3, &pwmcfg);
  pwmStart(&PWMD4, &pwmcfg);

  pwmEnableChannel(&PWMD3, 0, 1500);
  pwmEnableChannel(&PWMD3, 1, 1500);
  pwmEnableChannel(&PWMD3, 2, 1500);
  pwmEnableChannel(&PWMD3, 3, 1500);

  pwmEnableChannel(&PWMD4, 0, 1500);
  pwmEnableChannel(&PWMD4, 1, 1500);
  pwmEnableChannel(&PWMD4, 2, 1500);
  pwmEnableChannel(&PWMD4, 3, 1500);

  while (true) {
    chThdSleepSeconds(1);
  }

  return 0;
}
