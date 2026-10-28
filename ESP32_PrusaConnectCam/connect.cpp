/**
   @file connnect.cpp

   @brief library for communication with prusa connect backend

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "connect.h"

PrusaConnect Connect(&SystemConfig, &SystemLog, &SystemCamera, &SystemWifiMngt);

#if (true == CERT_DIAGNOSTIC)
static bool CertDiagnosticRunning = false;  ///< TEMPORARY: certificate diagnostic is running, data is not sent to backend
#endif

/**
 * @brief Constructor for PrusaConnect class
 *
 * @param Configuration*  - pointer to Configuration class
 * @param Logs*           - pointer to Logs class
 * @param Camera*         - pointer to Camera class
 */
PrusaConnect::PrusaConnect(Configuration *i_conf, Logs *i_log, Camera *i_camera, WiFiMngt *i_wifi) {
  config = i_conf;
  log = i_log;
  camera = i_camera;
  wifi = i_wifi;
  BackendAvailability = WaitForFirstConnection;
  SendDeviceInformationToBackend = true;
  TimelapsLastFileName = "";
  TimelapsFileIndex = 0;
}

/**
 * @brief init library PrusaConnect
 *
 * @param none
 * @return none
 */
void PrusaConnect::Init() {
  log->AddEvent(LogLevel_Info, F("Init PrusaConnect lib"));
  BackendReceivedStatus = F("Wait for first connection");
}

/**
 * @brief Load configuration from EEPROM
 *
 * @param none
 * @return none
 */
void PrusaConnect::LoadCfgFromEeprom() {
  log->AddEvent(LogLevel_Info, F("Load PrusaConnect CFG from EEPROM"));
  Token = config->LoadToken();
  Fingerprint = config->LoadFingerprint();
  RefreshInterval = config->LoadRefreshInterval();
  PrusaConnectHostname = config->LoadPrusaConnectHostname();
  EnableTimelapsPhotoSave = config->LoadTimeLapseFunctionStatus();
}

/**
 * @brief take picture
 *
 * @param none
 * @return none
 */
void PrusaConnect::TakePicture() {
  camera->CapturePhoto();
}

/**
 * @brief Sending data to prusa connect backend
 * 
 * @param i_data  - data to send
 * @param i_content_type - data content type
 * @param i_type - type of data for log message
 * @param i_url_path - url path for backend
 * @param i_fragmentation - flag for enable/disable data fragmentation
 * @return true - if data was sent successfully
 * @return false - if data was not sent successfully
 */
