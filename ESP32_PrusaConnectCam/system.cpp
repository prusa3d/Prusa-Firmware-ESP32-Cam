/**
   @file system.cpp

   @brief system library

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "system.h"

/**
   @brief Function for init system library
   @param none
   @return none
*/
void System_Init() {
  SystemLog.AddEvent(LogLevel_Info, F("Init system lib"));
  /* show last reset status */
  String reason_simple = System_printMcuResetReasonSimple();
  SystemLog.AddEvent(LogLevel_Warning, "CPU reset reason: " + reason_simple);

  String reason_core0 = System_PrintMcuResetReason(rtc_get_reset_reason(0));
  String reason_core1 = System_PrintMcuResetReason(rtc_get_reset_reason(1));
  SystemLog.AddEvent(LogLevel_Warning, "CPU0 reset reason: " + reason_core0);
  SystemLog.AddEvent(LogLevel_Warning, "CPU1 reset reason: " + reason_core1);

  SystemLog.AddEvent(LogLevel_Info, "MCU Temperature: " + String(temperatureRead()) + " *C");
  SystemLog.AddEvent(LogLevel_Info, "Internal Total heap: " + String(ESP.getHeapSize()) + " ,internal Free Heap: " + String(ESP.getFreeHeap()));
  SystemLog.AddEvent(LogLevel_Info, "SPIRam Total heap: " + String(ESP.getPsramSize()) + " ,SPIRam Free Heap: " + String(ESP.getFreePsram()));
  SystemLog.AddEvent(LogLevel_Info, "ChipRevision: " + String(ESP.getChipRevision()) + " ,Cpu Freq: " + String(ESP.getCpuFreqMHz()) + " ,SDK Version: " + String(ESP.getSdkVersion()));
  SystemLog.AddEvent(LogLevel_Info, "Flash Size: " + String(ESP.getFlashChipSize()) + " ,Flash Speed " + String(ESP.getFlashChipSpeed()));
  System_CheckIfPsramIsUsed();
}

/**
   @brief Function for load configuration from EEPROM
   @param none
   @return none
*/
void System_LoadCfg() {

}

/**
   @brief Function for check if PSRAM is used
   @param none
   @return none
*/
void System_CheckIfPsramIsUsed() {
  if (psramFound()) {
    SystemLog.AddEvent(LogLevel_Info, F("PSRAM is used."));
    void *ptr = malloc(100);

    if (ptr != NULL) {
      if (esp_ptr_external_ram(ptr)) {
        SystemLog.AddEvent(LogLevel_Info, F("malloc/new is using SPIRAM"));
      } else {
        SystemLog.AddEvent(LogLevel_Info, F("malloc/new is not using SPIRAM"));
      }
      free(ptr);
    } else {
      SystemLog.AddEvent(LogLevel_Info, F("Failed to allocate memory"));
    }
  } else {
    SystemLog.AddEvent(LogLevel_Info, F("PSRAM is not used."));
  }
}

/**
   @brief Function for init update functions from file
   @param none
   @return none
*/
void System_UpdateInit() {
  Update.onProgress([](int progress, size_t total) {
    /* update from file */
    SystemCamera.SetFlashStatus(true);
    uint8_t updateProgress = (progress * 100) / FirmwareUpdate.FirmwareSize;
    SystemLog.AddEvent(LogLevel_Info, "Updating: " + String(FirmwareUpdate.FirmwareSize) + "/" + String(progress) + " -> " + String(updateProgress) + "%");
    FirmwareUpdate.PercentProcess = updateProgress;
    FirmwareUpdate.TransferedBytes = progress;
    delay(10);
    SystemCamera.SetFlashStatus(false);
  });
}

/**
   @brief Main function for system lib
   @param none
   @return none
*/
void System_Main() {
  /* check new FW version */
  if (false == FirmwareUpdate.CheckNewVersionAfterBoot) {
    System_CheckNewVersion();
  }

  /* task for download and flash FW from server */
  System_OtaCloudUpdate();
}

