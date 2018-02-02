#include "hal.h"
#include "string.h"
#include "stdlib.h"
#include "ssd1306_i2c.h"

#define LEN 8
// servo len

#define MB_SIZE 4
#define BUFF_SIZE 256

static uint8_t buff_rx[MB_SIZE][BUFF_SIZE];

static msg_t mbfree_buffer[MB_SIZE];
static MAILBOX_DECL(mbfree, mbfree_buffer, MB_SIZE);
static msg_t mbduty_buffer[MB_SIZE];
static MAILBOX_DECL(mbduty, mbduty_buffer, MB_SIZE);

static uint16_t width_init[16];
static uint16_t width_mid[16];
static uint16_t width_min[16];
static uint16_t width_max[16];

typedef struct {
  double position;
  uint16_t span;
  uint16_t step;
  double start;
  double end;
  int16_t amplitude;
  int16_t phase;
  // double (*curve)(double r);
} Servo;

static Servo servos[LEN];

static THD_WORKING_AREA(waBlink, 128);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blinker");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(500);
  }
}

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

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

void init_servo(Servo * self, uint8_t index) {
  self->position = (double)width_init[index];
  self->span = 0;
  self->step = 0;
  self->start = self->position;
  self->end = self->position;
  // self->curve = easeLinear;
}

int main(void) {
    halInit();
    chSysInit();

    // pull-up resisitor inplace, otherwise use pushpull mode
    palSetPadMode(GPIOA, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOA, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 0, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 1, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);

    palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 8, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);


    palSetPadMode(GPIOB, 10, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
    palSetPadMode(GPIOB, 11, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA
    i2cStart(&I2CD2, &i2cfg);

    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // USART1 TX
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                     // USART1 RX
    sdStart(&SD1, NULL);

    pwmStart(&PWMD3, &pwmcfg);
    pwmStart(&PWMD4, &pwmcfg);

    ssd1306_init(&I2CD2);

    ssd1306_clear();
    ssd1306_frame();

    for (uint8_t i=0; i<LEN; i++) {
      width_init[i] = 1500;
      width_mid[i] = 1500;
      width_min[i] = 600;
      width_max[i] = 2400;
    }

    for (uint8_t i=0; i<LEN; i++) {
      init_servo(servos+i, i);
    }

    for (uint8_t i=0; i<LEN; i++) {
      ssd1306_putnumber(i, (int16_t)servos[i].position);
    }

    ssd1306_display();

    chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO+1, Blink, NULL);

    while (true) {
      chThdSleepMilliseconds(500);
    }
    return 0;
}