bool PrusaConnect::SendDataToBackend(String *i_data, int i_data_length, String i_content_type, String i_type, String i_url_path, SendDataToBackendType i_data_type) {
#if (true == CERT_DIAGNOSTIC)
  /* TEMPORARY: do not send data during certificate diagnostic */
  if (true == CertDiagnosticRunning) {
    log->AddEvent(LogLevel_Info, "Sending " + i_type + " skipped. Certificate diagnostic is running");
    return false;
  }
#endif

  WiFiClientSecure client;
  BackendReceivedStatus = "";
  bool ret = false;
  log->AddEvent(LogLevel_Info, "Sending " + i_type + " to PrusaConnect, " + String(i_data_length) + " bytes");

  /* check fingerprint and token length */
  if ((Fingerprint.length() > 0) && (Token.length() > 0)) {
    client.setCACert(root_CAs);
    //client.setInsecure();
    client.setTimeout(1000);
    client.setHandshakeTimeout(TLS_HANDSHAKE_TIMEOUT);
    client.setNoDelay(true);

    log->AddEvent(LogLevel_Verbose, F("Connecting to server..."));

    /* connecting to server */
    if (!client.connect(PrusaConnectHostname.c_str(), 443)) {
      char err_buf[200];
      int last_error = client.lastError(err_buf, sizeof(err_buf));
      int error = client.getWriteError();
      if (BackendAvailability != WaitForFirstConnection) {
        BackendAvailability = BackendUnavailable;
      }

      BackendReceivedStatus = "Connetion failed to domain! Error: " + String(last_error) + " - " + String(err_buf) + " : " + String(error);
      log->AddEvent(LogLevel_Info, BackendReceivedStatus + " ,BA:" + CovertBackendAvailabilitStatusToString(BackendAvailability));
      return false;

    } else {
      /* send data to server */
      log->AddEvent(LogLevel_Verbose, F("Connected to server!"));
      client.println("PUT https://" + PrusaConnectHostname + i_url_path + " HTTP/1.1");
      client.println("Host: " + PrusaConnectHostname);
      client.println("User-Agent: ESP32-CAM");
      client.println("Connection: close");

      client.println("Content-Type: " + i_content_type);
      client.println("fingerprint: " + Fingerprint);
      client.println("token: " + Token);
      client.println("Content-Length: " + String(i_data_length));
      client.println();

      esp_task_wdt_reset();
      size_t sendet_data = 0;
      /* sending photo */
      if (SendPhoto == i_data_type) {
        log->AddEvent(LogLevel_Verbose, F("Sendig photo"));

        /* get photo buffer */
        bool SendWithExif = false;
        uint8_t *fbBuf = NULL;
        size_t fbLen = 0;
        if (camera->GetStreamStatus() == true) {
          /* get duplicate photo buffer from camera */
          fbBuf = camera->GetPhotoFbDuplicate()->buf;
          fbLen = camera->GetPhotoFbDuplicate()->len;
        } else {
          /* get original photo buffer from camera */
          fbBuf = camera->GetPhotoFb()->buf;
          fbLen = camera->GetPhotoFb()->len;
        }

        /* sending exif data */
        if ((camera->GetPhotoExifData()->header != NULL) && (camera->GetStreamStatus() == false)) {
          SendWithExif = true;
          sendet_data += client.write(camera->GetPhotoExifData()->header, camera->GetPhotoExifData()->len);
          fbBuf += camera->GetPhotoExifData()->offset;
          fbLen -= camera->GetPhotoExifData()->offset;
        } else {
          SendWithExif = false;
        }

        /* sending photo */
        for (size_t i = 0; i < fbLen; i += PHOTO_FRAGMENT_SIZE) {
          if ((i + PHOTO_FRAGMENT_SIZE) < fbLen) {
            sendet_data += client.write(fbBuf, PHOTO_FRAGMENT_SIZE);
            fbBuf += PHOTO_FRAGMENT_SIZE;

          } else if ((fbLen % PHOTO_FRAGMENT_SIZE) > 0) {
            size_t remainder = fbLen % PHOTO_FRAGMENT_SIZE;
            sendet_data += client.write(fbBuf, remainder);
          }
        }
        client.println("\r\n");

        /* log message */
        if (SendWithExif) {
          SystemLog.AddEvent(LogLevel_Verbose, F("Photo with EXIF data sent"));
        } else {
          SystemLog.AddEvent(LogLevel_Warning, F("Photo without EXIF data sent"));
        }

        /* sending device information */
      } else if (SendInfo == i_data_type) {
        log->AddEvent(LogLevel_Verbose, F("Sending info"));
        sendet_data = client.print(*i_data);
      }

      client.flush();
      log->AddEvent(LogLevel_Info, "Send done: " + String(i_data_length) + "/" + String(sendet_data) + " bytes");

      /* check if all data was sent */
      if (i_data_length != sendet_data) {
        BackendReceivedStatus = F("INCOMPLETE DATA SEND TO SERVER!");
        log->AddEvent(LogLevel_Error, F("ERROR SEND DATA TO SERVER! INCORRECT DATA LENGTH!"));
        client.stop();
        return false;
      }
      //esp_task_wdt_reset();

      /* read response from server */
      String response = "";
      String fullResponse = "";
      delay(10); // wait for response
      log->AddEvent(LogLevel_Verbose, F("Response:"));
      unsigned long ResponseStartTime = millis();
      while (client.connected()) {
        /* timeout for response from server */
        if ((millis() - ResponseStartTime) > HOST_RESPONSE_TIMEOUT) {
          log->AddEvent(LogLevel_Warning, F("Timeout for response from server!"));
          break;
        }

        if (0 == client.available()) {
          /* waiting for data from server */
          delay(1);

        } else {
          response = client.readStringUntil('\n');
          fullResponse += response;
          log->AddEvent(LogLevel_Verbose, response.c_str());

          if (response.startsWith("HTTP/1.1")) {
            int httpCode = response.substring(9, 12).toInt();
            BackendReceivedStatus = i_type;
            BackendReceivedStatus += ": ";
            BackendReceivedStatus += ProcessHttpResponseCode(httpCode);
            if (true == ProcessHttpResponseCodeBool(httpCode)) {
              ret = true;
            }
          }
        }
      }
      log->AddEvent(LogLevel_Verbose, "Full response: " + fullResponse);

      BackendAvailability = BackendAvailable;
      client.stop();
    }
  } else {
    /* err message */
    log->AddEvent(LogLevel_Verbose, F("ERROR SEND DATA TO SERVER! INVALID DATA!"));
    log->AddEvent(LogLevel_Verbose, "Fingerprint: " + Fingerprint);
    log->AddEvent(LogLevel_Verbose, "Token: " + log->MaskSensitiveData(Token));

    if (Fingerprint.length() == 0) {
      BackendReceivedStatus = F("Missing fingerprint");
    } else if (Token.length() == 0) {
      BackendReceivedStatus = F("Missing token");
    }
  }

  log->AddEvent(LogLevel_Info, "Upload done. Response code: " + BackendReceivedStatus + " ,BA:" + CovertBackendAvailabilitStatusToString(BackendAvailability));
  return ret;
}

