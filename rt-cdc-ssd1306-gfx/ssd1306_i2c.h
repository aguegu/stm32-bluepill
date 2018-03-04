#ifndef SSD1306_H_
#define SSD1306_H_

#ifdef __cplusplus
extern "C" {
#endif

  #define WIDTH		128
  #define HEIGHT  32
  #define BYTES_PER_COL (HEIGHT >> 3)
  #define BUFFER_LEN (WIDTH * BYTES_PER_COL)
  #define BUFFER_LEN_RAW (BUFFER_LEN + 3)

  void ssd1306_init(I2CDriver *);
  void ssd1306_display(void);
  void ssd1306_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_H_ */
