/*
   This code is adapted for the ESP32-CAM board Ai Thinker version

   It's neccesary install support for ESP32 board to the arduino IDE. In the board manager we need add next link
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   Then we can install "ESP32 by Espressif Systems" board in the board manager.
   ESP32 lib version: 2.0.16 (ESP-IDF v4.4.7) by Espressif Systems

   This project uses other libraries. It is necessary to install them in the arduino IDE.
   - Library           - License  - Version - Link
   - ESPAsyncWebServer - LGPL 3.0 - 2.10.8  - https://github.com/mathieucarbou/ESPAsyncWebServer
   - AsyncTCP          - LGPL 3.0 - 3.1.4   - https://github.com/mathieucarbou/AsyncTCP
   - ArduinoJson       - MIT      - 7.0.4   - https://github.com/bblanchon/ArduinoJson
   - ArduinoUniqueID   - MIT      - 1.3.0   - https://github.com/ricaun/ArduinoUniqueID
   - ESP32             - LGPL 2.1 - 3.0.1   - https://github.com/espressif/arduino-esp32

   Arduino IDE configuration for the MCU are stored in the module_XXX.h file.

   When flashing the firmware to a new, empty ESP32-CAM device for the first time, it is necessary to use the 'Erase' function.
   This can be found under 'Tools' -> 'Erase all Flash Before Sketch Upload' -> 'Enable'.
   After the initial firmware upload to the MCU, it is possible to disable this option.
   If you do not disable this option, your camera configuration will continue to be erased from the flash memory
   after uploading new firmware from the Arduino IDE.

   Here is partitions table
     # Name,   Type, SubType, Offset,  Size, Flags
     nvs,      data, nvs,     0x9000,  0x5000,
     otadata,  data, ota,     0xe000,  0x2000,
     app0,     app,  ota_0,   0x10000, 0x1E0000,
     app1,     app,  ota_1,   0x1F0000,0x1E0000,
     spiffs,   data, spiffs,  0x3D0000,0x20000,
     coredump, data, coredump,0x3F0000,0x10000,

   Project: ESP32 PrusaConnect Camera
   Developed for: Prusa Research, prusa3d.com
   Author: Miroslav Pivovarsky
   e-mail: miroslav.pivovarsky@gmail.com
*/

/* includes */
#include "Arduino.h"
#include <esp_task_wdt.h>
#include <ESPmDNS.h>
#include "esp32-hal-cpu.h"

#include "WebServer.h"
#include "cfg.h"
#include "var.h"
#include "mcu_cfg.h"
#include "system.h"
#include "micro_sd.h"
#include "log.h"
#include "connect.h"
#include "wifi_mngt.h"
#include "stream.h"
#include "serial_cfg.h"

//#include "esp32s3eye_lcd.h"

void setup() {
  /* Serial port for debugging purposes */
  Serial.begin(SERIAL_PORT_SPEED);
  Serial.println(F("----------------------------------------------------------------"));
  Serial.println(F("Start MCU!"));
  Serial.println(F("Prusa ESP32-cam https://prusa3d.cz"));
  Serial.print(F("SW Version: "));
  Serial.println(SW_VERSION);
  Serial.print(F("Build: "));
  Serial.println(SW_BUILD);
#if (CONSOLE_VERBOSE_DEBUG == true)
  Serial.setDebugOutput(true);
#endif

  /* Init EEPROM */
  EEPROM.begin(EEPROM_SIZE);

  /* init system led */
  system_led.init();

  /* init micro SD card and logs */
  SystemLog.SetLogLevel((LogLevel_enum)EEPROM.read(EEPROM_ADDR_LOG_LEVEL));
  SystemLog.Init();

  /* init System lib */
  System_Init();

  /* read cfg from EEPROM */
  SystemConfig.Init();
  SystemConfig.CheckResetCfg();
  System_LoadCfg();
  Server_LoadCfg();
  SystemCamera.LoadCameraCfgFromEeprom();
  Connect.LoadCfgFromEeprom();
  SystemWifiMngt.LoadCfgFromEeprom();

  /* init WiFi mngt */
  SystemWifiMngt.Init();

  /* init camera interface */
  SystemCamera.Init();
  SystemCamera.CapturePhoto();
  SystemCamera.CaptureReturnFrameBuffer();

  /* init WEB server */
  Server_InitWebServer();

  /* init class for communication with PrusaConnect */
  Connect.Init();

  //Esp32Lcd LcdDisplay;
  //LcdDisplay.draw_color(0x000000);

  /* init wdg */
  SystemLog.AddEvent(LogLevel_Info, F("Init WDG"));
  esp_task_wdt_config_t twdt_config;
  twdt_config.timeout_ms = WDG_TIMEOUT;
  twdt_config.idle_core_mask = (1 << portNUM_PROCESSORS) - 1,    /* Bitmask of all cores */
  twdt_config.trigger_panic = true;
	
  esp_task_wdt_init(&twdt_config);       /* enable panic so ESP32 restarts */
  esp_task_wdt_reconfigure(&twdt_config);
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));                /* add current thread to WDT watch */
  esp_task_wdt_reset();                  /* reset wdg */

  /* init tasks */
  SystemLog.AddEvent(LogLevel_Info, F("Start tasks"));
  xTaskCreatePinnedToCore(System_TaskMain, "SystemNtpOtaUpdate", 6200, NULL, 1, &Task_SystemMain, 0);                           /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_SystemMain));
  xTaskCreatePinnedToCore(System_TaskCaptureAndSendPhoto, "CaptureAndSendPhoto", 4400, NULL, 2, &Task_CapturePhotoAndSend, 0);  /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_CapturePhotoAndSend));
  xTaskCreatePinnedToCore(System_TaskWifiManagement, "WiFiManagement", 2800, NULL, 3, &Task_WiFiManagement, 0);                 /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_WiFiManagement));
#if (true == ENABLE_SD_CARD)  
  xTaskCreatePinnedToCore(System_TaskSdCardCheck, "CheckMicroSdCard", 3000, NULL, 4, &Task_SdCardCheck, 0);                     /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_SdCardCheck));
#endif
  xTaskCreatePinnedToCore(System_TaskSerialCfg, "CheckSerialConfiguration", 2600, NULL, 5, &Task_SerialCfg, 0);                 /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_SerialCfg));
  xTaskCreatePinnedToCore(System_TaskStreamTelemetry, "PrintStreamTelemetry", 2200, NULL, 6, &Task_StreamTelemetry, 0);         /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_StreamTelemetry));
  xTaskCreatePinnedToCore(System_TaskSysLed, "SystemLed", 2100, NULL, 7, &Task_SysLed, 0);                                      /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_SysLed));
  xTaskCreatePinnedToCore(System_TaskWiFiWatchdog, "WiFiWatchdog", 2200, NULL, 8, &Task_WiFiWatchdog, 0);                       /*function, description, stack size, parameters, priority, task handle, core*/
  ESP_ERROR_CHECK(esp_task_wdt_add(Task_WiFiWatchdog));
  //xTaskCreatePinnedToCore(System_TaskSdCardRemove, "SdCardRemove", 3000, NULL, 9, &Task_SdCardFileRemove, 0);                   /*function, description, stack size, parameters, priority, task handle, core*/
  //esp_task_wdt_add(Task_SdCardFileRemove);

  SystemLog.AddEvent(LogLevel_Info, F("MCU configuration done"));
}

void loop() {
  /* reset wdg */
  esp_task_wdt_reset();
  delay(1000);
}

/* EOF */