/**
 * @brief Send photo to prusa connect backend
 *
 * @param none
 * @return none
 */
void PrusaConnect::SendPhotoToBackend() {
  log->AddEvent(LogLevel_Info, F("Start sending photo to prusaconnect"));
  camera->SetPhotoSending(true);
  String Photo = "";
  size_t total_len = 0;

  if ((camera->GetPhotoExifData()->header != NULL) && (camera->GetStreamStatus() == false)) {
    total_len = camera->GetPhotoExifData()->len + camera->GetPhotoFb()->len - camera->GetPhotoExifData()->offset;
  } else if (camera->GetStreamStatus() == false) {
    total_len = camera->GetPhotoFb()->len;
  } else {
    /* check duplicate photo buffer from stream */
    if ((camera->GetPhotoFbDuplicate() == NULL) || (camera->GetPhotoFbDuplicate()->buf == NULL)) {
      log->AddEvent(LogLevel_Error, F("Photo from stream is not available. Stop sending to backend!"));
      camera->SetPhotoSending(false);
      return;
    }
    total_len = camera->GetPhotoFbDuplicate()->len;
  }
  SendDataToBackend(&Photo, total_len, F("image/jpg"), F("Photo"), HOST_URL_CAM_PATH, SendPhoto);
  camera->SetPhotoSending(false);
}

/**
 * @brief seding device info to prusaconnect backend
 * 
 */
void PrusaConnect::SendInfoToBackend() {
  if (false == SendDeviceInformationToBackend) {
    return;

  } else {
    log->AddEvent(LogLevel_Info, F("Start sending device information to prusaconnect"));

    JsonDocument json_data;
    String json_string = "";

    JsonObject config = json_data["config"].to<JsonObject>();
    config["name"] = wifi->GetMdns();
    config["firmware"] = SW_VERSION;
    config["manufacturer"] = F("Prusa");
    config["model"] = BOARD_NAME;

    JsonObject resolution = config["resolution"].to<JsonObject>();
    resolution["width"] = SystemCamera.GetFrameSizeWidth();
    resolution["height"] = SystemCamera.GetFrameSizeHeight();

    JsonObject network_info = config["network_info"].to<JsonObject>();
    network_info["wifi_mac"] = SystemWifiMngt.GetWifiMac();
    network_info["wifi_ipv4"] = SystemWifiMngt.GetStaIp();
    network_info["wifi_ssid"] = SystemWifiMngt.GetStaSsid();

    serializeJson(json_data, json_string);
    log->AddEvent(LogLevel_Info, "Data: " + json_string);
    bool response = SendDataToBackend(&json_string, json_string.length(), F("application/json"), F("Info"), HOST_URL_INFO_PATH, SendInfo);

    if (true == response) {
      SendDeviceInformationToBackend = false;
    }
  }
}

