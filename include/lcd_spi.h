#ifndef LCD_SPI_H
#define LCD_SPI_H

// SPI LCD pin assignments
#define PIN_SDI    11
#define PIN_CS     9
#define PIN_SCK    10
#define PIN_DC     12
#define PIN_nRESET 13

// Initialize SPI and GPIO pins for the LCD
void lcd_spi_init(void);

#endif
