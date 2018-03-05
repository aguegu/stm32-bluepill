#include "ch.h"
#include "hal.h"
#include "string.h"
#include "ssd1306_i2c.h"

uint8_t raw[BUFFER_LEN_RAW];
uint8_t * buffer = raw + 3;
I2CDriver * i2cp;

static const uint8_t font[30] = {
  0x3f, 0x21, 0x3f,
  0x22, 0x3f, 0x20,
  0x3d, 0x25, 0x27,
  0x25, 0x25, 0x3f,
  0x0f, 0x08, 0x3f,
  0x27, 0x25, 0x3d,
  0x3f, 0x25, 0x3d,
  0x01, 0x01, 0x3f,
  0x3f, 0x25, 0x3f,
  0x2f, 0x29, 0x3f,
};

void ssd1306_init(I2CDriver *i2c) {
  raw[0] = 0x80;
  raw[1] = 0xb0;
  raw[2] = 0x40;
  i2cp = i2c;

  i2cAcquireBus(i2cp);

  uint8_t s[27] = {0x00, 0xae, 0xd5, 0x80, 0xa8, 63, 0x40, 0x8d, 0x14, 0x20, 0x00, 0xa1, 0xc8, 0xda, 0x12, 0x81, 0x8f, 0xd9, 0x02, 0xdb, 0x40, 0xa4, 0xa6, 0x2e, 0xaf, 0x00, 0x10};

  i2cMasterTransmit(i2cp, 0x3c,
                          s, 27, NULL, 0);

  i2cReleaseBus(i2cp);
}

void ssd1306_display() {
  i2cAcquireBus(i2cp);

  i2cMasterTransmit(i2cp, 0x3c,
                        raw, BUFFER_LEN_RAW, NULL, 0);
  i2cReleaseBus(i2cp);
}

void ssd1306_clear(void) {
  memset(buffer, 0x00, BUFFER_LEN);
}

void ssd1306_frame(void) {
  for (uint16_t i=0; i<16; i++) {
    uint16_t offset = (i << 6) + (i & 0x01) * 28;
    memcpy(buffer + offset, font + i * 3, 3);
    buffer[offset + 5] = 0x28;
  }
}

void ssd1306_putnumber(uint16_t index, uint16_t val) {
  uint8_t st = 32;
  uint16_t offset = (index << 6) + (index & 0x01) * 28;
  uint8_t i = 0;
  do {
    uint8_t x = val % 10;
    memcpy(buffer + offset + st, font + x * 3, 3);
    val /= 10;
    st -= 4;
    i++;
  } while (val);

  st += 3;
  while (st > 5) {
    buffer[offset + st] = 0;
    st--;
  }
}