/**
 * @brief Take picture and send to backend
 *
 * @param none
 * @return none
 */
void PrusaConnect::TakePictureAndSendToBackend() {
  camera->CapturePhoto();

  /* check if photo was captured */
  if (camera->GetCameraCaptureSuccess() == true) {

    /* send photo to backend */
    SendPhotoToBackend();

    /* save photo to SD card */
    if (false == camera->GetStreamStatus()) {
      SavePhotoToSdCard();
    }

  } else {
    log->AddEvent(LogLevel_Error, F("Error capturing photo. Stop sending to backend!"));
  }
  
  /* return frame buffer */
  if (camera->GetStreamStatus() == true) {
    /* set stream flag for sending photo to false */
    camera->StreamSetSendingPhoto(false);
  }
}

/**
   @brief Function for processing http response code from prusa backend
   @param int - http response code
   @return none
*/
String PrusaConnect::ProcessHttpResponseCode(int code) {
  String ret = "";
  switch (code) {
    case 200:
      ret = F("200 - OK");
      break;
    case 201:
      ret = F("201 - OK entry created");
      break;
    case 204:
      ret = F("204 - Upload OK");
      break;
    case 304:
      ret = F("304 - Response has not been modified");
      break;
    case 400:
      ret = F("400 - Some data received is not valid");
      break;
    case 401:
      ret = F("401 - Missing security toker or it is not valid");
      break;
    case 403:
      ret = F("403 - Security toke is not valid or is outdated");
      break;
    case 404:
      ret = F("404 - Entity not found or invalid auth token");
      break;
    case 409:
      ret = F("409 - Conflict with the state of target resource (user error)");
      break;
    case 503:
      ret += F("503 - Service is unavailable at this moment. Try again later");
      break;
    default:
      ret = String(code);
      ret += F(" - unknown error code");
      break;
  }

  return ret;
}
/**
 * @brief Translate http response code to boolean
 * 
 * @param code - http response code
 * @return true - if response code is OK
 * @return false - if response code is not OK
 */
bool PrusaConnect::ProcessHttpResponseCodeBool(int code) {
  bool ret = false;
  switch (code) {
    case 200:
      ret = true;
      break;
    case 201:
      ret = true;
      break;
    case 204:
      ret = true;
      break;
    case 304:
      ret = false;
      break;
    case 400:
      ret = false;
      break;
    case 401:
      ret = false;
      break;
    case 403:
      ret = false;
      break;
    case 404:
      ret = false;
      break;
    case 409:
      ret = false;
      break;
    case 503:
      ret = false;
      break;
    default:
      ret = false;
      break;
  }

  return ret;
}

/**
 * @brief Update device information
 *
 * @param none
 * @return none
 */
void PrusaConnect::UpdateDeviceInformation() {
  SendDeviceInformationToBackend = true;
}

/**
 * @brief Set refresh interval
 *
 * @param uint8_t i_data - refresh interval
 * @return none
 */
void PrusaConnect::SetRefreshInterval(uint8_t i_data) {
  RefreshInterval = i_data;
  config->SaveRefreshInterval(RefreshInterval);
}

/**
 * @brief Set token
 *
 * @param String i_data - token
 * @return none
 */
void PrusaConnect::SetToken(String i_data) {
  Token = i_data;
  config->SaveToken(Token);
}

/**
 * @brief Set backend availability status
 *
 * @param BackendAvailabilitStatus - backend status
 * @return none
 */
void PrusaConnect::SetBackendAvailabilitStatus(BackendAvailabilitStatus i_data) {
  BackendAvailability = i_data;
}

/**
 * @brief set prusa connect hostname
 * 
 * @param String i_data - hostname
 */
void PrusaConnect::SetPrusaConnectHostname(String i_data) {
  PrusaConnectHostname = i_data;
  config->SavePrusaConnectHostname(PrusaConnectHostname);
}

/**
 * @brief Set time laps photo save status
 * 
 * @param bool - status
 */
