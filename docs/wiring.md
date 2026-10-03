# Reference wiring

This reference uses a generic ESP32 development board. Confirm the pinout, voltage limits, and
strapping pins for your exact board and adjust `include/config.h` as needed. Power must be off while
wiring.

| Device | Signal | ESP32 reference pin | Notes |
|---|---|---:|---|
| BME280 | SDA / SCL | GPIO 21 / 22 | I2C address `0x76`; use a 3.3 V-compatible breakout |
| DS3231 | SDA / SCL | GPIO 21 / 22 | Shared I2C bus; verify breakout pull-ups are to 3.3 V |
| microSD | CS | GPIO 5 | SPI; SCK 18, MISO 19, MOSI 23 on this reference |
| Battery divider | midpoint | GPIO 34 | ADC input only |
| Optional SSD1306 | SDA / SCL | GPIO 21 / 22 | Shared I2C bus; display is not required |

## Battery measurement safety

Never connect a Li-ion/LiPo cell directly to an ESP32 ADC pin. Use a correctly sized resistor
divider so the highest possible cell voltage remains below the board's ADC/input limit. The default
2:1 divider is represented by `kBatteryDividerRatio = 2.0`; verify actual resistor values with a
multimeter and adjust the calibration factor. This project monitors a battery only—it does not
charge, protect, or balance one. Use a protected cell and a suitable external charger/power circuit.

The ADC-derived percentage is a coarse voltage estimate and varies with cell chemistry, load,
temperature, ADC calibration, and divider tolerance.

