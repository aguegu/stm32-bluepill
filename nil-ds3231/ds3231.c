#include "ch.h"
#include "hal.h"
// #include <stdlib.h>
#include <string.h>
#include "ds3231.h"

// msg_t status = MSG_OK;
// #define tmo = MS2ST(4);
// uint8_t data[7];

// #define ALLOC_SIZE 16
#define HEAP_SIZE (32)

static memory_heap_t _heap;
static CH_HEAP_AREA(myheap, HEAP_SIZE);

void ds3231_init(Ds3231 *self) {
  uint8_t s[4] = {0x9c, 0x00};

  chHeapObjectInit(&_heap, myheap, sizeof(myheap));
  i2cAcquireBus(self->i2cp);
  // i2c_writeSingleReg(self->i2cp, self->address, 0x0e, 0x9c);
  // i2c_writeSingleReg(self->i2cp, self->address, 0x0f, 0x00);
  i2c_writeMultipleReg(self->i2cp, self->address, 0x0e, s, 2);
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

void i2c_writeSingleReg(I2CDriver *i2cp, uint8_t address, uint8_t start, uint8_t data) {
  uint8_t * s = (uint8_t *)chHeapAlloc(&_heap, 2);
  s[0] = start;
  s[1] = data;
  msg_t status = i2cMasterTransmit(i2cp, address, s, 2, NULL, 0);
  chHeapFree(s);
  osalDbgCheck(MSG_OK == status);
}

void i2c_writeMultipleReg(I2CDriver * i2cp, uint8_t address, uint8_t start, uint8_t *data, uint8_t length) {
  uint8_t * s = (uint8_t *)chHeapAlloc(&_heap, length + 1);
  s[0] = start;
  memcpy(s + 1, data, length);
  msg_t status = i2cMasterTransmit(i2cp, address, s, length + 1, NULL, 0);
  osalDbgCheck(MSG_OK == status);
}

void i2c_readReg(I2CDriver * i2cp, uint8_t address, uint8_t start, uint8_t *data, uint8_t length) {
  msg_t status = i2cMasterTransmit(i2cp, address, &start, 1, data, length);
  osalDbgCheck(MSG_OK == status);
}

// uint8_t ds3231_getHexData(uint8_t index) {
//   return data[index] & 0x7f;
// }
