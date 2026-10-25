#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "lcd_spi.h"

// Initialize SPI for the LCD
void lcd_spi_init(void) {
    // Set control pins as regular GPIO outputs
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_DC, GPIO_FUNC_SIO);
    gpio_set_function(PIN_nRESET, GPIO_FUNC_SIO);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_nRESET, GPIO_OUT);

    gpio_put(PIN_CS, 1);        // CS high
    gpio_put(PIN_DC, 0);        // DC low
    gpio_put(PIN_nRESET, 1);    // RESET high

    // Configure SPI pins
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SDI, GPIO_FUNC_SPI);

    // Initialize SPI1 at 12 MHz
    spi_init(spi1, 12 * 1000 * 1000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}