void PrusaConnect::SetTimeLapsPhotoSaveStatus(bool i_data) {
  EnableTimelapsPhotoSave = i_data;
  config->SaveTimeLapseFunctionStatus(EnableTimelapsPhotoSave);
}

/**
   @brief Function for saving photo to SD card
   @param none
   @return none
*/
void PrusaConnect::SavePhotoToSdCard() {
#if (ENABLE_SD_CARD == true)
  /* check if time laps photo save is enabled */
  if (EnableTimelapsPhotoSave == true) {
    log->AddEvent(LogLevel_Info, F("Save TimeLaps photo to SD card"));
    
    /* check if SD card is detected */
    if (log->GetCardDetectedStatus() == false) {
      log->AddEvent(LogLevel_Error, F("SD card not detected!"));
      return;
    }

    /* check if folder for time laps photos exists */
    if (false == log->CheckDir(SD_MMC, TIMELAPS_PHOTO_FOLDER)) {
      log->AddEvent(LogLevel_Info, F("Create folder for TimeLaps photos"));
      log->CreateDir(SD_MMC, TIMELAPS_PHOTO_FOLDER);
    }

    /* one folder per day: FAT has a limited number of entries in one folder (with the long names ~16 thousand photos, ~5 days with 30 s interval) */
    String SystemTime = log->GetSystemTime();
    String DirName = String(TIMELAPS_PHOTO_FOLDER) + "/" + SystemTime.substring(0, 10);
    if (false == log->CheckDir(SD_MMC, DirName)) {
      log->AddEvent(LogLevel_Info, F("Create folder for TimeLaps photos: "), DirName);
      log->CreateDir(SD_MMC, DirName);
    }

    /* create file name. When the file already exists (time before the NTP sync, two photos in one second), the name gets _INDEX.
       The index continues from the last one, the files with the lower index exist */
    String FileNameBase = DirName + "/" + String(TIMELAPS_PHOTO_PREFIX) + "_" + SystemTime;
    String FileName = FileNameBase + TIMELAPS_PHOTO_SUFFIX;
    if (true == log->CheckDir(SD_MMC, FileName)) {
      if (FileNameBase != TimelapsLastFileName) {
        TimelapsLastFileName = FileNameBase;
        TimelapsFileIndex = 0;
      }

      do {
        TimelapsFileIndex++;
        FileName = FileNameBase + "_" + String(TimelapsFileIndex) + TIMELAPS_PHOTO_SUFFIX;
      } while (true == log->CheckDir(SD_MMC, FileName));
    }
    log->AddEvent(LogLevel_Verbose, F("Saving file: "), FileName);

    /* save photo to SD card */
    if (camera->GetPhotoExifData()->header != NULL) {
      if (log->WritePicture(FileName, camera->GetPhotoFb()->buf + camera->GetPhotoExifData()->offset, camera->GetPhotoFb()->len - camera->GetPhotoExifData()->offset, camera->GetPhotoExifData()->header, camera->GetPhotoExifData()->len) == true) {
        log->AddEvent(LogLevel_Info, F("Photo saved to SD card. EXIF"));
      } else {
        log->AddEvent(LogLevel_Error, F("Error saving photo to SD card. EXIF"));
      }

    } else {
      if (log->WritePicture(FileName, camera->GetPhotoFb()->buf, camera->GetPhotoFb()->len) == true) {
        log->AddEvent(LogLevel_Info, F("Photo saved to SD card"));
      } else {
        log->AddEvent(LogLevel_Error, F("Error saving photo to SD card"));
      }
    }
  }
#endif
}

/**
 * @brief Get refresh interval
 *
 * @param none
 * @return uint8_t - refresh interval
 */
uint8_t PrusaConnect::GetRefreshInterval() {
  return RefreshInterval;
}

/**
 * @brief get backend received status
 *
 * @param none
 * @return String - backend received status
 */
String PrusaConnect::GetBackendReceivedStatus() {
  return BackendReceivedStatus;
}

/**
 * @brief get token
 *
 * @param none
 * @return String - token
 */
String PrusaConnect::GetToken() {
  return Token;
}

/**
 * @brief get fingerprint
 *
 * @param none
 * @return String - fingerprint
 */
String PrusaConnect::GetFingerprint() {
  return Fingerprint;
}

