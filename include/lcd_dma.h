#ifndef LCD_DMA_H
#define LCD_DMA_H

#include <stdint.h>

// Initialize SPI DMA after LCD_Setup()
void lcd_dma_init(void);

// Transfer a packed RGB565 image rectangle to the LCD
void lcd_dma_blit(int x, int y, int width, int height, const uint16_t *pixels);

#endif
