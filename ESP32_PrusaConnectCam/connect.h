/**
   @file connnect.h

   @brief library for communication with prusa connect backend

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/
#pragma once

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_task_wdt.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "wifi_mngt.h"
#include "mcu_cfg.h"
#include "var.h"
#include "log.h"
#include "camera.h"
#include "cfg.h"
#include "Certificate.h"
#include "WebServer.h"
#include "connect_types.h"

class WiFiMngt;
class Configuration;
class Camera;

class PrusaConnect {
private:
  uint8_t RefreshInterval;                        ///< interval for sending photo to backend
  String BackendReceivedStatus;                   ///< status of backend response
  BackendAvailabilitStatus BackendAvailability;   ///< status of backend availability
  bool SendDeviceInformationToBackend;            ///< flag for sending device information to backend
  uint8_t SendingIntervalCounter;                 ///< counter for sending interval, represents seconds
  bool EnableTimelapsPhotoSave;                   ///< flag for saving photo to SD card

  String Token;                                   ///< token for backend communication
  String Fingerprint;                             ///< fingerprint for backend communication 
  String PrusaConnectHostname;                    ///< hostname of prusa connect backend          

  Configuration *config;                          ///< pointer to configuration object
  Logs *log;                                      ///< pointer to logs object
  Camera *camera;                                 ///< pointer to camera object
  WiFiMngt *wifi;                                 ///< pointer to wifi object

  bool SendDataToBackend(String *, int, String, String, String, SendDataToBackendType);

public:
  PrusaConnect(Configuration*, Logs*, Camera*, WiFiMngt*);
  ~PrusaConnect(){};

  void Init();
  void LoadCfgFromEeprom();

  void TakePicture();
  void SendPhotoToBackend();
  void SendInfoToBackend();
  void TakePictureAndSendToBackend();
  String ProcessHttpResponseCode(int);
  bool ProcessHttpResponseCodeBool(int);
  void UpdateDeviceInformation();

  void SetRefreshInterval(uint8_t);
  void SetToken(String);
  void SetBackendAvailabilitStatus(BackendAvailabilitStatus);
  void SetPrusaConnectHostname(String);
  void SetTimeLapsPhotoSaveStatus(bool);

  void SavePhotoToSdCard();

  uint8_t GetRefreshInterval();
  String GetBackendReceivedStatus();
  String GetToken();
  String GetFingerprint();
  String GetPrusaConnectHostname();
  BackendAvailabilitStatus GetBackendAvailabilitStatus();
  String CovertBackendAvailabilitStatusToString(BackendAvailabilitStatus);
  bool GetTimeLapsPhotoSaveStatus();

  void IncreaseSendingIntervalCounter();
  void SetSendingIntervalCounter(uint8_t);
  void SetSendingIntervalExpired();
  uint8_t GetSendingIntervalCounter();
  bool CheckSendingIntervalExpired();
};

extern PrusaConnect Connect;  ///< PrusaConnect object

/* EOF */