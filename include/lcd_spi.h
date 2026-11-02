#ifndef LCD_SPI_H
#define LCD_SPI_H

// SPI0 LCD pin assignments
#define PIN_SDI    19
#define PIN_CS     17
#define PIN_SCK    18
#define PIN_DC     20
#define PIN_nRESET 27

// Initialize SPI and GPIO pins for the LCD
void lcd_spi_init(void);

#endif