/**
   @brief Function for check FW version on the WEB server

    https://docs.github.com/en/rest/releases/releases?apiVersion=2022-11-28

   @param none
   @return none
*/
void System_CheckNewVersion() {
  if (WL_CONNECTED == WiFi.status()) {
    SystemLog.AddEvent(LogLevel_Info, F("Check new FW version from OTA"));
    FirmwareUpdate.CheckNewVersionAfterBoot = true;
    WiFiClientSecure client;
    client.setCACert(root_CAs_ota);
    //client.setInsecure();
    FirmwareUpdate.CheckNewVersionFwStatus = "N/A";
    FirmwareUpdate.NewVersionFw = "Unknown";
    FirmwareUpdate.OtaUpdateFwUrl = "";
    FirmwareUpdate.OtaUpdateFwAvailable = false;

    /* connect to server and get json */
    if (!client.connect("api.github.com", 443)) {
      FirmwareUpdate.CheckNewVersionFwStatus = "Failed connect to OTA server!";
      SystemLog.AddEvent(LogLevel_Info, FirmwareUpdate.CheckNewVersionFwStatus);

    } else {
      SystemLog.AddEvent(LogLevel_Verbose, F("Connected to server!"));
      client.println("GET https://" + String(OTA_UPDATE_API_SERVER) + String(OTA_UPDATE_API_URL) + " HTTP/1.0");
      client.println("Host: " + String(OTA_UPDATE_API_SERVER));
      client.println("User-Agent: " + String(DEVICE_HOSTNAME));
      client.println("Connection: close");
      client.println();

      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          SystemLog.AddEvent(LogLevel_Verbose, "headers received: " + line);
          break;
        }
      }

      /* wait for data, and read it */
      delay(350);
      String Data = "";
      while (client.connected()) {
        while (client.available()) {
          Data += (char)client.read();
        }
      }

      SystemLog.AddEvent(LogLevel_Verbose, Data);
      client.stop();

      /* json analyzed */
      JsonDocument jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, Data);

      if (error) {
        FirmwareUpdate.CheckNewVersionFwStatus = F("Failed to parse JSON from OTA server!");
        SystemLog.AddEvent(LogLevel_Warning, FirmwareUpdate.CheckNewVersionFwStatus);
        SystemLog.AddEvent(LogLevel_Warning, Data);

      } else {
        const char *firmwareVersion = jsonDoc["tag_name"];
        if (firmwareVersion) {
          FirmwareUpdate.CheckNewVersionFwStatus = F("Download successful");
          FirmwareUpdate.NewVersionFw = firmwareVersion;
          SystemLog.AddEvent(LogLevel_Info, "Available OTA firmware: " + FirmwareUpdate.NewVersionFw);

          /* get assets */
          JsonArray assets = jsonDoc["assets"];
          int assetsCount = assets.size();
          SystemLog.AddEvent(LogLevel_Info, "Assets count: " + String(assetsCount));
          for(int i = 0; i < assetsCount; i++) {
            JsonObject asset = assets[i];
            const char* name = asset["name"];
            SystemLog.AddEvent(LogLevel_Info, "Assets[" + String(i) + "]: " + String(name));

            /* get FW file and URL */
            if (strcmp(name, OTA_UPDATE_FW_FILE) == 0) {
              /* get download URL */
              const char* download_url = asset["browser_download_url"];
              FirmwareUpdate.OtaUpdateFwUrl = download_url;
              SystemLog.AddEvent(LogLevel_Info, "Found FW file: " + String(name) + " URL: " + FirmwareUpdate.OtaUpdateFwUrl);
              FirmwareUpdate.OtaUpdateFwAvailable = true;
            }
          }
        } else {
          FirmwareUpdate.CheckNewVersionFwStatus = F("JSON key 'tag_name' from OTA server not found!");
          SystemLog.AddEvent(LogLevel_Warning, FirmwareUpdate.CheckNewVersionFwStatus);
        }
      }
    }
  }
}

/**
   @brief Function for check start OTA update from WEB server 
   @param none
   @return none
*/
void System_OtaCloudUpdate() {
  if (true == FirmwareUpdate.StartOtaUpdate) {
    FirmwareUpdate.Processing = true;
    FirmwareUpdate.StartOtaUpdate = false;
    FirmwareUpdate.UpdatingStatus = F("Sync NTP time...");
    SystemWifiMngt.SyncNtpTime();
    FirmwareUpdate.UpdatingStatus = F("Start updating");
    System_OtaUpdateStart();
  }
}

