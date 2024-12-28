/**
   @file module_ESP32-S3_Wroom_Freenove.h

   @brief Definition of the ESP32-S3 Wroom FreeNove board

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   Board configuration in the arduino IDE 2.3.2
   Tools -> Board -> ESP32 Arduino -> ESP32S3 Dev Module
   Tools -> USB CDC on BOOT -> Disabled
   Tools -> CPU Frequency -> 240MHz (WiFi/BT)
   Tools -> Core debug level -> None
   Tools -> USB DFU on BOOT -> Disable
   Tools -> Erase all Flash Before Sketch Upload -> Disable (first flash, new board = enable. otherwise = disable)
   Tools -> Events Run On -> Core 0
   Tools -> Flash Mode -> QIO 80MHz
   Tools -> Flash Size -> 8MB
   Tools -> Jtag Adapter -> Disable
   Tools -> Arduino Runs On -> Core 0
   Tools -> USB Firmware MSC On Boot -> Disable
   Tools -> Partition scheme -> Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
   Tools -> PSRAM -> OPI PSRAM
   Tools -> Upload Mode -> UART0 / Hardware CDC
   Tools -> Upload Speed -> 921600
   Tools -> USB Mode -> Hardware CDC and JTAG
   Tools -> Zigbee mode -> Disable

   https://freenove.com/fnk0085

   @bug: no know bug

*/

#pragma once

#include "mcu_cfg.h"

#ifdef ESP32_S3_WROOM_FREENOVE

/* --------------- CAMERA CFG -------------------*/
#define PWDN_GPIO_NUM               -1      ///< Power down control pin
#define RESET_GPIO_NUM              -1      ///< Reset control pin
#define SIOD_GPIO_NUM               4       ///< SCCB: SI/O data pin
#define SIOC_GPIO_NUM               5       ///< SCCB: SI/O control pin
#define Y2_GPIO_NUM                 11      ///< SCCB: Y2 pin
#define Y3_GPIO_NUM                 9       ///< SCCB: Y3 pin
#define Y4_GPIO_NUM                 8       ///< SCCB: Y4 pin
#define Y5_GPIO_NUM                 10      ///< SCCB: Y5 pin
#define Y6_GPIO_NUM                 12      ///< SCCB: Y6 pin
#define Y7_GPIO_NUM                 18      ///< SCCB: Y7 pin
#define Y8_GPIO_NUM                 17      ///< SCCB: Y8 pin
#define Y9_GPIO_NUM                 16      ///< SCCB: Y9 pin
#define VSYNC_GPIO_NUM              6       ///< Vertical sync pin
#define HREF_GPIO_NUM               7       ///< Line sync pin
#define PCLK_GPIO_NUM               13      ///< Pixel clock pin
#define XCLK_GPIO_NUM               15      ///< External clock pin

/* ------------------ MCU CFG  ------------------*/
#define BOARD_NAME                  F("ESP32-S3 WROOM Freenove") ///< Board name
#define ENABLE_BROWN_OUT_DETECTION  false  ///< Enable brown out detection
#define ENABLE_PSRAM                true   ///< Enable PSRAM

/* --------------- OTA UPDATE CFG  --------------*/
#define OTA_UPDATE_FW_FILE          PSTR("ESP32-S3-WROOM_FREENOVE.bin") ///< OTA update firmware file name
#define FW_STATUS_LED_PIN           2      ///< GPIO pin for status FW update LED
#define FW_STATUS_LED_LEVEL_ON      LOW   ///< GPIO pin level for status LED ON

/* --------------- FLASH LED CFG  ---------------*/
#define ENABLE_CAMERA_FLASH         true    ///< Enable camera flash function
#define CAMERA_FLASH_DIGITAL_CTRL   true   ///< Enable camera flash digital control
#define CAMERA_FLASH_PWM_CTRL       false   ///< Enable camera flash PWM control
#define CAMERA_FLASH_NEOPIXEL       true    ///< Enable camera flash NeoPixel control
#define FLASH_GPIO_NUM              14      ///< Flash control pin
#define FLASH_NEOPIXEL_LED_PIN      48      ///< External flash control pin. RGB LED NeoPixel
#define FLASH_OFF_STATUS            0       ///< PWM intensity LED for OFF. 0-2^FLASH_PWM_RESOLUTION = 0-255
#define FLASH_ON_STATUS             205     ///< PWM intensity LED for ON. limitation to 80%. 2^FLASH_PWM_RESOLUTION * 0.8% = 204
#define FLASH_PWM_FREQ              2000    ///< frequency of pwm [240MHz / (100 prescale * pwm cycles)] = frequency
#define FLASH_PWM_CHANNEL           0       ///< channel 0
#define FLASH_PWM_RESOLUTION        8       ///< range 1-20bit. 8bit = 0-255 range

/* --------------- SD CARD CFG  ---------------*/
#define ENABLE_SD_CARD              true    ///< Enable SD card function
#define SD_PIN_CLK                  39      ///< GPIO pin for SD card clock
#define SD_PIN_CMD                  38      ///< GPIO pin for SD card command
#define SD_PIN_DATA0                40      ///< GPIO pin for SD card data 0

/* ---------- RESET CFG CONFIGURATION  ----------*/
#define CFG_RESET_PIN               21      ///< GPIO 12 is for reset CFG to default
#define CFG_RESET_LED_PIN           2       ///< GPIO for indication of reset CFG
#define CFG_RESET_LED_LEVEL_ON      HIGH    ///< GPIO pin level for status LED ON

/* -------------- STATUS LED CFG ----------------*/
#define STATUS_LED_ENABLE           false    ///< enable/disable status LED
#define STATUS_LED_GPIO_NUM         2       ///< GPIO pin for status LED
#define STATUS_LED_OFF_PIN_LEVEL    LOW     ///< GPIO pin level for status LED ON

/* -------------- DHT SENSOR CFG ----------------*/
#define DHT_SENSOR_ENABLE           false   ///< enable/disable DHT sensor
#define DHT_SENSOR_PIN              47     ///< GPIO pin for DHT sensor

#endif  // AI_THINKER_ESP32_CAM
/* EOF */