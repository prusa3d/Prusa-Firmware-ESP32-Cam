/**
   @file system.h

   @brief system library

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <WiFi.h>
#include <Update.h>
#include "Arduino.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include "esp32/rom/rtc.h"
#include <esp_task_wdt.h>

#include "mcu_cfg.h"
#include "var.h"
#include "cfg.h"
#include "log.h"
#include "Certificate.h"
#include "wifi_mngt.h"
#include "connect.h"
#include "serial_cfg.h"
#include "sys_led.h"

#define SYSTEM_MSG_UPDATE_DONE    "FW update successfully done! Please reboot the MCU."
#define SYSTEM_MSG_UPDATE_FAIL    "FW update failed! Please reboot MCU, and try again."
#define SYSTEM_MSG_UPDATE_PROCESS "FW update in progress"
#define SYSTEM_MSG_UPDATE_NO_FW   "No new FW version available!"

void System_Init();
void System_LoadCfg();
void System_CheckIfPsramIsUsed();
void System_Main();
void System_UpdateInit();

void System_CheckNewVersion();
void System_OtaCloudUpdate();
bool System_OtaUpdateStart();
void System_OtaUpdateProgressCB(int, int);
void System_OtaUpdateErrorCB(int);
void System_OtaUpdateEndCB();
void System_OtaUpdateStartCB();

String System_PrintMcuResetReason(int);
String System_printMcuResetReasonSimple();

void System_TaskWifiManagement(void *);
void System_TaskMain(void *);
void System_TaskCaptureAndSendPhoto(void *);
void System_TaskSdCardCheck(void *);
void System_TaskSerialCfg(void *);
void System_TaskStreamTelemetry(void *);
void System_TaskSysLed(void *);
void System_TaskWiFiWatchdog(void *);

#endif
/* EOF */