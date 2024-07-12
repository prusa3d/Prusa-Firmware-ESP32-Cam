/**
   @file module_XIAO_ESP32-S3-cam.h

   @brief Definition of the XIAO ESP32-S3 sense cam

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html
	 https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
   https://github.com/limengdu/SeeedStudio-XIAO-ESP32S3-Sense-camera
   https://github.com/Seeed-Studio/XIAO_Series


   Board configuration in the arduino IDE 2.3.2
   Tools -> Board -> ESP32 Arduino -> XIAO_ESP32S3
   Tools -> USB CDC on BOOT -> Enabled
   Tools -> CPU Frequency -> 240MHz (WiFi)
   Tools -> Core debug level -> None
   Tools -> USB DFU on BOOT -> Disable
   Tools -> Erase all Flash Before Sketch Upload -> Disable (first flash, new board = enable. otherwise = disable)
   Tools -> Events Run On -> Core 1
   Tools -> Flash Mode -> QIO 80MHz
   Tools -> Flash Size -> 8MB
   Tools -> Jtag Adapter -> Disable
   Tools -> Arduino Runs On -> Core 1
   Tools -> USB Firmware MSC On Boot -> Disable
   Tools -> Partition scheme -> 3MB APP/1.5MB SPIFFS
   Tools -> PSRAM -> OPI PSRAM
   Tools -> Upload Mode -> UART0 / Hardware CDC
   Tools -> Upload Speed -> 921600
   Tools -> USB Mode -> Hardware CDC and JTAG

   @bug: 

*/

#pragma once

#include "mcu_cfg.h"

#ifdef CAMERA_MODEL_XIAO_ESP32_S3_CAM

/* --------------- CAMERA CFG -------------------*/
#define PWDN_GPIO_NUM               -1      ///< Power down control pin
#define RESET_GPIO_NUM              -1      ///< Reset control pin
#define SIOD_GPIO_NUM               40      ///< SCCB: SI/O data pin
#define SIOC_GPIO_NUM               39      ///< SCCB: SI/O control pin
#define Y2_GPIO_NUM                 15      ///< SCCB: Y2 pin
#define Y3_GPIO_NUM                 17      ///< SCCB: Y3 pin
#define Y4_GPIO_NUM                 18      ///< SCCB: Y4 pin
#define Y5_GPIO_NUM                 16      ///< SCCB: Y5 pin
#define Y6_GPIO_NUM                 14      ///< SCCB: Y6 pin
#define Y7_GPIO_NUM                 12      ///< SCCB: Y7 pin
#define Y8_GPIO_NUM                 11      ///< SCCB: Y8 pin
#define Y9_GPIO_NUM                 48      ///< SCCB: Y9 pin
#define VSYNC_GPIO_NUM              38      ///< Vertical sync pin
#define HREF_GPIO_NUM               47      ///< Line sync pin
#define PCLK_GPIO_NUM               13      ///< Pixel clock pin
#define XCLK_GPIO_NUM               10      ///< External clock pin

/* ------------------ MCU CFG  ------------------*/
#define BOARD_NAME                  F("XIAO ESP32-S3 Sense cam") ///< Board name
#define ENABLE_BROWN_OUT_DETECTION  false   ///< Enable brown out detection
#define ENABLE_PSRAM                true    ///< Enable PSRAM   

/* --------------- OTA UPDATE CFG  --------------*/
#define OTA_UPDATE_FW_FILE          PSTR("XIAO_ESP32S3.bin") ///< OTA update firmware file name
#define FW_STATUS_LED_PIN           21      ///< GPIO pin for status FW update LED
#define FW_STATUS_LED_LEVEL_ON      LOW    ///< GPIO pin level for status LED ON

/* --------------- FLASH LED CFG  ---------------*/
#define ENABLE_CAMERA_FLASH         true    ///< Enable camera flash function
#define CAMERA_FLASH_DIGITAL_CTRL   true    ///< Enable camera flash digital control
#define CAMERA_FLASH_PWM_CTRL       false   ///< Enable camera flash PWM control
#define FLASH_GPIO_NUM              4       ///< Flash control pin. This is pin for enable LCD backlight
#define FLASH_OFF_STATUS            LOW     ///< value for flash OFF
#define FLASH_ON_STATUS             HIGH    ///< value for flash ON
//#define FLASH_PWM_FREQ              2000    ///< frequency of pwm [240MHz / (100 prescale * pwm cycles)] = frequency
//#define FLASH_PWM_CHANNEL           0       ///< channel 0
//#define FLASH_PWM_RESOLUTION        8       ///< range 1-20bit. 8bit = 0-255 range

/* --------------- SD CARD CFG  ---------------*/
#define ENABLE_SD_CARD              true    ///< Enable SD card function
#define SD_PIN_CLK                  7       ///< GPIO pin for SD card clock
#define SD_PIN_CMD                  9      ///< GPIO pin for SD card command
#define SD_PIN_DATA0                8       ///< GPIO pin for SD card data 0

/* ---------- RESET CFG CONFIGURATION  ----------*/
#define CFG_RESET_PIN               2       ///< GPIO 1 is for reset CFG to default. This is button UP+`
#define CFG_RESET_LED_PIN           21      ///< GPIO for indication of reset CFG
#define CFG_RESET_LED_LEVEL_ON      LOW     ///< GPIO pin level for status LED ON

/* -------------- STATUS LED CFG ----------------*/
#define STATUS_LED_ENABLE           true    ///< enable/disable status LED
#define STATUS_LED_GPIO_NUM         21      ///< GPIO pin for status LED
#define STATUS_LED_OFF_PIN_LEVEL    LOW    ///< GPIO pin level for status LED ON

/* -------------- DHT SENSOR CFG ----------------*/
#define DHT_SENSOR_ENABLE           true   ///< enable/disable DHT sensor
#define DHT_SENSOR_PIN              1      ///< GPIO pin for DHT sensor

#endif  // CAMERA_MODEL_XIAO_ESP32_S3_CAM
/* EOF */
