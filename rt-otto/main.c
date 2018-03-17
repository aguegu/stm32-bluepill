#include <string.h>
#include <stdlib.h>
#include "hal.h"
#include "ch.h"
#include "chprintf.h"
#include "usbcfg.h"
#include "easing.h"
#include "tinycrypt/aes.h"

#define ADDRESS_PCA9685 0x40
#define ADDRESS_AT24C02 0x50

#define LEN 16

#define MB_SIZE 4
#define BUFF_SIZE 256

#define UUID ((uint8_t *)0x1FFFF7E8)
#define ANGLE2WIDTH (2)

#define LICENSE_ADDRESS 0xf0

#define VERSION 2
#define POSTFIX 0x5846d7dc

static const uint8_t aes_key[16] = {
  0x02, 0xc8, 0x69, 0x40, 0xec, 0x17, 0xe0, 0xf8, 0xbd, 0xaa, 0xfd, 0x2b, 0xa4, 0x1c, 0xa8, 0x78
};

typedef struct {
  uint8_t buff[BUFF_SIZE];
  BaseChannel * source;
} Instruction;

static Instruction buff_rx[MB_SIZE];

static msg_t mbfree_buffer[MB_SIZE];
static MAILBOX_DECL(mbfree, mbfree_buffer, MB_SIZE);

static msg_t mbduty_buffer[MB_SIZE];
static MAILBOX_DECL(mbduty, mbduty_buffer, MB_SIZE);

static mutex_t mtx_bc;
static mutex_t mtx_servos[LEN];

static uint16_t width_init[LEN];
static uint16_t width_mid[LEN];
static uint16_t width_min[LEN];
static uint16_t width_max[LEN];

static binary_semaphore_t i2c_bsem;

typedef struct {
  double position;
  uint16_t span;
  uint16_t step;
  double start;
  double end;
  int16_t amplitude;
  int16_t phase;
  double (*curve)(double r);
  // uint8_t curve;
} Servo;

static Servo servos[LEN];

static uint8_t licensed = 0x00;
static uint16_t cnt = 0;

static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

static THD_WORKING_AREA(waBlink, 0);
static THD_FUNCTION(Blink, arg) {
  (void)arg;
  chRegSetThreadName("blink");
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(serusbcfg.usbp->state == USB_ACTIVE ? 250 : 1000);
  }
}

static const uint8_t PCA9685_CONF[6] = {0x00, 0x31, 0xfe, 136, 0x00, 0x21};
static uint8_t buff_i2c[LEN * 4 + 1] = {0};

void transmit(I2CDriver * i2cp) {
  uint16_t width;
  for (uint8_t i=0; i<LEN; i++) {
    width = (uint16_t)(servos[i].position + 0.5);
    if (width > width_max[i]) {
      width = width_max[i];
    }
    if (width < width_min[i]) {
      width = width_min[i];
    }
    buff_i2c[i * 4 + 3] = width & 0xff;
    buff_i2c[i * 4 + 4] = width >> 8;
  }

  i2cAcquireBus(i2cp);
  i2cMasterTransmit(i2cp, ADDRESS_PCA9685, buff_i2c, LEN * 4 + 1, NULL, 0);
  i2cReleaseBus(i2cp);
}

