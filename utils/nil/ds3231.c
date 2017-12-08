#include "ch.h"
#include "hal.h"
#include <string.h>
#include "ds3231.h"


void ds3231_init(Ds3231 *self) {
  i2cAcquireBus(self->i2cp);
  i2c_writeSingleReg(self->i2cp, self->address, 0x0e, 0x9c);
  i2c_writeSingleReg(self->i2cp, self->address, 0x0f, 0x00);
  i2cReleaseBus(self->i2cp);
}

void ds3231_setTime(Ds3231 *self, uint8_t * data, uint8_t length) {
  i2cAcquireBus(self->i2cp);
  i2c_writeMultipleReg(self->i2cp, self->address, 0, data, length);
  i2cReleaseBus(self->i2cp);
}

void ds3231_refresh(Ds3231 *self) {
  i2cAcquireBus(self->i2cp);
  i2c_readReg(self->i2cp, self->address, 0, self->data, 7);
  i2cReleaseBus(self->i2cp);
}

// uint8_t ds3231_getHexData(uint8_t index) {
//   return data[index] & 0x7f;
// }
