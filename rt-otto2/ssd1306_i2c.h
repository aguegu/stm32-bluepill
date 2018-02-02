#ifndef SSD1306_H_
#define SSD1306_H_

#ifdef __cplusplus
extern "C" {
#endif

  #define WIDTH		128
  #define HEIGHT  32
  #define BUFFER_LEN (WIDTH * HEIGHT >> 3)
  #define BUFFER_LEN_RAW (BUFFER_LEN + 3)

  void ssd1306_init(I2CDriver *);
  void ssd1306_display(void);
  void ssd1306_clear(void);
  void ssd1306_frame(void);
  void ssd1306_putnumber(uint16_t index, uint16_t val);


#ifdef __cplusplus
}
#endif

#endif /* SSD1306_H_ */
