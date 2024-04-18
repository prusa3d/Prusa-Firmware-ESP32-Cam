/**
   @file connnect.h

   @brief library for communication with prusa connect backend

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _PRUSA_CONNECT_H_
#define _PRUSA_CONNECT_H_

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_task_wdt.h>
#include "Arduino.h"
#include <ArduinoJson.h>

#include "wifi_mngt.h"
#include "mcu_cfg.h"
#include "var.h"
#include "log.h"
#include "camera.h"
#include "cfg.h"
#include "Certificate.h"

/**
 * @brief BackendAvailabilitStatus enum
 * status of backend availability
 */
enum BackendAvailabilitStatus {
  WaitForFirstConnection = 0,     ///< waiting for first connection to backend
  BackendAvailable = 1,           ///< backend is available
  BackendUnavailable = 2,         ///< backend is unavailable
};

class PrusaConnect {
private:
  uint8_t RefreshInterval;                        ///< interval for sending photo to backend
  String BackendReceivedStatus;                   ///< status of backend response
  BackendAvailabilitStatus BackendAvailability;   ///< status of backend availability
  bool SendDeviceInformationToBackend;           ///< flag for sending device information to backend

  String Token;                                   ///< token for backend communication
  String Fingerprint;                             ///< fingerprint for backend communication
  String Photo;                                   ///< photo for sending to backend        
  String PrusaConnectHostname;                    ///< hostname of prusa connect backend          

  Configuration *config;                          ///< pointer to configuration object
  Logs *log;                                      ///< pointer to logs object
  Camera *camera;                                 ///< pointer to camera object

  bool SendDataToBackend(String *, String, String, String, bool);

public:
  PrusaConnect(Configuration*, Logs*, Camera*);
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

  uint8_t GetRefreshInterval();
  String GetBackendReceivedStatus();
  String GetToken();
  String GetFingerprint();
  String GetPrusaConnectHostname();
  BackendAvailabilitStatus GetBackendAvailabilitStatus();
  String CovertBackendAvailabilitStatusToString(BackendAvailabilitStatus);
};

extern PrusaConnect Connect;  ///< PrusaConnect object

#endif