double calc_position(Servo * self, uint16_t step) {
  double t = (double)step / (double)self->span;
  double width;
  if (self->curve == oscillate) {
    width = self->start + self->amplitude * sin(t * 2 * PI + self->phase * PI / 180);
  } else {
    double r = self->curve(t);
    width = self->start + self->end * r - self->start * r;
  }
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
static THD_FUNCTION(ServoDriver, i2cp) {
  // (void)arg;
  chRegSetThreadName("ServoDriver");

  i2cAcquireBus(i2cp);
  i2cMasterTransmit(i2cp, ADDRESS_PCA9685, PCA9685_CONF, 2, NULL, 0);
  i2cMasterTransmit(i2cp, ADDRESS_PCA9685, PCA9685_CONF + 2, 2, NULL, 0);
  i2cMasterTransmit(i2cp, ADDRESS_PCA9685, PCA9685_CONF + 4, 2, NULL, 0);
  i2cReleaseBus(i2cp);

  buff_i2c[0] = 0x06;
  memset(buff_i2c + 1, 0, LEN * 4);

  transmit(i2cp);
  palClearPad(GPIOB, 5);

  while (true) {
    bool isUpdated = false;
    for (uint8_t i=0; i<LEN; i++) {
      if (servos[i].span) {
        update(i);
        isUpdated = true;
      }
    }
    if (isUpdated) {
      transmit(i2cp);
    }
    chBSemWait(&i2c_bsem);
  }
}

void fetchInstruction(BaseChannel *bc, uint8_t *buff, uint8_t * rx) {
  static Instruction * p;
  static msg_t status;
  static systime_t timeout = MS2ST(20);

  chnRead(bc, buff, 1);
  uint8_t length = buff[0];
  if (length >= 2) {
    chnRead(bc, buff + 1, 2);
    if (buff[1] + buff[2] == 0xff) {
      if (length == 2) {
        chMtxLock(&mtx_bc);
        chnWrite(bc, buff, 3);
        chMtxUnlock(&mtx_bc);
      } else {
        uint8_t len = chnReadTimeout(bc, buff + 3, length - 2, timeout);
        if (len == length - 2) {
          status = chMBFetch(&mbfree, (msg_t *)(&p), TIME_IMMEDIATE);
          if (status == MSG_OK) {
            if (!licensed && cnt > 0x1000 && !(cnt & 0x03)) {
              rx[0] = 3;
              rx[1] = buff[1];
              rx[2] = buff[2];
              rx[3] = 0xfe; // no license response
              chMtxLock(&mtx_bc);
              chnWrite(bc, rx, 4);
              chMtxUnlock(&mtx_bc);
              chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);
            } else {
              memcpy(p->buff, buff, length + 1);
              p->source = bc;
              chMBPost(&mbduty, (msg_t)p, TIME_INFINITE);
            }
            cnt++;
          } else {
            rx[0] = 3;
            rx[1] = buff[1];
            rx[2] = buff[2];
            rx[3] = 0xff; // busy response
            chMtxLock(&mtx_bc);
            chnWrite(bc, rx, 4);
            chMtxUnlock(&mtx_bc);
          }
        }
      }
    }
  }
}

static THD_WORKING_AREA(waPing, 512);
static THD_FUNCTION(Ping, bc0) {
  chRegSetThreadName("ping");

  uint8_t buff[BUFF_SIZE];
  uint8_t rx[4];
  BaseChannel *bc = (BaseChannel *)bc0;

  while (true) {
    fetchInstruction(bc, buff, rx);
  }
}

static THD_WORKING_AREA(waCdc, 512);
static THD_FUNCTION(Cdc, bc0) {
  chRegSetThreadName("cdc");
  uint8_t buff[BUFF_SIZE];
  uint8_t rx[4];
  BaseChannel *bc = (BaseChannel *)bc0;

  while (true) {
    fetchInstruction(bc, buff, rx);
  }
}

void check_license(I2CDriver * i2cp) {
  struct tc_aes_key_sched_struct s;
  tc_aes128_set_encrypt_key(&s, aes_key);

  uint8_t * license, * decrypted, * sid;
  license = (uint8_t *)malloc(16);
  decrypted = (uint8_t *)malloc(16);
  sid = (uint8_t *)malloc(16);

  uint8_t tx = LICENSE_ADDRESS;
  i2cAcquireBus(i2cp);
  i2cMasterTransmit(i2cp, ADDRESS_AT24C02, &tx, 1, license, 16);
  i2cReleaseBus(i2cp);

  memcpy(sid, UUID, 12);
  *(uint32_t *)(sid + 12) = POSTFIX;

  tc_aes_decrypt(decrypted, license, &s);

  if (!memcmp(decrypted, sid, 16)) {
    licensed = 0x01;
  } else {
    licensed = 0x00;
  }
}


