#include "hal.h"
#include "ch.h"
#include "chprintf.h"

THD_WORKING_AREA(waThread1, 32);
THD_FUNCTION(Thread1, arg) {
  (void)arg;
  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);

  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

void logBuff(BaseSequentialStream* chp, uint8_t *p, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    chprintf(chp, "%02x ", p[i]);
  }
  chprintf(chp, "\r\n");
}

THD_WORKING_AREA(waThread2, 128);
THD_FUNCTION(Thread2, arg) {
  (void)arg;

  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // USART TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                     // USART RX

  chnWrite(&SD1, (const uint8_t *)"Hello World!\r\n", 14);

  i2cStart(&I2CD1, &i2cfg1);
  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
  palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA

  BaseSequentialStream* chp = (BaseSequentialStream*) &SD1;

  uint8_t address = 0x50;
  uint8_t out[16], in[17];
  // uint8_t x = 0x00;

  i2cMasterTransmit(&I2CD1, address, in, 1, out, 16);
  logBuff(chp, out, 16);

  for (uint8_t i = 1; i < 17; i++) {
    in[i] = i - 1;
  }

  i2cMasterTransmit(&I2CD1, address, in, 17, NULL, 0);
  chThdSleepMilliseconds(5);

  i2cMasterTransmit(&I2CD1, address, in, 1, out, 16);
  logBuff(chp, out, 16);

  for (uint8_t i = 1; i < 17; i++) {
    in[i] = 0xff;
  }

  i2cMasterTransmit(&I2CD1, address, in, 17, NULL, 0);
  chThdSleepMilliseconds(5);

  i2cMasterTransmit(&I2CD1, address, in, 1, out, 16);
  logBuff(chp, out, 16);

  while (true) {
    chThdSleepSeconds(1);
  }
}

THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "blinker1", Thread1, NULL)
  THD_TABLE_ENTRY(waThread2, "at24c", Thread2, NULL)
THD_TABLE_END


int main(void) {
  halInit();
  chSysInit();

  while(true);

  return 0;
}
