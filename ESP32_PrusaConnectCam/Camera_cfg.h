/**
   @file Camera_cfg.h

   @brief Here is saved camera GPIO cfg for camera module OV2640

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _CAMERA_CFG_H_
#define _CAMERA_CFG_H_

// Uncomment this to switch from ESP32-Cam to ESP32-WROVER board.  You also have to do it in the
// mcu_config.h file.
// #define USE_ESP32_WROVER 1

#if USE_ESP32_WROVER
// OV2640 camera module pins (ESP32-WROVER)
#define PWDN_GPIO_NUM     32      ///< Power down control pin
#define RESET_GPIO_NUM    -1      ///< Reset control pin
#define XCLK_GPIO_NUM     21      ///< External clock pin
#define SIOD_GPIO_NUM     26      ///< SCCB: SI/O data pin
#define SIOC_GPIO_NUM     27      ///< SCCB: SI/O control pin
#define Y9_GPIO_NUM       35      ///< SCCB: Y9 pin
#define Y8_GPIO_NUM       34      ///< SCCB: Y8 pin
#define Y7_GPIO_NUM       39      ///< SCCB: Y7 pin
#define Y6_GPIO_NUM       36      ///< SCCB: Y6 pin
#define Y5_GPIO_NUM       19      ///< SCCB: Y5 pin
#define Y4_GPIO_NUM       18      ///< SCCB: Y4 pin
#define Y3_GPIO_NUM        5      ///< SCCB: Y3 pin
#define Y2_GPIO_NUM        4      ///< SCCB: Y2 pin
#define VSYNC_GPIO_NUM    25      ///< Vertical sync pin
#define HREF_GPIO_NUM     23      ///< Line sync pin
#define PCLK_GPIO_NUM     22      ///< Pixel clock pin

#define FLASH_GPIO_NUM    32       ///< Flash control pin

#else

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32      ///< Power down control pin
#define RESET_GPIO_NUM    -1      ///< Reset control pin
#define XCLK_GPIO_NUM      0      ///< External clock pin
#define SIOD_GPIO_NUM     26      ///< SCCB: SI/O data pin
#define SIOC_GPIO_NUM     27      ///< SCCB: SI/O control pin
#define Y9_GPIO_NUM       35      ///< SCCB: Y9 pin
#define Y8_GPIO_NUM       34      ///< SCCB: Y8 pin
#define Y7_GPIO_NUM       39      ///< SCCB: Y7 pin
#define Y6_GPIO_NUM       36      ///< SCCB: Y6 pin
#define Y5_GPIO_NUM       21      ///< SCCB: Y5 pin
#define Y4_GPIO_NUM       19      ///< SCCB: Y4 pin
#define Y3_GPIO_NUM       18      ///< SCCB: Y3 pin
#define Y2_GPIO_NUM        5      ///< SCCB: Y2 pin
#define VSYNC_GPIO_NUM    25      ///< Vertical sync pin
#define HREF_GPIO_NUM     23      ///< Line sync pin
#define PCLK_GPIO_NUM     22      ///< Pixel clock pin

#define FLASH_GPIO_NUM    4       ///< Flash control pin

#endif //USE_ESP32_WROVER

#endif

/* EOF */