void softreset(void) {
  *(uint32_t *)(0xE000ED0CUL) = 0x05FA0000UL | (*(uint32_t *)(0xE000ED0CUL) & 0x0700) | 0x04;
}

static THD_WORKING_AREA(waPong, 1024);
static THD_FUNCTION(Pong, i2cp) {
  // (void)arg;
  // uint8_t *p;
  chRegSetThreadName("pong");
  Instruction *instruction;
  uint8_t buff[BUFF_SIZE];
  uint8_t flag_reset = 0;

  while (true) {
    chMBFetch(&mbduty, (msg_t *)(&instruction), TIME_INFINITE);
    uint8_t *p = instruction->buff;

    memcpy(buff, p, 3);

    buff[3] = 0x00; // success
    uint8_t cursor = 4;

    if (p[3] == 0x01) { // curve
      for (uint8_t i = 4; i < p[0]; i += 6) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        uint16_t width = *(uint16_t *)(p + i + 1);
        uint16_t span = *(uint16_t *)(p + i + 3);
        uint8_t curve = *(uint8_t *)(p + i + 5);

        chMtxLock(mtx_servos + index);
        servos[index].start = servos[index].position;
        servos[index].end = width;
        servos[index].span = span;
        servos[index].curve = EASING[curve];
        chMtxUnlock(mtx_servos + index);
      }

      buff[0] = 3;
    } else if (p[3] == 0x02) { // oscillate
      for (uint8_t i = 4; i < p[0]; i += 7) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        int16_t amplitude = *(int16_t *)(p + i + 1);
        uint16_t span = *(uint16_t *)(p + i + 3);
        int16_t phase = *(int16_t *)(p + i + 5);

        chMtxLock(mtx_servos + index);
        servos[index].phase = phase;
        servos[index].amplitude = amplitude;
        servos[index].start = servos[index].position - amplitude * sin(phase * PI / 180.);
        servos[index].span = span;
        servos[index].curve = oscillate;
        chMtxUnlock(mtx_servos + index);
      }

      buff[0] = 3;
    } else if (p[3] == 0x03) { // write init/mid/min/max
      for (uint8_t i = 4; i < p[0]; i += 9) {
        uint8_t tx[9] = {0};
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        width_init[index] = *(uint16_t *)(p + i + 1);
        width_mid[index] = *(uint16_t *)(p + i + 3);
        width_min[index] = *(uint16_t *)(p + i + 5);
        width_max[index] = *(uint16_t *)(p + i + 7);

        tx[0] = index * 8;
        memcpy(tx + 1, p + i + 1, 8);
        i2cAcquireBus(i2cp);
        i2cMasterTransmit(i2cp, ADDRESS_AT24C02, tx, 9, NULL, 0);
        chThdSleepMilliseconds(5);
        i2cReleaseBus(i2cp);
      }
      buff[0] = 3;
    } else if (p[3] == 0x04) { // read init/mid/min/max
      for (uint8_t i = 0; i < LEN; i++) {
        *(uint16_t *)(buff + cursor) = width_init[i];
        *(uint16_t *)(buff + cursor + 2) = width_mid[i];
        *(uint16_t *)(buff + cursor + 4) = width_min[i];
        *(uint16_t *)(buff + cursor + 6) = width_max[i];
        cursor += 8;
      }
      buff[0] = 8 * LEN + 3;
    } else if (p[3] == 0x05) { // curve angle
      for (uint8_t i = 4; i < p[0]; i += 6) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        uint16_t angle = *(uint16_t *)(p + i + 1);
        uint16_t span = *(uint16_t *)(p + i + 3);
        uint8_t curve = *(uint8_t *)(p + i + 5);

        chMtxLock(mtx_servos + index);
        servos[index].start = servos[index].position;
        servos[index].end = width_mid[index] + (angle - 90) * ANGLE2WIDTH;
        servos[index].span = span;
        servos[index].curve = EASING[curve];
        chMtxUnlock(mtx_servos + index);
      }
      buff[0] = 3;
    } else if (p[3] == 0x06) { // oscillate angle
      for (uint8_t i = 4; i < p[0]; i += 7) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        int16_t amplitude = *(int16_t *)(p + i + 1);
        uint16_t span = *(uint16_t *)(p + i + 3);
        int16_t phase = *(int16_t *)(p + i + 5);

        chMtxLock(mtx_servos + index);
        servos[index].phase = phase;
        servos[index].amplitude = amplitude * ANGLE2WIDTH;
        servos[index].start = servos[index].position - servos[index].amplitude * sin(phase * PI / 180.);
        servos[index].span = span;
        servos[index].curve = oscillate;
        chMtxUnlock(mtx_servos + index);
      }
      buff[0] = 3;
    } else if (p[3] == 0x07) {
      for (uint8_t i = 4; i < p[0]; i += 3) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        uint16_t width = *(uint16_t *)(p + i + 1);
        chMtxLock(mtx_servos + index);
        servos[index].position = width;
        chMtxUnlock(mtx_servos + index);
      }
      buff[0] = 3;
    } else  if (p[3] == 0x08) {
      for (uint8_t i = 4; i < p[0]; i += 3) {
        uint8_t index = *(uint8_t *)(p + i) % LEN;
        uint16_t angle = *(uint16_t *)(p + i + 1);
        chMtxLock(mtx_servos + index);
        servos[index].position = width_mid[index] + (angle - 90) * ANGLE2WIDTH;
        chMtxUnlock(mtx_servos + index);
      }
      buff[0] = 3;
    } else if (p[3] == 0xf0) { // read chip serial
      memcpy(buff + cursor, UUID, 12);
      buff[0] = 12 + 3;
    } else if (p[3] == 0xf1) { // read license
      uint8_t tx[1] = {LICENSE_ADDRESS};
      i2cAcquireBus(i2cp);
      i2cMasterTransmit(i2cp, ADDRESS_AT24C02, tx, 1, buff + cursor, 16);
      i2cReleaseBus(i2cp);
      buff[0] = 16 + 3;
    } else if (p[3] == 0xf2) { // write license
      uint8_t tx[9] = {0};
      if (p[0] == 20) {
        uint8_t sum = 0;
        for (uint8_t i=4; i<20; i++) {
          sum += p[i];
        }
        if (sum == p[20]) {
          memcpy(tx + 1, p + 4, 8);
          tx[0] = LICENSE_ADDRESS;
          i2cAcquireBus(i2cp);
          i2cMasterTransmit(i2cp, ADDRESS_AT24C02, tx, 9, NULL, 0);
          chThdSleepMilliseconds(5);
          i2cReleaseBus(i2cp);
          memcpy(tx + 1, p + 4 + 8, 8);
          tx[0] = LICENSE_ADDRESS + 8;
          i2cAcquireBus(i2cp);
          i2cMasterTransmit(i2cp, ADDRESS_AT24C02, tx, 9, NULL, 0);
          chThdSleepMilliseconds(5);
          i2cReleaseBus(i2cp);

          check_license(i2cp);
        } else {
          buff[3] = 0xff; // failure
        }
      } else {
        buff[3] = 0xff; // failure
      }
      buff[0] = 3;
    } else if (p[3] == 0xf3) { // read licensed and version
      buff[4] = licensed;
      buff[5] = VERSION;
      buff[0] = 5;
    } else if (p[3] == 0xf4 && p[0] == 3) { // reset
      flag_reset = 1;
      buff[0] = 3;
    }

    chMtxLock(&mtx_bc);
    chnWrite(instruction->source, buff, buff[0] + 1);
    chMtxUnlock(&mtx_bc);

    chMBPost(&mbfree, (msg_t)p, TIME_INFINITE);

    if (flag_reset) { // reset
      chThdSleepMilliseconds(1);
      flag_reset = 0;
      softreset();
    }
  }
}

