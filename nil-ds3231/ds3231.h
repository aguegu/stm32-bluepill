#ifndef DS3231_H_
#define DS3231_H_

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
  // uint8_t ds3231_getHexData(uint8_t index);


#ifdef __cplusplus
}
#endif

#endif /* DS3231_H_ */
