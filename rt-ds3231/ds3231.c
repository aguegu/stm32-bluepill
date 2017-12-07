#include "ch.h"
#include "hal.h"

#include "ds3231.h"

msg_t status = MSG_OK;
systime_t tmo = TIME_MS2I(4);
uint8_t data[7];

void ds3231_init(I2CDriver *i2cp) {
  uint8_t s[4] = {0x0e, 0x9c, 0x0f, 0x00};
  i2cAcquireBus(i2cp);
  status = i2cMasterTransmitTimeout(i2cp, 0x68,
                          s, 2, NULL, 0, tmo);
  osalDbgCheck(MSG_OK == status);
  status = i2cMasterTransmitTimeout(i2cp, 0x68,
                          s+2, 2, NULL, 0, tmo);
  osalDbgCheck(MSG_OK == status);
  i2cReleaseBus(i2cp);
}

void ds3231_refresh(I2CDriver *i2cp) {
  i2cAcquireBus(i2cp);
  uint8_t addr = 0x00;
  status = i2cMasterTransmitTimeout(i2cp, 0x68,
                          &addr, 1, data, 7, tmo);
  osalDbgCheck(MSG_OK == status);
  i2cReleaseBus(i2cp);
}

uint8_t ds3231_getHexData(uint8_t index) {
  return data[index] & 0x7f;
}