/**
   @brief Function for downloading FW and flash new FW to the memmory

    https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPUpdate/src/HTTPUpdate.h

   @param none
   @return bool - true if update is done
*/
bool System_OtaUpdateStart() {
  bool b_ret = false;
  WiFiClientSecure client;

  /* check if new FW version is available */
  if (FirmwareUpdate.OtaUpdateFwAvailable == false) {
    FirmwareUpdate.UpdatingStatus = SYSTEM_MSG_UPDATE_NO_FW;
    SystemLog.AddEvent(LogLevel_Info, FirmwareUpdate.UpdatingStatus);
    FirmwareUpdate.Processing = false;

    return b_ret;
  }
  /* reset wdg */
  esp_task_wdt_reset();

  /* set certificat for secure connection, set timeout and follow redirect */
  client.setCACert(root_CAs_ota);
  // client.setInsecure();
  client.setTimeout(12000 / SECOND_TO_MILISECOND);  /* timeout argument is defined in seconds for setTimeout */
  httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); /* HTTPC_FORCE_FOLLOW_REDIRECTS */

  /* set callback function */
  httpUpdate.onStart(System_OtaUpdateStartCB);
  httpUpdate.onEnd(System_OtaUpdateEndCB);
  httpUpdate.onError(System_OtaUpdateErrorCB);
  httpUpdate.onProgress(System_OtaUpdateProgressCB);

  /* mcu configuration */
  httpUpdate.rebootOnUpdate(false);
  FirmwareUpdate.UpdatingStatus = SYSTEM_MSG_UPDATE_PROCESS;
  httpUpdate.setLedPin(4, HIGH);

  SystemLog.AddEvent(LogLevel_Info, "Start OTA update URL: " + FirmwareUpdate.OtaUpdateFwUrl + ";");

  /* start update */
  t_httpUpdate_return ret = httpUpdate.update(client, FirmwareUpdate.OtaUpdateFwUrl.c_str());

  /* check result */
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      b_ret = true;
      FirmwareUpdate.UpdatingStatus = String(SYSTEM_MSG_UPDATE_FAIL) + "[" + String(httpUpdate.getLastError()) + "]: " + httpUpdate.getLastErrorString();
      break;

    case HTTP_UPDATE_NO_UPDATES:
      FirmwareUpdate.UpdatingStatus = F("No updates");
      break;

    case HTTP_UPDATE_OK:
      FirmwareUpdate.UpdatingStatus = SYSTEM_MSG_UPDATE_DONE;
      break;
  }
  FirmwareUpdate.Processing = false;
  SystemLog.AddEvent(LogLevel_Info, "OTA update DONE. " + FirmwareUpdate.UpdatingStatus);

  return b_ret;
}

/**
   @brief CB function OTA update process 
   @param int - current bytes
   @param int - total bytes
   @return none
*/
void System_OtaUpdateProgressCB(int cur, int total) {
  /* OTA update from server */
  FirmwareUpdate.FirmwareSize = total;
  uint8_t updateProgress = (cur * 100) / FirmwareUpdate.FirmwareSize;
  FirmwareUpdate.PercentProcess = updateProgress;
  FirmwareUpdate.TransferedBytes = cur;
  SystemLog.AddEvent(LogLevel_Info, "Downloaded: " + String(cur) + "/" + String(FirmwareUpdate.FirmwareSize) + " -> " + String(FirmwareUpdate.PercentProcess) + "%");
}

/**
   @brief CB function start OTA update
   @param none
   @return none
*/
void System_OtaUpdateStartCB() {
  SystemLog.AddEvent(LogLevel_Info, F("Start OTA update"));
}

/**
   @brief CB function start OTA update
   @param none
   @return none
*/
void System_OtaUpdateEndCB() {
  SystemLog.AddEvent(LogLevel_Info, F("OTA update done"));
}

/**
   @brief CB function error OTA update
   @param int - error code
   @return none
*/
void System_OtaUpdateErrorCB(int error) {
  SystemLog.AddEvent(LogLevel_Info, "OTA update error: " + String(error));
}

