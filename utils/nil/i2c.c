#include "ch.h"
#include "hal.h"
// #include <stdlib.h>
#include <string.h>
#include "i2c.h"

#define HEAP_SIZE (32)

static memory_heap_t _heap;
static CH_HEAP_AREA(ic2heap, HEAP_SIZE);

void i2c_init(void) {
  chHeapObjectInit(&_heap, ic2heap, sizeof(ic2heap));
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
  chHeapFree(s);
  osalDbgCheck(MSG_OK == status);
}

void i2c_readReg(I2CDriver * i2cp, uint8_t address, uint8_t start, uint8_t *data, uint8_t length) {
  msg_t status = i2cMasterTransmit(i2cp, address, &start, 1, data, length);
  osalDbgCheck(MSG_OK == status);
}

// uint8_t ds3231_getHexData(uint8_t index) {
//   return data[index] & 0x7f;
// }
