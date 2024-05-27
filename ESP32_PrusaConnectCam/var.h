/**
   @file variable.h

   @brief Library with global variables

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "Arduino.h"
#include "mcu_cfg.h"

struct WebBasicAuth_struct {
  bool EnableAuth;                        ///< user definition for enable/disable basic auth
  String UserName;                        ///< login name for basic auth
  String Password;                        ///< password for basic auth
};

struct FirmwareUpdate_struct {
  String UpdatingStatus;                  ///< Updateing status
  bool Processing;                        ///< status abour processing firmware update
  uint8_t PercentProcess;                 ///< processed firmware update
  int TransferedBytes;                    ///< transfered bytes

  int FirmwareSize;                       ///< uploaded firmware size

  bool StartOtaUpdate;                    ///< Start OTA update process
  bool CheckNewVersionAfterBoot;          ///< Check new version OTA update after MCU boot
  String NewVersionFw;                    ///< New FW version
  String CheckNewVersionFwStatus;         ///< connection status from checking new OTA update version
  String OtaUpdateFwUrl;                  ///< URL for OTA update
  bool OtaUpdateFwAvailable;              ///< flag for available new FW version
};

extern struct WebBasicAuth_struct WebBasicAuth;      ///< structure with configuration for basic auth
extern struct FirmwareUpdate_struct FirmwareUpdate;  ///< firmware update status and process

extern TaskHandle_t Task_CapturePhotoAndSend;        ///< task handle for capture photo and send
extern TaskHandle_t Task_WiFiManagement;             ///< task handle for wifi management
extern TaskHandle_t Task_SystemMain;                 ///< task handle for system main
extern TaskHandle_t Task_SdCardCheck;                ///< task handle for sd card check  
extern TaskHandle_t Task_SerialCfg;                  ///< task handle for serial configuration
extern TaskHandle_t Task_StreamTelemetry;            ///< task handle for stream telemetry
extern TaskHandle_t Task_SysLed;                     ///< task handle for system led
extern TaskHandle_t Task_WiFiWatchdog;               ///< task handle for wifi watchdog
//extern TaskHandle_t Task_SdCardFileRemove;           ///< task handle for remove file from sd card  

extern uint8_t StartRemoveSdCard;
extern uint32_t SdCardRemoveTime;

#endif

/* EOF */