void read_config(I2CDriver * i2cp) {
  uint8_t *rx;
  uint8_t tx[1] = {0};
  rx = (uint8_t *)malloc(8 * LEN);
  i2cAcquireBus(i2cp);
  i2cMasterTransmit(i2cp, ADDRESS_AT24C02, tx, 1, rx, LEN << 3);
  i2cReleaseBus(i2cp);
  for (uint8_t i=0; i<LEN; i++) {
    uint8_t offset = i * 8;
    width_init[i] = *(uint16_t *)(rx + offset);
    width_mid[i] = *(uint16_t *)(rx + offset + 2);
    width_min[i] = *(uint16_t *)(rx + offset + 4);
    width_max[i] = *(uint16_t *)(rx + offset + 6);
  }
  free(rx);
}

void init_servo(Servo * self, uint8_t index) {
  self->position = (double)width_init[index];
  self->span = 0;
  self->step = 0;
  self->start = self->position;
  self->end = self->position;
  // self->curve = NULL;
  self->curve = easeLinear;
}

static void gpt1cb(GPTDriver *gptp) {
  (void)gptp;
  chSysLockFromISR();
  chBSemSignalI(&i2c_bsem);
  chSysUnlockFromISR();
  // palTogglePad(GPIOB, 5);
}

static const GPTConfig gpt1cfg = {
  10000,    /* timer clock.*/
  gpt1cb,        /* Timer callback.*/
  0,
  0
};