/**
 * @brief get prusa connect hostname
 * 
 * @return String - hostanme
 */
String PrusaConnect::GetPrusaConnectHostname() {
  return PrusaConnectHostname;
}

/**
 * @brief Get backend availability status
 *
 * @param none
 * @return BackendAvailabilitStatus - backend status
 */
BackendAvailabilitStatus PrusaConnect::GetBackendAvailabilitStatus() {
  return BackendAvailability;
}

/** 
 * @brief Convert backend availability status to string
 * @param BackendAvailabilitStatus - backend status
 * @return String - backend status as string
*/
String PrusaConnect::CovertBackendAvailabilitStatusToString(BackendAvailabilitStatus i_data) {
  String ret = "";
  switch (i_data) {
    case WaitForFirstConnection:
      ret = F("Wait for first connection");
      break;
    case BackendAvailable:
      ret = F("Backend available");
      break;
    case BackendUnavailable:
      ret = F("Backend unavailable");
      break;
    default:
      ret = F("Unknown");
      break;
  }

  return ret;
}

bool PrusaConnect::GetTimeLapsPhotoSaveStatus() {
  return EnableTimelapsPhotoSave;
}

/**
 * @brief Increase sending interval counter
 *
 * @param none
 * @return none
 */
void PrusaConnect::IncreaseSendingIntervalCounter() {
  SendingIntervalCounter++;
}

/**
 * @brief Set sending interval counter
 *
 * @param uint8_t i_data - counter
 * @return none
 */
void PrusaConnect::SetSendingIntervalCounter(uint8_t i_data) {
  SendingIntervalCounter = i_data;
}

void PrusaConnect::SetSendingIntervalExpired() {
  SendingIntervalCounter = RefreshInterval;
}

/**
 * @brief Get sending interval counter
 * 
 * @return uint8_t - counter
 */
uint8_t PrusaConnect::GetSendingIntervalCounter() {
  return SendingIntervalCounter;
}

/**
 * @brief Check if sending interval is expired. and can I send the data to the backend. [seconds]
 * 
 * @return true 
 * @return false 
 */
bool PrusaConnect::CheckSendingIntervalExpired() {
  bool ret = false;
  if (SendingIntervalCounter >= RefreshInterval) {
    ret = true;
  }

  return ret;
}

#if (true == CERT_DIAGNOSTIC)
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"

/**
 * @brief TEMPORARY: verify callback for the certificate diagnostic. Log verification flags for each certificate in the chain
 *
 * @param void* - label of the trusted CA
 * @param mbedtls_x509_crt* - certificate
 * @param int - depth of the certificate in the chain
 * @param uint32_t* - verification flags
 * @return int - 0
 */
static int Connect_CertDiagnosticVerifyCb(void *i_label, mbedtls_x509_crt *i_crt, int i_depth, uint32_t *i_flags) {
  char name[96] = { '\0' };
  char info[160] = { '\0' };

  mbedtls_x509_dn_gets(name, sizeof(name), &i_crt->subject);
  if (0 != *i_flags) {
    mbedtls_x509_crt_verify_info(info, sizeof(info), "", *i_flags);
  }
  SystemLog.AddEvent(LogLevel_Info, "CertDiag verify [" + String((const char *)i_label) + "] depth " + String(i_depth) + ": " + String(name) + ", flags: 0x" + String(*i_flags, HEX) + " " + String(info));

  return 0;
}

/**
 * @brief TEMPORARY: check signature of the certificate by the public key of the parent certificate
 *
 * @param mbedtls_x509_crt* - certificate
 * @param mbedtls_x509_crt* - parent certificate
 * @param String - label for log message
 * @return none
 */
