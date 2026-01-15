#ifndef GPIO_MAP_H
#define GPIO_MAP_H

enum GPIO_PINS {
    INT_IMU     = 2,  // IMU interrupt
    OTG         = 4,  // USB On-The-Go
    INT_CHRG    = 5,  // Charger interrupt
    SDA_I2C     = 6,  // I2C Data
    SCL_I2C     = 7,  // I2C Clock
    GNSS_WKP    = 8,  // GNSS Wake-up
    LED_IO9     = 9,  // User LED
    CS_SPI      = 10, // SPI Chip Select
    MOSI_SPI    = 11, // SPI Master Out Slave In
    CLK_SPI     = 12, // SPI Clock
    LED_IO13    = 13, // User LED
    GNSS_RST    = 14, // GNSS Reset
    DSEL        = 15, // Charger USB d+/d- selection status
    STAT        = 16, // Charge Status, Charging in progress = LED ON,  Charge complete = LED OFF,  Charge fault = LED Blinking
    EN_LS       = 17, // Enable Load Switch
    INT_UCON    = 18, // Interrupt USB Controller
    LATCH_SR    = 21, // Latch Shift Register
    DBNCD_0     = 35, // Debounced Input 0
    DBNCD_1     = 36, // Debounced Input 1
    LED_EXT     = 37, // External LED
    DBNCD_2     = 38, // Debounced Input 2
    SI_SR       = 39, // Shift register serial input
    CLK_SR      = 40, // Shift register clock
    FREE_IO41   = 41, // Free General Purpose IO
    FREE_IO42   = 42, // Free General Purpose IO
    GNSS_RX     = 43, // UART_TX
    GNSS_TX     = 44, // UART_RX
    FREE_IO48   = 48, // Free General Purpose IO
};

#endif // GPIO_MAP_H