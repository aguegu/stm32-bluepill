#ifndef UTILS_I2C_H_
#define UTILS_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

  void i2c_init(void);
  void i2c_writeSingleReg(I2CDriver *, uint8_t address, uint8_t start ,uint8_t data);
  void i2c_writeMultipleReg(I2CDriver *, uint8_t address, uint8_t start ,uint8_t *data, uint8_t length);
  void i2c_readReg(I2CDriver *, uint8_t address, uint8_t start, uint8_t *data, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_I2C_H_ */
