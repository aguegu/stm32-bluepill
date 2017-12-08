#ifndef UTILS_DS3231_H_
#define UTILS_DS3231_H_

#include "i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    I2CDriver   *i2cp;
    uint8_t     address;
    uint8_t     data[7];
  } Ds3231;

  void ds3231_init(Ds3231 *);
  void ds3231_refresh(Ds3231 *);
  void ds3231_setTime(Ds3231 *, uint8_t * data, uint8_t length);

  // void i2c_writeSingleReg(I2CDriver *, uint8_t address, uint8_t start ,uint8_t data);
  // void i2c_writeMultipleReg(I2CDriver *, uint8_t address, uint8_t start ,uint8_t *data, uint8_t length);
  // void i2c_readReg(I2CDriver *, uint8_t address, uint8_t start, uint8_t *data, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_DS3231_H_ */
