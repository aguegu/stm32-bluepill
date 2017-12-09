#ifndef UTILS_PCA9685_H_
#define UTILS_PCA9685_H_

#include "i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    I2CDriver   *i2cp;
    uint8_t     address;
  } Pca9685;

  void pca9685_init(Pca9685 *);
  void pca9685_middle(Pca9685 *);
  void pca9685_setWidth(Pca9685 *self, uint8_t index, uint16_t width);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_PCA9685_H_ */
