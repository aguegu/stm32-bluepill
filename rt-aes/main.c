#include "hal.h"
#include "ch.h"
#include "chprintf.h"
#include <string.h>
#include "tinycrypt/aes.h"
// #include "tinycrypt/constants.h"

#define UUID ((uint8_t *)0x1FFFF7E8)

BaseSequentialStream* chp = (BaseSequentialStream*) &SD1;

static THD_WORKING_AREA(waBlinky, 0);
static THD_FUNCTION(Blinky, arg) {
  (void)arg;
  chRegSetThreadName("blinker");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

static time_measurement_t tm_encrypt, tm_decrypt;

int main(void) {
  halInit();
  chSysInit();

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);

  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // USART1 TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                     // USART1 RX

  uint8_t uuid[16] = {0};
  uint8_t encrypted[16] = {0};
  uint8_t decrypted[16] = {0};

  memcpy(uuid, UUID, 12);

  const uint8_t nist_key[16] = {
		0x02, 0xc8, 0x69, 0x40, 0xec, 0x17, 0xe0, 0xf8, 0xbd, 0xaa, 0xfd, 0x2b, 0xa4, 0x1c, 0xa8, 0x78
	};

  struct tc_aes_key_sched_struct s;
  tc_aes128_set_encrypt_key(&s, nist_key);
  chThdCreateStatic(waBlinky, sizeof(waBlinky), NORMALPRIO+1, Blinky, NULL);

  chTMObjectInit(&tm_encrypt);
  chTMObjectInit(&tm_decrypt);

  while (true) {
    chTMStartMeasurementX(&tm_encrypt);
    tc_aes_encrypt(encrypted, uuid, &s);
    chTMStopMeasurementX(&tm_encrypt);

    chTMStartMeasurementX(&tm_decrypt);
    tc_aes_decrypt(decrypted, encrypted, &s);
    chTMStopMeasurementX(&tm_decrypt);


    chprintf(chp, "%u/%u/%u/%u ", tm_encrypt.last, tm_encrypt.worst, tm_encrypt.best, tm_encrypt.n);
    chprintf(chp, "%u/%u/%u/%u ", tm_decrypt.last, tm_decrypt.worst, tm_decrypt.best, tm_decrypt.n);

    for (uint8_t i=0; i<16; i++) {
      chprintf(chp, "%02x", *(uuid+i));
    }

    chprintf(chp, " ");

    for (uint8_t i=0; i<16; i++) {
      chprintf(chp, "%02x", *(encrypted+i));
    }

    chprintf(chp, " ");

    for (uint8_t i=0; i<16; i++) {
      chprintf(chp, "%02x", *(decrypted+i));
    }

    chprintf(chp, " %u\r\n", chVTGetSystemTimeX());
    chThdSleepMilliseconds(1000);
  }
  return 0;
}