int main(void) {
  halInit();
  chSysInit();

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN);   // LED

  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // USART1 TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                     // USART1 RX

  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SCL
  palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);  // I2C SDA
  palSetPadMode(GPIOB, 5, PAL_MODE_OUTPUT_OPENDRAIN);           // PCA9685 OE

  palSetPad(GPIOB, 5);

  i2cStart(&I2CD1, &i2cfg1);
  sdStart(&SD1, NULL);
  chMtxObjectInit(&mtx_bc);

  read_config(&I2CD1);

  for (uint8_t i=0; i<LEN; i++) {
    init_servo(servos+i, i);
    chMtxObjectInit(mtx_servos + i);
  }

  chBSemObjectInit(&i2c_bsem, true);

  chMBObjectInit(&mbfree, mbfree_buffer, MB_SIZE);
  chMBObjectInit(&mbduty, mbduty_buffer, MB_SIZE);

  for (uint8_t i = 0; i < MB_SIZE; i++) {
    chMBPost(&mbfree, (msg_t)(buff_rx + i), TIME_INFINITE);
  }

  chThdCreateStatic(waBlink, sizeof(waBlink), (NORMALPRIO + 1), Blink, NULL);
  chThdCreateStatic(waServoDriver, sizeof(waServoDriver), (NORMALPRIO + 1), ServoDriver, &I2CD1);
  chThdCreateStatic(waPing, sizeof(waPing), (NORMALPRIO), Ping, (BaseChannel *)&SD1);
  chThdCreateStatic(waPong, sizeof(waPong), (NORMALPRIO), Pong, &I2CD1);

  gptStart(&GPTD1, &gpt1cfg);
  gptStartContinuous(&GPTD1, 98);  // 1000 / 100 = 10Hz

  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  usbDisconnectBus(serusbcfg.usbp);
  check_license(&I2CD1);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  chThdCreateStatic(waCdc, sizeof(waCdc), (NORMALPRIO - 1), Cdc, (BaseChannel *)&SDU1);

  while(true) {
    chThdSleepMilliseconds(1000);
  }
  return 0;
}
