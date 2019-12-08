#include "ch.h"
#include "hal.h"

static PWMConfig pwmcfg = {
  1000000,
  20000,
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  TIM_DIER_UDE,
#if STM32_PWM_USE_ADVANCED
  0
#endif
};

static THD_WORKING_AREA(waThread1, 0);
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");

  palSetPadMode(GPIOC, GPIOC_LED, PAL_MODE_OUTPUT_OPENDRAIN); // only 50MHz in GPIOv1
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(250);
  }
}

// #define WS2812_LED_N            (16)
// #define WS2812_RESET_BIT_N      (50)
// #define WS2812_COLOR_BIT_N      (WS2812_LED_N*24)                           /**< Number of data bits */
// #define WS2812_BIT_N            (WS2812_COLOR_BIT_N + WS2812_RESET_BIT_N)
// #define WS2812_DUTYCYCLE_0      22
// #define WS2812_DUTYCYCLE_1      56
// #define WS2812_DMA_STREAM       STM32_DMA_STREAM_ID(1, 3)
// #define WS2812_DMA_CHANNEL      3
//
// static uint8_t ws2812_frame_buffer[WS2812_BIT_N];

int main(void) {
  halInit();
  chSysInit();

  // uint16_t i;
  // for (i = 0; i < WS2812_COLOR_BIT_N; i++) {
  //   ws2812_frame_buffer[i]                       = WS2812_DUTYCYCLE_0;   // All color bits are zero duty cycle
  // }
  // for (i = 0; i < WS2812_RESET_BIT_N; i++) {
  //   ws2812_frame_buffer[i + WS2812_COLOR_BIT_N]  = 0;                    // All reset bits are zero
  // }

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

  palSetPadMode(GPIOA, 6, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  pwmStart(&PWMD3, &pwmcfg);
  pwmEnableChannel(&PWMD3, 0, 1500);

  dmaStreamAllocI(STM32_UART_USART1_RX_DMA_STREAM,
                                     STM32_UART_USART1_IRQ_PRIORITY,
                                     NULL, NULL);

  // dmaInit(); // Joe added this
  // dmaStreamAlloc(WS2812_DMA_STREAM, 10, NULL, NULL);
  // dmaStreamAllocI(STM32_DMA_STREAM_ID(3, 3), 10, NULL, NULL);
  // dmaStreamSetPeripheral(STM32_DMA_STREAM_ID(1, 3), &(PWMD3.tim->CCR[0]));  // Ziel ist der An-Zeit im Cap-Comp-Register
  // dmaStreamSetMemory0(WS2812_DMA_STREAM, ws2812_frame_buffer);
  // dmaStreamSetTransactionSize(WS2812_DMA_STREAM, WS2812_BIT_N);
  // dmaStreamSetMode(WS2812_DMA_STREAM,
  //   STM32_DMA_CR_CHSEL(WS2812_DMA_CHANNEL) | STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD |
  //   STM32_DMA_CR_MINC | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(3));
   // M2P: Memory 2 Periph; PL: Priority Level

  // Start DMA
  // dmaStreamEnable(WS2812_DMA_STREAM);

  while (true) {
    chThdSleepSeconds(1);

  }

  return 0;
}
