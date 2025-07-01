#pragma once

// 0	GPIO0	ADC1
// 1	GPIO1	ADC1
// 2	GPIO2	ADC1, boot mode / strapping pin
// 3	GPIO3	ADC1
// 4	GPIO4	ADC1, JTAG
// 5	GPIO5	JTAG
// 6	GPIO6	JTAG
// 7	GPIO7	JTAG
// 8	GPIO8	Blue status_led (inverted), boot mode / strapping pin
// 9	GPIO9	Boot mode / strapping pin, boot button
// 10	GPIO10
// 20	GPIO20
// 21	GPIO21

#define I2C_MASTER_SDA_IO 3                  ///< SDA
#define I2C_MASTER_SCL_IO 4                  ///< SCL
#define X9C103S_CS_PIN    5                  ///< GPIO для выбора чипа
#define X9C103S_UD_PIN    6                  ///< GPIO для направления изменения
#define X9C103S_INC_PIN   7                  ///< GPIO для шага изменения
#define LED_GPIO          CONFIG_BLINK_GPIO  ///< LED 8