/**
   @brief Function for print reset reason
   @param int - reset reason
   @return String - reset reason
*/
String System_PrintMcuResetReason(int reason) {
  String ret = "";
  switch (reason) {
    case 1: /**<1,  Vbat power on reset*/
      ret = F("POWERON_RESET");
      break;
    case 3: /**<3,  Software reset digital core*/
      ret = F("SW_RESET");
      break;
    case 4: /**<4,  Legacy watch dog reset digital core*/
      ret = F("OWDT_RESET");
      break;
    case 5: /**<5,  Deep Sleep reset digital core*/
      ret = F("DEEPSLEEP_RESET");
      break;
    case 6: /**<6,  Reset by SLC module, reset digital core*/
      ret = F("SDIO_RESET");
      break;
    case 7: /**<7,  Timer Group0 Watch dog reset digital core*/
      ret = F("TG0WDT_SYS_RESET");
      break;
    case 8: /**<8,  Timer Group1 Watch dog reset digital core*/
      ret = F("TG1WDT_SYS_RESET");
      break;
    case 9: /**<9,  RTC Watch dog Reset digital core*/
      ret = F("RTCWDT_SYS_RESET");
      break;
    case 10: /**<10, Instrusion tested to reset CPU*/
      ret = F("INTRUSION_RESET");
      break;
    case 11: /**<11, Time Group reset CPU*/
      ret = F("TGWDT_CPU_RESET");
      break;
    case 12: /**<12, Software reset CPU*/
      ret = F("SW_CPU_RESET");
      break;
    case 13: /**<13, RTC Watch dog Reset CPU*/
      ret = F("RTCWDT_CPU_RESET");
      break;
    case 14: /**<14, for APP CPU, reseted by PRO CPU*/
      ret = F("EXT_CPU_RESET");
      break;
    case 15: /**<15, Reset when the vdd voltage is not stable*/
      ret = F("RTCWDT_BROWN_OUT_RESET");
      break;
    case 16: /**<16, RTC Watch dog reset digital core and rtc module*/
      ret = F("RTCWDT_RTC_RESET");
      break;
    default:
      ret = F("NO_MEAN");
  }

  return ret;
}

/**
   @brief Function for print reset reason
   @param none
   @return String - reset reason
*/
String System_printMcuResetReasonSimple() {
  String ret = "";
  esp_reset_reason_t reason = esp_reset_reason();

  switch (reason) {
    case ESP_RST_UNKNOWN:
      ret = F("Reset reason can not be determined");
      break;
    case ESP_RST_POWERON:
      ret = F("Reset due to power-on event");
      break;
    case ESP_RST_EXT:
      ret = F("Reset by external pin (not applicable for ESP32)");
      break;
    case ESP_RST_SW:
      ret = F("Software reset via esp_restart");
      break;
    case ESP_RST_PANIC:
      ret = F("Software reset due to exception/panic");
      break;
    case ESP_RST_INT_WDT:
      ret = F("Reset (software or hardware) due to interrupt watchdog");
      break;
    case ESP_RST_TASK_WDT:
      ret = F("Reset due to task watchdog");
      break;
    case ESP_RST_WDT:
      ret = F("Reset due to other watchdogs");
      break;
    case ESP_RST_DEEPSLEEP:
      ret = F("Reset after exiting deep sleep mode");
      break;
    case ESP_RST_BROWNOUT:
      ret = F("Brownout reset (software or hardware)");
      break;
    case ESP_RST_SDIO:
      ret = F("Reset over SDIO");
      break;
    default:
      ret = F("N/A");
      break;
  }

  return ret;
}

