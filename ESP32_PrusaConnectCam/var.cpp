/**
   @file variable.cpp

   @brief Library with global variables

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "var.h"

WebBasicAuth_struct WebBasicAuth = { false, "", "" };
struct FirmwareUpdate_struct FirmwareUpdate = { "Ready", false, 0, 0, 0, false, false, "", "", "", false };
struct McuTemperature_struct McuTemperature = {0.0};

TaskHandle_t Task_CapturePhotoAndSend;
TaskHandle_t Task_WiFiManagement;
TaskHandle_t Task_SystemMain;
TaskHandle_t Task_SdCardCheck;
TaskHandle_t Task_SerialCfg;
TaskHandle_t Task_SystemTelemetry;
TaskHandle_t Task_SysLed;
TaskHandle_t Task_WiFiWatchdog;
//TaskHandle_t Task_SdCardFileRemove;

uint8_t StartRemoveSdCard = 0;
uint32_t SdCardRemoveTime = 0;

/* EOF */