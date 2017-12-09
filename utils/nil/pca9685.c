#include "ch.h"
#include "hal.h"
#include <string.h>
#include "pca9685.h"

#define MIDDLE (300)

uint8_t buff[2] = {0};

void pca9685_init(Pca9685 *self) {
  i2cAcquireBus(self->i2cp);
  i2c_writeSingleReg(self->i2cp, self->address, 0x00, 0x31);
  i2c_writeSingleReg(self->i2cp, self->address, 0xfe, 136);
  i2c_writeSingleReg(self->i2cp, self->address, 0x00, 0x21);
  i2cReleaseBus(self->i2cp);
}

void pca9685_middle(Pca9685 *self) {
  i2cAcquireBus(self->i2cp);
  uint8_t v[4] = {0, 0, MIDDLE & 0xff, (MIDDLE >> 8) & 0x0f};
  for (uint8_t i = 0; i < 16; i++) {
    i2c_writeMultipleReg(self->i2cp, self->address, 6 + 4 * i, v, 4);
  }
  i2cReleaseBus(self->i2cp);
}

void pca9685_setWidth(Pca9685 *self, uint8_t index, uint16_t width) {
  buff[0] = width & 0xff;
  buff[1] = (width >> 8) & 0x0f;
  i2c_writeMultipleReg(self->i2cp, self->address, 8 + 4 * index, buff, 2);
}