static void Connect_CertDiagnosticCheckSignature(const mbedtls_x509_crt *i_child, mbedtls_x509_crt *i_parent, String i_label) {
  unsigned char hash[MBEDTLS_MD_MAX_SIZE];
  const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(i_child->MBEDTLS_PRIVATE(sig_md));
  int ret = -1;

  if (md_info != NULL) {
    ret = mbedtls_md(md_info, i_child->tbs.p, i_child->tbs.len, hash);
    if (0 == ret) {
      ret = mbedtls_pk_verify_ext(i_child->MBEDTLS_PRIVATE(sig_pk), i_child->MBEDTLS_PRIVATE(sig_opts), &i_parent->pk, i_child->MBEDTLS_PRIVATE(sig_md),
                                  hash, mbedtls_md_get_size(md_info), i_child->MBEDTLS_PRIVATE(sig).p, i_child->MBEDTLS_PRIVATE(sig).len);
    }
  }

  SystemLog.AddEvent(LogLevel_Info, "CertDiag signature " + i_label + ": ret " + String(ret) + " (-0x" + String((unsigned int)(-ret), HEX) + "), parent key " + String(mbedtls_pk_get_bitlen(&i_parent->pk)) + " bit");
}

/**
 * @brief TEMPORARY: diagnostic of the TLS certificate chain of the Prusa Connect backend.
 *        Log the server certificate chain, the signature check of each certificate and the verification flags.
 *
 * @param none
 * @return none
 */
