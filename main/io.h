#pragma once

#include "driver/gpio.h"

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

#define LCD_SCLK_PIN     GPIO_NUM_5
#define LCD_MOSI_PIN     GPIO_NUM_6
#define LCD_RST_PIN      GPIO_NUM_7
#define LCD_DC_PIN       GPIO_NUM_8
// #define LED_GPIO         GPIO_NUM_8   // GPIO8 для светодиода
#define LCD_CS_PIN       GPIO_NUM_9
#define LCD_BK_LIGHT_PIN GPIO_NUM_10

#define CAN_TX_PIN       GPIO_NUM_20  // GPIO20 для CAN TX
#define CAN_RX_PIN       GPIO_NUM_21  // GPIO21 для CAN RX
