/**
   @file connnect.cpp

   @brief library for communication with prusa connect backend

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "connect.h"

PrusaConnect Connect(&SystemConfig, &SystemLog, &SystemCamera);

/**
 * @brief Constructor for PrusaConnect class
 *
 * @param Configuration*  - pointer to Configuration class
 * @param Logs*           - pointer to Logs class
 * @param Camera*         - pointer to Camera class
 */
PrusaConnect::PrusaConnect(Configuration *i_conf, Logs *i_log, Camera *i_camera) {
  config = i_conf;
  log = i_log;
  camera = i_camera;
  BackendAvailability = WaitForFirstConnection;
  SendDeviceInformationToBackend = true;
}

/**
 * @brief init library PrusaConnect
 *
 * @param none
 * @return none
 */
void PrusaConnect::Init() {
  log->AddEvent(LogLevel_Info, F("Init PrusaConnect lib"));
  //camera->CapturePhoto();
  //camera->CaptureReturnFrameBuffer();
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
  WiFiClientSecure client;
  BackendReceivedStatus = "";
  bool ret = false;
  log->AddEvent(LogLevel_Info, "Sending " + i_type + " to PrusaConnect, " + String(i_data_length) + " bytes");

  /* check fingerprint and token length */
  if ((Fingerprint.length() > 0) && (Token.length() > 0)) {
    client.setCACert(root_CAs);
    //client.setInsecure();
    client.setTimeout(1000);
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
        log->AddEvent(LogLevel_Verbose, F("Send data photo"));

        /* sending photo */
        uint8_t *fbBuf = camera->GetPhotoFb()->buf;
        size_t fbLen = camera->GetPhotoFb()->len;
        for (size_t i=0; i < fbLen; i += PHOTO_FRAGMENT_SIZE) {
          if ((i + PHOTO_FRAGMENT_SIZE) < fbLen) {
            sendet_data += client.write(fbBuf, PHOTO_FRAGMENT_SIZE);
            fbBuf += PHOTO_FRAGMENT_SIZE;

          } else if ((fbLen % PHOTO_FRAGMENT_SIZE) > 0) {
            size_t remainder = fbLen % PHOTO_FRAGMENT_SIZE;
            sendet_data += client.write(fbBuf, remainder);
          }
        }

        client.println("\r\n");
        client.flush();
        log->AddEvent(LogLevel_Verbose, String(i_data_length) + "/" + String(sendet_data));

      /* sending device information */
      } else if (SendInfo == i_data_type) {
        log->AddEvent(LogLevel_Verbose, F("Send data info"));
        sendet_data = client.print(*i_data);
      }

      log->AddEvent(LogLevel_Info, "Send done: " + String(sendet_data) + " bytes");
      esp_task_wdt_reset();

      /* read response from server */
      String response = "";
      String fullResponse = "";
      log->AddEvent(LogLevel_Verbose, "Response:");
      while (client.connected()) {
        if (client.available()) {
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
    log->AddEvent(LogLevel_Verbose, "Token: " + Token);

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
  String Photo = "";
  SendDataToBackend(&Photo, camera->GetPhotoFb()->len, "image/jpg", "Photo", HOST_URL_CAM_PATH, SendPhoto);
  SystemLog.AddEvent(LogLevel_Info, "Free RAM: " + String(ESP.getFreeHeap()) + " bytes");
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
    config["name"] = "ESP32-CAM";

    JsonObject resolution = config["resolution"].to<JsonObject>();
    resolution["width"] = SystemCamera.GetFrameSizeWidth();
    resolution["height"] = SystemCamera.GetFrameSizeHeight();

    JsonObject network_info = config["network_info"].to<JsonObject>();
    network_info["wifi_mac"] = SystemWifiMngt.GetWifiMac();
    network_info["wifi_ipv4"] = SystemWifiMngt.GetStaIp();
    network_info["wifi_ssid"] = SystemWifiMngt.GetStaSsid();

    serializeJson(json_data, json_string);
    log->AddEvent(LogLevel_Info, "Data: " + json_string);
    bool response = SendDataToBackend(&json_string, json_string.length(), "application/json", "Info", HOST_URL_INFO_PATH, SendInfo);

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
  SendPhotoToBackend();
  camera->CaptureReturnFrameBuffer();
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

/* EOF */