void PrusaConnect::CertificateDiagnostic() {
  /* stop sending data to backend and wait for the end of the running sending */
  CertDiagnosticRunning = true;
  log->AddEvent(LogLevel_Info, F("CertDiag: waiting 5s"));
  delay(5000);
  esp_task_wdt_reset();

  log->AddEvent(LogLevel_Info, "CertDiag: start. Hostname: " + PrusaConnectHostname + ", Free RAM: " + String(ESP.getFreeHeap()) + " B, Max block: " + String(ESP.getMaxAllocHeap()));
  log->AddEvent(LogLevel_Info, "CertDiag: Free DRAM (8-bit): " + String(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)) + " B, Min: " + String(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)) + ", Max block: " + String(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)));

  /* 1. connection with certificate verification, the same as in the SendDataToBackend */
  {
    WiFiClientSecure client;
    client.setCACert(root_CAs);
    client.setTimeout(1000);
    if (client.connect(PrusaConnectHostname.c_str(), 443)) {
      log->AddEvent(LogLevel_Info, F("CertDiag: connection with root_CAs OK"));
    } else {
      char err_buf[100];
      int err = client.lastError(err_buf, sizeof(err_buf));
      log->AddEvent(LogLevel_Info, "CertDiag: connection with root_CAs failed: " + String(err) + " - " + String(err_buf));
    }
    client.stop();
  }
  esp_task_wdt_reset();

  /* 2. connection without certificate verification. Get the server certificate chain */
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(1000);
  if (!client.connect(PrusaConnectHostname.c_str(), 443)) {
    log->AddEvent(LogLevel_Info, F("CertDiag: insecure connection failed"));
    CertDiagnosticRunning = false;
    return;
  }
  esp_task_wdt_reset();

  log->AddEvent(LogLevel_Info, "CertDiag: TLS connected. Free DRAM (8-bit): " + String(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)) + " B, Max block: " + String(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)) + ", Free PSRAM: " + String(ESP.getFreePsram()) + " B");

  const mbedtls_x509_crt *peer = client.getPeerCertificate();
  if (peer == NULL) {
    log->AddEvent(LogLevel_Info, F("CertDiag: server certificate chain is not available"));
    client.stop();
    CertDiagnosticRunning = false;
    return;
  }

  /* server certificate chain */
  int depth = 0;
  for (const mbedtls_x509_crt *crt = peer; crt != NULL; crt = crt->next) {
    char subject[96] = { '\0' };
    char issuer[96] = { '\0' };
    mbedtls_x509_dn_gets(subject, sizeof(subject), &crt->subject);
    mbedtls_x509_dn_gets(issuer, sizeof(issuer), &crt->issuer);
    log->AddEvent(LogLevel_Info, "CertDiag chain [" + String(depth) + "]: " + String(subject) + " <- " + String(issuer) + ", key " + String(mbedtls_pk_get_bitlen(&crt->pk)) + " bit, DER " + String(crt->raw.len) + " B");
    depth++;
  }

  /* root CA certificates */
  mbedtls_x509_crt *ca = (mbedtls_x509_crt *)calloc(1, sizeof(mbedtls_x509_crt));
  if (ca == NULL) {
    log->AddEvent(LogLevel_Info, F("CertDiag: memory allocation failed"));
    client.stop();
    CertDiagnosticRunning = false;
    return;
  }
  mbedtls_x509_crt_init(ca);
  int ret = mbedtls_x509_crt_parse(ca, (const unsigned char *)root_CAs, strlen(root_CAs) + 1);
  int ca_count = 0;
  for (mbedtls_x509_crt *c = ca; (c != NULL) && (c->version != 0); c = c->next) {
    char subject[96] = { '\0' };
    mbedtls_x509_dn_gets(subject, sizeof(subject), &c->subject);
    log->AddEvent(LogLevel_Info, "CertDiag root CA [" + String(ca_count) + "]: " + String(subject) + ", key " + String(mbedtls_pk_get_bitlen(&c->pk)) + " bit");
    ca_count++;
  }
  log->AddEvent(LogLevel_Info, "CertDiag: root_CAs parse ret: " + String(ret) + ", certificates: " + String(ca_count));

  /* signature of each certificate in the chain by the next certificate in the chain */
  depth = 0;
  for (const mbedtls_x509_crt *crt = peer; (crt != NULL) && (crt->next != NULL); crt = crt->next) {
    Connect_CertDiagnosticCheckSignature(crt, crt->next, "chain[" + String(depth) + "] by chain[" + String(depth + 1) + "]");
    depth++;
  }
  esp_task_wdt_reset();

  /* signature of the certificates in the chain by the root CA certificates with the same name */
  depth = 0;
  for (const mbedtls_x509_crt *crt = peer; crt != NULL; crt = crt->next) {
    int ca_index = 0;
    for (mbedtls_x509_crt *c = ca; (c != NULL) && (c->version != 0); c = c->next) {
      if ((crt->issuer_raw.len == c->subject_raw.len) && (0 == memcmp(crt->issuer_raw.p, c->subject_raw.p, c->subject_raw.len))) {
        Connect_CertDiagnosticCheckSignature(crt, c, "chain[" + String(depth) + "] by root CA [" + String(ca_index) + "]");
      }
      ca_index++;
    }
    depth++;
  }
  esp_task_wdt_reset();

  /* verification of the chain with all root CA certificates */
  uint32_t flags = 0;
  ret = mbedtls_x509_crt_verify((mbedtls_x509_crt *)peer, ca, NULL, PrusaConnectHostname.c_str(), &flags, Connect_CertDiagnosticVerifyCb, (void *)"all");
  log->AddEvent(LogLevel_Info, "CertDiag verify [all]: ret " + String(ret) + ", flags: 0x" + String(flags, HEX));
  esp_task_wdt_reset();

  /* verification of the chain with each root CA certificate separately */
  int ca_index = 0;
  for (mbedtls_x509_crt *c = ca; (c != NULL) && (c->version != 0); c = c->next) {
    mbedtls_x509_crt *single = (mbedtls_x509_crt *)calloc(1, sizeof(mbedtls_x509_crt));
    if (single != NULL) {
      mbedtls_x509_crt_init(single);
      mbedtls_x509_crt_parse_der(single, c->raw.p, c->raw.len);
      String label = "root CA [" + String(ca_index) + "]";
      flags = 0;
      ret = mbedtls_x509_crt_verify((mbedtls_x509_crt *)peer, single, NULL, PrusaConnectHostname.c_str(), &flags, Connect_CertDiagnosticVerifyCb, (void *)label.c_str());
      log->AddEvent(LogLevel_Info, "CertDiag verify [" + label + "]: ret " + String(ret) + ", flags: 0x" + String(flags, HEX));
      mbedtls_x509_crt_free(single);
      free(single);
    }
    ca_index++;
    esp_task_wdt_reset();
  }

  mbedtls_x509_crt_free(ca);
  free(ca);
  client.stop();
  CertDiagnosticRunning = false;
  log->AddEvent(LogLevel_Info, "CertDiag: done. Free RAM: " + String(ESP.getFreeHeap()) + " B, Max block: " + String(ESP.getMaxAllocHeap()));
}
#endif

/* EOF */