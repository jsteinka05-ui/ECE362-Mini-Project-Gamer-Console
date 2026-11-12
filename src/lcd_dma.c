
#include "lcd_dma.h"
#include "lcd_spi.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stddef.h>

static int tx_channel = -1;
static int rx_channel = -1;
static uint16_t rx_discard = 0;

static void spi_byte(uint8_t value, int is_data) {
    while (spi_is_busy(spi0)) {
        tight_loop_contents();
    }

    gpio_put(PIN_DC, is_data);
    spi_write_blocking(spi0, &value, 1);
}

static void send_window(int x, int y, int width, int height) {
    int x2 = x + width - 1;
    int y2 = y + height - 1;

    spi_byte(0x2A, 0);
    spi_byte((uint8_t)(x >> 8), 1);
    spi_byte((uint8_t)x, 1);
    spi_byte((uint8_t)(x2 >> 8), 1);
    spi_byte((uint8_t)x2, 1);

    spi_byte(0x2B, 0);
    spi_byte((uint8_t)(y >> 8), 1);
    spi_byte((uint8_t)y, 1);
    spi_byte((uint8_t)(y2 >> 8), 1);
    spi_byte((uint8_t)y2, 1);

    spi_byte(0x2C, 0);

    while (spi_is_busy(spi0)) {
        tight_loop_contents();
    }

    gpio_put(PIN_DC, 1);
}

void lcd_dma_init(void) {
    if (tx_channel >= 0) return;

    tx_channel = dma_claim_unused_channel(true);
    rx_channel = dma_claim_unused_channel(true);
}

void lcd_dma_blit(int x, int y, int width, int height, const uint16_t *pixels) {
    if (width <= 0 || height <= 0 || pixels == NULL) return;
    if (x < 0 || y < 0 || x + width > 240 || y + height > 320) return;
    if (tx_channel < 0 || rx_channel < 0) return;

    // Never use SPI DMA and regular LCD commands simultaneously
    while (spi_is_busy(spi0)) {
        tight_loop_contents();
    }

    gpio_put(PIN_CS, 0);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    send_window(x, y, width, height);

    spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    dma_channel_config tx = dma_channel_get_default_config((uint)tx_channel);
    channel_config_set_transfer_data_size(&tx, DMA_SIZE_16);
    channel_config_set_read_increment(&tx, true);
    channel_config_set_write_increment(&tx, false);
    channel_config_set_dreq(&tx, spi_get_dreq(spi0, true));

    dma_channel_config rx = dma_channel_get_default_config((uint)rx_channel);
    channel_config_set_transfer_data_size(&rx, DMA_SIZE_16);
    channel_config_set_read_increment(&rx, false);
    channel_config_set_write_increment(&rx, false);
    channel_config_set_dreq(&rx, spi_get_dreq(spi0, false));

    int count = width * height;

    dma_channel_configure((uint)rx_channel, &rx, &rx_discard, &spi_get_hw(spi0)->dr, count, false);
    dma_channel_configure((uint)tx_channel, &tx, &spi_get_hw(spi0)->dr, pixels, count, false);

    dma_start_channel_mask((1u << tx_channel) | (1u << rx_channel));

    dma_channel_wait_for_finish_blocking((uint)rx_channel);
    dma_channel_wait_for_finish_blocking((uint)tx_channel);

    while (spi_is_busy(spi0)) {
        tight_loop_contents();
    }

    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_put(PIN_CS, 1);
}
