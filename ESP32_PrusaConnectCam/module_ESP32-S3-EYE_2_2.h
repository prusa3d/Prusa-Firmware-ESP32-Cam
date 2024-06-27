/**
   @file module_ESP32-S3-EYE_2_2.h

   @brief Definition of the ESP32-S3 EYE 2.2 module

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   Board configuration in the arduino IDE 2.3.2
   Tools -> Board -> ESP32 Arduino -> ESP32S3 Dev Module
   Tools -> USB CDC on BOOT -> Enabled
   Tools -> CPU Frequency -> 240MHz (WiFi/BT)
   Tools -> Core debug level -> None
   Tools -> USB DFU on BOOT -> Disable
   Tools -> Events Run On -> Core 0
   Tools -> Flash Mode -> DIO 80MHz
   Tools -> Flash Size -> 8MB
   Tools -> Jtag Adapter -> Disable
   Tools -> Arduino Runs On -> Core 0
   Tools -> USB Firmware MSC On Boot -> Disable
   Tools -> Partition scheme -> Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
   Tools -> PSRAM -> OPI PSRAM
   Tools -> Upload Mode -> USB-OTG CDC (TinyUSB)
   Tools -> Upload Speed -> 921600
   Tools -> USB Mode -> Hardware CDC and JTAG
   Tools -> Zigbee mode -> Disable

   https://github.com/espressif/esp-who/blob/master/docs/en/get-started/ESP32-S3-EYE_Getting_Started_Guide.md

   @bug: Currently SW don't work with this DEV board

*/

#pragma once

#include "mcu_cfg.h"

#ifdef CAMERA_MODEL_ESP32_S3_EYE_2_2

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
#define ENABLE_BROWN_OUT_DETECTION  false   ///< Enable brown out detection
#define ENABLE_PSRAM                true    ///< Enable PSRAM   

/* --------------- OTA UPDATE CFG  --------------*/
#define OTA_UPDATE_FW_FILE          PSTR("ESP32_S3_EYE_22_PrusaConnectCam.ino.bin") ///< OTA update firmware file name

/* --------------- FLASH LED CFG  ---------------*/
#define ENABLE_CAMERA_FLASH         false   ///< Enable camera flash function
#define FLASH_GPIO_NUM              4       ///< Flash control pin
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
#define CFG_RESET_PIN               1       ///< GPIO 16 is for reset CFG to default

/* -------------- STATUS LED CFG ----------------*/
#define STATUS_LED_ENABLE           true    ///< enable/disable status LED
#define STATUS_LED_GPIO_NUM         3       ///< GPIO pin for status LED
#define STATUS_LED_OFF_PIN_LEVEL    HIGH    ///< GPIO pin level for status LED ON

#endif  // ESP32_WROVER_DEV
/* EOF */