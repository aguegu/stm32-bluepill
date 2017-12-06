#ifndef DS3231_H_
#define DS3231_H_

#ifdef __cplusplus
extern "C" {
#endif
  void ds3231_init(I2CDriver *);
  void ds3231_refresh(I2CDriver *);
  uint8_t ds3231_getHexData(uint8_t index);
#ifdef __cplusplus
}
#endif

#endif /* DS3231_H_ */
