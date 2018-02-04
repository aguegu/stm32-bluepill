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

static systime_t timeout = MS2ST(200);
static mutex_t mtx_sd1;
static mutex_t mtx_servos[LEN];

static binary_semaphore_t i2c_bsem;

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

static void gpt1cb(GPTDriver *gptp)
{
  (void)gptp;
  chSysLockFromISR();
  chBSemSignalI(&i2c_bsem);
  chSysUnlockFromISR();
  palTogglePad(GPIOB, 5);
}

static GPTConfig gpt1cfg =
{
  10000,    /* timer clock.*/
  gpt1cb,        /* Timer callback.*/
  0,
  0
};

void init_servo(Servo * self, uint8_t index) {
  self->position = (double)width_init[index];
  self->span = 0;
  self->step = 0;
  self->start = self->position;
  self->end = self->position;
  // self->curve = easeLinear;
}

static THD_WORKING_AREA(waPing, 512);
static THD_FUNCTION(Ping, arg) {
  (void)arg;
  chRegSetThreadName("ping");
  uint8_t buff[BUFF_SIZE];
  uint8_t rx[4];;
  uint8_t length;
  uint8_t *p;
  msg_t status;

  while (true) {
    sdRead(&SD1, buff, 1);
    length = buff[0];
    if (length >= 2) {
      sdRead(&SD1, buff + 1, 2);
      if (buff[1] + buff[2] == 0xff) {
        if (length == 2) {
          chMtxLock(&mtx_sd1);
          sdWrite(&SD1, buff, 3);
          chMtxUnlock(&mtx_sd1);
        } else {
          uint8_t len = sdReadTimeout(&SD1, buff + 3, length - 2, timeout);
          if (len == length - 2) {
            status = chMBFetch(&mbfree, (msg_t *)&p, TIME_IMMEDIATE);
            if (status == MSG_OK) {
              memcpy(p, buff, length + 1);
              chMBPost(&mbduty, (msg_t)p, TIME_INFINITE);
            } else {
              rx[0] = 3;
              rx[1] = buff[1];
              rx[2] = buff[2];
              rx[3] = 0xff; // busy response
              chMtxLock(&mtx_sd1);
              sdWrite(&SD1, rx, 4);
              chMtxUnlock(&mtx_sd1);
              chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);
            }
          }
        }
      }
    }
  }
}

static THD_WORKING_AREA(waPong, 1024);
static THD_FUNCTION(Pong, arg) {
  (void)arg;
  uint8_t *p;
  chRegSetThreadName("pong");
  uint8_t buff[BUFF_SIZE];

  while (true) {
    chMBFetch(&mbduty, (msg_t *)&p, TIME_INFINITE);
    memcpy(buff, p, 3);

    buff[3] = 0x00; // success
    // uint8_t cursor = 4;

    if (p[3] == 0x01) { // curve
      for (uint8_t i = 4; i < p[0]; i += 6) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        uint16_t width = *(uint16_t *)(p + i + 1);
        uint16_t span = *(uint16_t *)(p + i + 3);
        // uint8_t curve = *(uint8_t *)(p + i + 5);

        chMtxLock(mtx_servos + index);
        servos[index].start = servos[index].position;
        servos[index].end = width;
        servos[index].span = span;
        // servos[index].curve = EASING[curve];
        chMtxUnlock(mtx_servos + index);
      }
      buff[0] = 3;
    }

    chMtxLock(&mtx_sd1);
    sdWrite(&SD1, buff, buff[0] + 1);
    chMtxUnlock(&mtx_sd1);

    chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);
  }
}

double calc_position(Servo * self, uint16_t step) {
  double t = (double)step / (double)self->span;
  double width;
  // if (self->curve == oscillate) {
  //   width = self->start + self->amplitude * sin(t * 2 * PI + self->phase * PI / 180);
  // } else {
    // double r = self->curve(t);
    double r = t;
    width = self->start + self->end * r - self->start * r;
  // }
  return width;
}

void update(uint8_t index) {
  Servo * self = servos + index;
  chMtxLock(mtx_servos + index);
  if (self->step < self->span) {
    self->position = calc_position(self, ++self->step);
  }

  if (self->step >= self->span) {
    self->step = 0;
    self->span = 0;
  }
  chMtxUnlock(mtx_servos + index);
}

static THD_WORKING_AREA(waServoDriver, 512);
static THD_FUNCTION(ServoDriver, arg) {
  (void)arg;
  chRegSetThreadName("ServoDriver");

  ssd1306_init(&I2CD2);
  ssd1306_clear();
  ssd1306_frame();

  for (uint8_t i=0; i<LEN; i++) {
    ssd1306_putnumber(i, (int16_t)servos[i].position);
  }

  ssd1306_display();

  while (true) {
    for (uint8_t i=0; i<LEN; i++) {
      if (servos[i].span) {
        update(i);
        ssd1306_putnumber(i, (int16_t)servos[i].position);
      }
    }
    ssd1306_display();
    chBSemWait(&i2c_bsem);
  }
}

int main(void) {
    halInit();
    chSysInit();

    chMBObjectInit(&mbfree, mbfree_buffer, MB_SIZE);
    chMBObjectInit(&mbduty, mbduty_buffer, MB_SIZE);

    chMtxObjectInit(&mtx_sd1);
    for (uint8_t i = 0; i < LEN; i++) {
      chMtxObjectInit(mtx_servos + i);
    }

    for (uint8_t i = 0; i < MB_SIZE; i++)
      chMBPost(&mbfree, (msg_t)(*(buff_rx + i)), TIME_INFINITE);

    chBSemObjectInit(&i2c_bsem, true);

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


    for (uint8_t i=0; i<LEN; i++) {
      width_init[i] = 1500;
      width_mid[i] = 1500;
      width_min[i] = 600;
      width_max[i] = 2400;
    }

    for (uint8_t i=0; i<LEN; i++) {
      init_servo(servos+i, i);
    }

    chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO+2, Blink, NULL);
    chThdCreateStatic(waServoDriver, sizeof(waServoDriver), (NORMALPRIO + 1), ServoDriver, NULL);
    chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, NULL);
    chThdCreateStatic(waPong, sizeof(waPong), (NORMALPRIO), Pong, NULL);


    gptStart(&GPTD1, &gpt1cfg);
    gptStartContinuous(&GPTD1, 98);  // 1000 / 100 = 10Hz

    while (true) {
      chThdSleepMilliseconds(500);
    }
    return 0;
}