/**
   @brief Function for WiFi management system task
   @param void *pvParameters
   @return none
*/
void System_TaskWifiManagement(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "Task Wifi Management. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    /* wifi management. Enable/disable AP_STA mode and STA mode*/
    SystemWifiMngt.WifiManagement();

    /* wifi reconnect after signal lost */
    SystemWifiMngt.WiFiReconnect();

    SystemLog.AddEvent(LogLevel_Verbose, "WiFiManagement task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");

    /* reset wdg */
    esp_task_wdt_reset();

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, TASK_WIFI / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Function for main system task
 * 
 * @param void *pvParameters
 * @return none
 */
void System_TaskMain(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "System task. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    /* for ota update */
    esp_task_wdt_reset();
    System_Main();
    SystemLog.AddEvent(LogLevel_Info, "Free RAM: " + String(ESP.getFreeHeap()) + " bytes");
    SystemLog.AddEvent(LogLevel_Info, "Free SPIRAM: " + String(ESP.getFreePsram()) + " bytes");
    SystemLog.AddEvent(LogLevel_Info, "Temperature: " + String(temperatureRead()) + " *C");
    SystemLog.AddEvent(LogLevel_Verbose, "System task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");

    /* reset wdg */
    esp_task_wdt_reset();

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, TASK_SYSTEM / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Function for capture and send photo task
 * 
 * @param void *pvParameters
 * @return none
 */
void System_TaskCaptureAndSendPhoto(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "Task photo processing. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    if (Connect.CheckSendingIntervalExpired()) {
      Connect.SetSendingIntervalCounter(0);
      /* send network information to backend */
      if ((WL_CONNECTED == WiFi.status()) && (false == FirmwareUpdate.Processing)) {
        esp_task_wdt_reset();
        Connect.SendInfoToBackend();
      }

      /* send photo to backend*/
      if ((WL_CONNECTED == WiFi.status()) && (false == FirmwareUpdate.Processing)) {
        esp_task_wdt_reset();
        Connect.TakePictureAndSendToBackend();
      }

    } else {
      /* update counter */
      Connect.IncreaseSendingIntervalCounter();
    }
    
    SystemLog.AddEvent(LogLevel_Verbose, "Photo processing task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");

    /* reset wdg */
    esp_task_wdt_reset();

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, TASK_PHOTO_SEND / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Function for micro SD card check task
 * 
 * @param void *pvParameters
 * @return none
 */
void System_TaskSdCardCheck(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "MicroSdCard check task. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    esp_task_wdt_reset();
    /* check micro SD card */
    if ((true == SystemLog.GetCardDetectAfterBoot()) && (false == SystemLog.GetCardDetectedStatus())) {
      SystemLog.ReinitCard();
      SystemLog.AddEvent(LogLevel_Warning, F("Reinit micro SD card done!"));
    }
    SystemLog.AddEvent(LogLevel_Verbose, "MicroSdCard task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");

    /* reset wdg */
    esp_task_wdt_reset();

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, TASK_SDCARD / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Function for serial configuration task
 * 
 * @param void *pvParameters
 * @return none
 */
void System_TaskSerialCfg(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "SerialCg task. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    esp_task_wdt_reset();
    SystemSerialCfg.ProcessIncommingData();
    SystemLog.AddEvent(LogLevel_Verbose, "SerialCfg task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");

    /* reset wdg */
    esp_task_wdt_reset();

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, TASK_SERIAL_CFG / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Function for stream telemetry task
 * 
 * @param void *pvParameters
 * @return none
 */
void System_TaskStreamTelemetry(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "StreamTelemetry task. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    esp_task_wdt_reset();
    SystemLog.AddEvent(LogLevel_Verbose, "StreamTelemetry task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");
    if (SystemCamera.GetStreamStatus()) {
      char buf[80] = { '\0' };
      sprintf(buf, "Stream, average data in %dsec. FPS: %.1f, Size: %uKB", (TASK_STREAM_TELEMETRY / SECOND_TO_MILISECOND), SystemCamera.StreamGetFrameAverageFps(), SystemCamera.StreamGetFrameAverageSize());
      SystemLog.AddEvent(LogLevel_Info, buf);
      SystemCamera.StreamClearFrameData();
    }

    /* reset wdg */
    esp_task_wdt_reset();

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, TASK_STREAM_TELEMETRY / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Function for system led task
 * 
 * @param void *pvParameters
 * @return none
 */
void System_TaskSysLed(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "SystemLed task. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    system_led.toggle();
    /* reset wdg */
    esp_task_wdt_reset();
    SystemLog.AddEvent(LogLevel_Verbose, "SystemLed task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, system_led.getTimer() / portTICK_PERIOD_MS);
  }
} 

void System_TaskWiFiWatchdog(void *pvParameters) {
  SystemLog.AddEvent(LogLevel_Info, "WiFiWatchdog task. core: " + String(xPortGetCoreID()));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    esp_task_wdt_reset();
    SystemWifiMngt.WiFiWatchdog();
    SystemLog.AddEvent(LogLevel_Verbose, "WiFiWatchdog task. Stack free size: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");

    /* reset wdg */
    esp_task_wdt_reset();

    /* next start task */
    vTaskDelayUntil(&xLastWakeTime, TASK_WIFI_WATCHDOG / portTICK_PERIOD_MS);
  }
}

/* EOF */