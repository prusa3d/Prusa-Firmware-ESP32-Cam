/**
   @file server.cpp

   @brief Library for WEB server

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug

*/

#include "server.h"
#include "Certificate.h"

AsyncWebServer server(WEB_SERVER_PORT);

/**
   @brief Load configuration from EEPROM
   @param none
   @return none
*/
void Server_LoadCfg() {
  WebBasicAuth.EnableAuth = SystemConfig.LoadBasicAuthFlag();
  WebBasicAuth.UserName = SystemConfig.LoadBasicAuthUsername();
  WebBasicAuth.Password = SystemConfig.LoadBasicAuthPassword();
}

/**
   @brief Init WEB server
   @param none
   @return none
*/
void Server_InitWebServer() {
  SystemLog.AddEvent(LogLevel_Info, F("Starting init WEB server"));

  /* route for get last capture photo */
  server.on("/saved-photo.jpg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: get photo"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    if (SystemCamera.GetCameraCaptureSuccess() == false) {
      request->send_P(404, "text/plain", "Photo not found!");
      return;
    }
    SystemCamera.SetPhotoSending(true);

    SystemLog.AddEvent(LogLevel_Verbose, "Photo size: " + String(SystemCamera.GetPhotoFb()->len) + " bytes");
  
    if (SystemCamera.GetPhotoExifData()->header != NULL) {
      /* send photo with exif data */
      SystemLog.AddEvent(LogLevel_Verbose, F("Send photo with EXIF data"));
      size_t total_len = SystemCamera.GetPhotoExifData()->len + SystemCamera.GetPhotoFb()->len - SystemCamera.GetPhotoExifData()->offset;
      auto response = request->beginResponseStream("image/jpg");
      response->addHeader("Content-Length", String(total_len));
      response->write(SystemCamera.GetPhotoExifData()->header, SystemCamera.GetPhotoExifData()->len);
      response->write(&SystemCamera.GetPhotoFb()->buf[SystemCamera.GetPhotoExifData()->offset], SystemCamera.GetPhotoFb()->len - SystemCamera.GetPhotoExifData()->offset);
      request->send(response);

    } else {
      /* send photo without exif data */
      SystemLog.AddEvent(LogLevel_Verbose, F("Send photo without EXIF data"));
      request->send_P(200, "image/jpg", SystemCamera.GetPhotoFb()->buf, SystemCamera.GetPhotoFb()->len);
    }

    SystemCamera.SetPhotoSending(false);
  });

  /* route to jquery */
  server.on("/jquery-3.7.0.min.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get jquery-3.7.0.min.js"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "application/javascript", jquery_3_7_0_js);
  });

  Server_InitWebServer_JsonData();
  Server_InitWebServer_WebPages();
  Server_InitWebServer_Icons();
  Server_InitWebServer_Actions();
  Server_InitWebServer_Sets();
  Server_InitWebServer_Update();
  Server_InitWebServer_Stream();

  /* route for not found page */
  server.onNotFound(Server_handleNotFound);

  /* start WEB server */
  server.begin();
}

/**
   @brief Init WEB server json data
   @param none
   @return none
*/
void Server_InitWebServer_JsonData() {
  /* route for json with cfg parameters */
  server.on("/json_input", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: get json_input"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/plain"), Server_GetJsonData().c_str());
  });

  /* route for json with wifi networks */
  server.on("/json_wifi", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: get json_wifi"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/plain"), SystemWifiMngt.GetAvailableWifiNetworks().c_str());
  });

  /* route for san wi-fi networks */
  server.on("/wifi_scan", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: scan WI-FI networks"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/html"), MSG_SCANNING);
    SystemWifiMngt.ScanWiFiNetwork();
  });

  /* route for firmware update processing status */
  server.on("/UpdateProcessing", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (Server_CheckBasicAuth(request) == false)
      return;

    JsonDocument doc_json;
    doc_json["processed_bytes"] = String(FirmwareUpdate.TransferedBytes);
    doc_json["processed_percent"] = String(FirmwareUpdate.PercentProcess);
    doc_json["updating"] = FirmwareUpdate.Processing;
    doc_json["message"] = FirmwareUpdate.UpdatingStatus;
    String string_json = "";
    serializeJson(doc_json, string_json);

    request->send(200, "application/json", string_json);
  });
}

/**
   @brief Init WEB server web pages
   @param none
   @return none
*/
void Server_InitWebServer_WebPages() {
  /* Route for root / web page */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get index.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", index_html);
  });

  /* Route for styles */
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get styles.css"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "text/css", styles_css);
  });

  /* Route for java scripts */
  server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get scripts.js"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, "application/javascript", scripts_js);
  });


  /* Route for config web page */
  server.on("/page_config.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get page_config.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", page_config_html);
  });

  /* Route for wifi web page */
  server.on("/page_wifi.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get page_wifi.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", page_wifi_html);
  });

  /* Route for auth web page */
  server.on("/page_auth.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get page_auth.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", page_auth_html);
  });

  /* Route for system web page */
  server.on("/page_system.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get page_system.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", page_system_html);
  });

  /* route to license page */
  server.on("/license.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get license.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", license_html);
  });

  /* route to gtac page */
  server.on("/gtac.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get gtac.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", gtac_html);
  });

  /* route to privacy policy page */
  server.on("/privacypolicy.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get privacypolicy.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", privacypolicy_html);
  });

  /* route to cookie page */
  server.on("/cookie.html", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get cookie.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "text/html", cookies_html);
  });

  /* route to logs page */
  server.on("/get_logs", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get get_logs.html"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    if (true == SystemLog.GetCardDetectedStatus()) {
      request->send(SD_MMC, SystemLog.GetFilePath() + SystemLog.GetFileName(), "text/plain");
      //SystemLog.LogOpenFile();
    } else {
      request->send_P(404, "text/plain", "Micro SD card not found with FAT32 partition!");
    }
  });
}

/**
   @brief Init WEB server icons
   @param none
   @return none
*/
void Server_InitWebServer_Icons() {
  /* route to logo */
  server.on("/esp32_cam.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get esp32_cam.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "image/svg+xml", esp32_cam_logo_svg);
  });

  /* route to github icon */
  server.on("/github-icon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get github-icon.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "image/svg+xml", github_icon_svg);
  });

  /* route to light on icon */
  server.on("/light-on-icon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get light-icon.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", light_icon_on_svg);
  });

  /* route to light off icon */
  server.on("/light-off-icon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get light-icon.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    
    Server_handleCacheRequest(request, "image/svg+xml", light_icon_off_svg);
  });

  /* route to refresh icon */
  server.on("/refresh-icon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get refresh-icon.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", refresh_icon_svg);
  });

  /* route to reboot icon */
  server.on("/reboot-icon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get reboot-icon.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", reboot_icon_svg);
  });

  /* route to wifi icon */
  server.on("/wifi-icon-1.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get wifi-icon-1.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", wifi_icon_1_svg);
  });

  /* route to wifi icon */
  server.on("/wifi-icon-2.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get wifi-icon-2.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", wifi_icon_2_svg);
  });

  /* route to wifi icon */
  server.on("/wifi-icon-3.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get wifi-icon-3.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", wifi_icon_3_svg);
  });

  /* route to wifi icon */
  server.on("/wifi-icon-4.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get wifi-icon-4.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", wifi_icon_4_svg);
  });

  /* route to wifi icon */
  server.on("/wifi-icon-0.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get wifi-icon-0.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", wifi_icon_0_svg);
  });

  /* route to eye icon */
  server.on("/eye.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get eye.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", eye_svg);
  });

  /* route to eye-slash icon */
  server.on("/eye-slash.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get eye-slash.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", eye_slash_svg);
  });

  /* route to favicon */
  server.on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: Get favicon.svg"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    Server_handleCacheRequest(request, "image/svg+xml", favicon_svg);
  });
}

/**
   @brief Init WEB server actions
   @param none
   @return none
*/
void Server_InitWebServer_Actions() {
  /*route for capture photo */
  server.on("/action_capture", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /action_capture Take photo"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    SystemCamera.CapturePhoto();
    request->send_P(200, "text/plain", "Take Photo");
  });

  /* route for send photo to prusa backend */
  server.on("/action_send", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /action_send send photo to cloud"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    Connect.SetSendingIntervalExpired();
    request->send_P(200, "text/plain", "Send Photo");
  });

  /* route for change LED status */
  server.on("/action_led", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /action_led Change LED status"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    SystemCamera.SetFlashStatus(!SystemCamera.GetFlashStatus());
    SystemCamera.SetCameraFlashEnable(false);

    request->send_P(200, "text/plain", "Change LED status");
  });

  /* reboot MCU */
  server.on("/action_reboot", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /action_reboo reboot MCU!"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/html"), MSG_REBOOT_MCU);
    delay(100); /* wait for sending data */
    ESP.restart();
  });

    /* route for change LED status */
  server.on("/action_sderase", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /action_sderase remove files from SD card"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    StartRemoveSdCard = 1;

    request->send_P(200, F("text/plain"), "Starting remove files from SD card");
  });
}

/**
   @brief Init WEB server sets
   @param none
   @return none
*/
void Server_InitWebServer_Sets() {
  /* route to set integer value */
  server.on("/set_int", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /set_int"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    bool response = false;
    String response_msg = "";

    /* set refresh interval */
    if (request->hasParam("refresh")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set refresh interval"));
      uint8_t value = request->getParam("refresh")->value().toInt();
      if ((value >= REFRESH_INTERVAL_MIN) && (value <= REFRESH_INTERVAL_MAX)) {
        Connect.SetRefreshInterval(value);
        response_msg = MSG_SAVE_OK;
      } else {
        response_msg = "ERROR! Bad value. Minimum is " + String(REFRESH_INTERVAL_MIN) + ", maximum " + String(REFRESH_INTERVAL_MAX) + " second";
      }
      response = true;
    }

    /* set saturation */
    if (request->hasParam("saturation")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set saturation"));
      SystemCamera.SetSaturation(request->getParam("saturation")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set contrast */
    if (request->hasParam("contrast")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set contrast"));
      SystemCamera.SetContrast(request->getParam("contrast")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set brightness */
    if (request->hasParam("brightness")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set brightness"));
      SystemCamera.SetBrightness(request->getParam("brightness")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set frame size */
    if (request->hasParam("framesize")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set framesize"));
      SystemCamera.SetFrameSize(request->getParam("framesize")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set photo quality */
    if (request->hasParam("photo_quality")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set photo_quality"));
      SystemCamera.SetPhotoQuality(73 - request->getParam("photo_quality")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set flash time */
    if (request->hasParam("flash_time")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set flash_time"));
      SystemCamera.SetCameraFlashTime(request->getParam("flash_time")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set white balancing mode */
    if (request->hasParam("wb_mode")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set wb_mode"));
      SystemCamera.SetAwbMode(request->getParam("wb_mode")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set auto exposition level */
    if (request->hasParam("ae_level")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set ae_level"));
      SystemCamera.SetAeLevel(request->getParam("ae_level")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set auto exposition controll value */
    if (request->hasParam("aec_value")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set aec_value"));
      SystemCamera.SetAecValue(request->getParam("aec_value")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set auto gain correction value */
    if (request->hasParam("agc_gain")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set agc_gain"));
      SystemCamera.SetAgcGain(request->getParam("agc_gain")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set image exif rotation */
    if (request->hasParam("image_rotation")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set image EXIF rotation"));
      SystemCamera.SetCameraImageRotation(request->getParam("image_rotation")->value().toInt());
      response_msg = MSG_SAVE_OK;
      response = true;
    }

    /* set log level /set_int?log_level=2 */
    if (request->hasParam("log_level")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set log_level"));
      LogLevel_enum level = (LogLevel_enum)request->getParam("log_level")->value().toInt();
      if ((level >= LogLevel_Error) && (level <= LogLevel_Verbose)) {
        SystemConfig.SaveLogLevel(level);
        SystemLog.SetLogLevel(level);
        response_msg = MSG_SAVE_OK;
      } else {
        response_msg = MSG_SAVE_NOTOK;
      }

      response = true;
    }

    /*  set network ip method. 0 - DHCP, 1 - Static */
    if (request->hasParam("ipcfg")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set ipcfg"));
      SystemWifiMngt.SetNetIpMethod(request->getParam("ipcfg")->value().toInt());
      response_msg = MSG_SAVE_OK_REBOOT;

      response = true;
    }

    if (true == response) {
      request->send_P(200, F("text/html"), response_msg.c_str());
    }
  });

  /* route to set bool value */
  server.on("/set_bool", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /set_bool"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    bool response = false;

    /* check cfg for hmirror */
    if (request->hasParam("hmirror")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set hmirror"));
      SystemCamera.SetHMirror(Server_TransfeStringToBool(request->getParam("hmirror")->value()));
      response = true;
    }

    /* set vertical flip */
    if (request->hasParam("vflip")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set hmirror"));
      SystemCamera.SetVFlip(Server_TransfeStringToBool(request->getParam("vflip")->value()));
      response = true;
    }

    /* set lens correction */
    if (request->hasParam("lenc")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set lensc"));
      SystemCamera.SetLensC(Server_TransfeStringToBool(request->getParam("lenc")->value()));
      response = true;
    }

    /* set exposure controll */
    if (request->hasParam("exposure_ctrl")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set exposure ctrl"));
      SystemCamera.SetExposureCtrl(Server_TransfeStringToBool(request->getParam("exposure_ctrl")->value()));
      response = true;
    }

    /* set auto white balancing */
    if (request->hasParam("awb")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set awb"));
      SystemCamera.SetAwb(Server_TransfeStringToBool(request->getParam("awb")->value()));
      response = true;
    }

    /* set auto white balancing gain */
    if (request->hasParam("awb_gain")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set awb_gain"));
      SystemCamera.SetAwbGain(Server_TransfeStringToBool(request->getParam("awb_gain")->value()));
      response = true;
    }

    /* set bad pixel correction */
    if (request->hasParam("bpc")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set bpc"));
      SystemCamera.SetBpc(Server_TransfeStringToBool(request->getParam("bpc")->value()));
      response = true;
    }

    /* set white pixel correction */
    if (request->hasParam("wpc")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set wpc"));
      SystemCamera.SetWpc(Server_TransfeStringToBool(request->getParam("wpc")->value()));
      response = true;
    }

    /* set raw gama correction */
    if (request->hasParam("raw_gama")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set raw_gama"));
      SystemCamera.SetRawGama(Server_TransfeStringToBool(request->getParam("raw_gama")->value()));
      response = true;
    }

    /* set automatic exposure correction */
    if (request->hasParam("aec2")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set aec2"));
      SystemCamera.SetAec2(Server_TransfeStringToBool(request->getParam("aec2")->value()));
      response = true;
    }

    /* set gain controll */
    if (request->hasParam("gain_ctrl")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set gain_ctrl"));
      SystemCamera.SetGainCtrl(Server_TransfeStringToBool(request->getParam("gain_ctrl")->value()));
      response = true;
    }

    /* set flash */
    if (request->hasParam("flash")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set flash"));
      SystemCamera.SetCameraFlashEnable(Server_TransfeStringToBool(request->getParam("flash")->value()));
      SystemCamera.SetFlashStatus(false);
      response = true;
    }

    if (request->hasParam("serviceap_enable")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set service AP enable"));
      SystemWifiMngt.SetEnableServiceAp(Server_TransfeStringToBool(request->getParam("serviceap_enable")->value()));
      response = true;
    }

    if (request->hasParam("timelaps_enable")) {
      SystemLog.AddEvent(LogLevel_Verbose, F("Set timelaps enable"));
      bool val = Server_TransfeStringToBool(request->getParam("timelaps_enable")->value());
      if ((true == val ) && (SystemLog.GetCardDetectedStatus() == true)) {
        Connect.SetTimeLapsPhotoSaveStatus(val);
      } else {
        Connect.SetTimeLapsPhotoSaveStatus(false);
      }
	  
      response = true;
    }

    if (true == response) {
      request->send_P(200, F("text/html"), MSG_SAVE_OK);
    }
  });

  /* route for set token for authentification to prusa backend*/
  server.on("/set_token", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /set_token"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/html"), MSG_SAVE_OK);

    if (request->hasParam("token")) {
      Connect.SetToken(request->getParam("token")->value());
    }
  });

  /* route for set prusa connect hostname /set_hostname?hostname=*/
  server.on("/set_hostname", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /set_hostname"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/html"), MSG_SAVE_OK);

    if (request->hasParam("hostname")) {
      Connect.SetPrusaConnectHostname(request->getParam("hostname")->value());
    }
  });

  /* route for set WI-FI credentials */
  server.on("/wifi_cfg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: set WI-FI credentials"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    String TmpPassword = "";
    String TmpSsid = "";
    /* get SSID */
    if (request->hasParam("wifi_ssid")) {
      TmpSsid = request->getParam("wifi_ssid")->value();
    }

    /* get password */
    if (request->hasParam("wifi_pass")) {
      TmpPassword = request->getParam("wifi_pass")->value();
    }

    /* check min and max length WI-FI ssid and password */
    if (((TmpPassword.length() > 0) && (TmpSsid.length() > 0)) && ((TmpPassword.length() < EEPROM_ADDR_WIFI_PASSWORD_LENGTH) && (TmpSsid.length() < EEPROM_ADDR_WIFI_SSID_LENGTH))) {
      /* send OK response */
      request->send_P(200, F("text/html"), MSG_SAVE_OK_WIFI);

      /* save ssid and password */
      SystemWifiMngt.SetStaCredentials(TmpSsid,TmpPassword);
      SystemWifiMngt.WiFiStaConnect();

    } else {
      request->send_P(200, F("text/html"), MSG_SAVE_NOTOK);
    }
  });

    /* route for set WI-FI static IP address */
  server.on("/wifi_net_cfg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: set WI-FI static IP address"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    String tmpIp = "";
    String tmpMask = "";
    String tmpGw = "";
    String tmpDns = "";

    /* get ip */
    if (request->hasParam("ip")) {
      tmpIp = request->getParam("ip")->value();
    }

    /* get mask */
    if (request->hasParam("mask")) {
      tmpMask = request->getParam("mask")->value();
    }

    /* get gw */
    if (request->hasParam("gw")) {
      tmpGw = request->getParam("gw")->value();
    }

    /* get dns */
    if (request->hasParam("dns")) {
      tmpDns = request->getParam("dns")->value();
    }

    /* check min and max length network parameters */
    if (((tmpIp.length() > 0) && (tmpMask.length() > 0) && (tmpGw.length() > 0) &&  (tmpDns.length() > 0)) && 
        ((tmpIp.length() <= IPV4_ADDR_MAX_LENGTH) && (tmpMask.length() <= IPV4_ADDR_MAX_LENGTH) &&
         (tmpGw.length() <= IPV4_ADDR_MAX_LENGTH) && (tmpDns.length() <= IPV4_ADDR_MAX_LENGTH))) {
      
      /* save ssid and password */
      SystemWifiMngt.SetNetworkConfig(tmpIp, tmpMask, tmpGw, tmpDns);

      /* send OK response */
      request->send_P(200, F("text/html"), MSG_SAVE_OK_REBOOT);

    } else {
      request->send_P(200, F("text/html"), MSG_SAVE_NOTOK);
    }
  });


  /* route for set basic auth */
  server.on("/basicauth_cfg", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: set basic auth user name and password"));
    bool ret = false;
    String ret_msg = "";

    if (Server_CheckBasicAuth(request) == false)
      return;

    /* get username */
    if (request->hasParam("auth_username")) {
      WebBasicAuth.UserName = request->getParam("auth_username")->value();

      /* check min and max length  */
      if ((WebBasicAuth.UserName.length() > 0) && (WebBasicAuth.UserName.length() < EEPROM_ADDR_BASIC_AUTH_USERNAME_LENGTH)) {
        SystemConfig.SaveBasicAuthUsername(WebBasicAuth.UserName);
        ret = true;
      } else {
        ret = false;
        ret_msg = "Maximum username length: " + String(EEPROM_ADDR_BASIC_AUTH_USERNAME_LENGTH);
      }
    }

    /* get password */
    if (request->hasParam("auth_password")) {
      WebBasicAuth.Password = request->getParam("auth_password")->value();

      /* check min and max length  */
      if ((WebBasicAuth.Password.length() > 0) && (WebBasicAuth.Password.length() < EEPROM_ADDR_BASIC_AUTH_PASSWORD_LENGTH)) {
        SystemConfig.SaveBasicAuthPassword(WebBasicAuth.Password);
        ret = true;
      } else {
        ret = false;
        ret_msg = "Maximum password length: " + String(EEPROM_ADDR_BASIC_AUTH_PASSWORD_LENGTH);
      }
    }

    /* get enable / disable we bauth */
    if (request->hasParam("basicauth_enable")) {
      WebBasicAuth.EnableAuth = Server_TransfeStringToBool(request->getParam("basicauth_enable")->value());
      SystemConfig.SaveBasicAuthFlag(WebBasicAuth.EnableAuth);
      ret = true;
    }

    /* send OK response */
    if (true == ret) {
      request->send_P(200, F("text/html"), MSG_SAVE_OK);
      
    } else {
      String msg = MSG_SAVE_NOTOK;
      msg += " " + ret_msg;
      request->send_P(200, F("text/html"), msg.c_str());
    }
  });

  /* route for set firmware size */
  server.on("/set_firmware_size", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Info, F("WEB server: /set_firmware_size"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/html"), MSG_SAVE_OK);

    /* check cfg for flash */
    if (request->hasParam("size")) {
      SystemLog.AddEvent(LogLevel_Info, String(request->getParam("size")->value().toInt()));
      FirmwareUpdate.FirmwareSize = request->getParam("size")->value().toInt();
    }
  });

  /* route for set firmware size */
  server.on("/set_mdns", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /set_mdns"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    /* check cfg for mdns */
    if (request->hasParam("mdns")) {
      String tmp = request->getParam("mdns")->value();
      if (tmp.length() < EEPROM_ADDR_MDNS_RECORD_LENGTH) {
        request->send_P(200, F("text/html"), MSG_SAVE_OK_REBOOT);
        SystemWifiMngt.SetMdns(tmp);

      } else {
        String msg = "Error save mDNS. Maximum length: " + String(EEPROM_ADDR_MDNS_RECORD_LENGTH);
        request->send_P(200, F("text/html"), msg.c_str());
      }
    }
  });
}

/**
   @brief Init WEB FW update
   @param none
   @return none
*/
void Server_InitWebServer_Update() {
  System_UpdateInit();

  /* route for firmware update with file */
  server.on(
    "/upload", HTTP_POST, [](AsyncWebServerRequest* request) {
      String string_json = "";
      JsonDocument doc_json;

      String responseMessage = (Update.hasError()) ? SYSTEM_MSG_UPDATE_FAIL : SYSTEM_MSG_UPDATE_DONE;
      doc_json["message"] = responseMessage;
      FirmwareUpdate.UpdatingStatus = responseMessage;

      /* get error string */
      if (Update.hasError()) {
        String errorMessage = Update.errorString();
        doc_json["errorMessage"] = errorMessage;
        FirmwareUpdate.UpdatingStatus += errorMessage;
        FirmwareUpdate.Processing = false;
      }

      /* make and send json msg */
      serializeJson(doc_json, string_json);
      AsyncWebServerResponse* response = request->beginResponse(200, "application/json", string_json);
      response->addHeader("Connection", "close");
      request->send(response);
    },
    [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
      if (!index) {
        FirmwareUpdate.Processing = true;
        SystemLog.AddEvent(LogLevel_Info, F("Start FW update from file: "), filename);
        FirmwareUpdate.UpdatingStatus = String(SYSTEM_MSG_UPDATE_PROCESS);
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
          Update.printError(Serial);
        }
      }

      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }

      if (final) {
        FirmwareUpdate.Processing = false;
        if (Update.end(true)) {
          FirmwareUpdate.UpdatingStatus = String(SYSTEM_MSG_UPDATE_DONE);
          SystemLog.AddEvent(LogLevel_Info, F("Update FW from file done. Reboot MCU"));
        } else {
          Update.printError(Serial);
          SystemLog.AddEvent(LogLevel_Error, String(SYSTEM_MSG_UPDATE_FAIL));
        }
      }
    }
  );

  /* route for start web OTA update from server */
  server.on("/web_ota_update", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Info, F("WEB server: /web_ota_update"));
    if (Server_CheckBasicAuth(request) == false)
      return;
    request->send_P(200, F("text/html"), MSG_UPDATE_START);
    FirmwareUpdate.Processing = true;

    /* check flag */
    if (request->hasParam("update")) {
      bool ret = Server_TransfeStringToBool(request->getParam("update")->value());
      if (ret == true) {
        FirmwareUpdate.StartOtaUpdate = true;
      } else {
        FirmwareUpdate.StartOtaUpdate = false;
      }
    }
  });

  /* get OTA FW version on the server */
  server.on("/check_web_ota_update", HTTP_GET, [](AsyncWebServerRequest* request) {
    SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: /check_web_ota_update"));
    if (Server_CheckBasicAuth(request) == false)
      return;

    System_CheckNewVersion();
    request->send_P(200, F("text/html"), FirmwareUpdate.CheckNewVersionFwStatus.c_str());
  });
}

/**
   @brief Init WEB server stream
   @param none
   @return none
*/
void Server_InitWebServer_Stream() {
  server.on("/stream.mjpg", HTTP_GET, Server_streamJpg);
}

/**
   @brief Pause WEB server
   @param none
   @return none
*/
void Server_pause() {
  server.end();
  SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: pause"));
}

/**
   @brief Resume WEB server
   @param none
   @return none
*/
void Server_resume() {
  server.begin();
  SystemLog.AddEvent(LogLevel_Verbose, F("WEB server: resume"));
}

/**
 * @brief Handle cache request
 * 
 * @param AsyncWebServerRequest* - http request
 * @param const char* - content type
 * @param const char* - data
 */
void Server_handleCacheRequest(AsyncWebServerRequest* request, const char *contentType, const char *data) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, contentType, data);
  response->addHeader("Cache-Control", "public, max-age=" + String(WEB_CACHE_INTERVAL));
  request->send(response);
}

/**
   @brief if the page was not found on ESP, then print which page is not there
   @param AsyncWebServerRequest* - request
   @return none
*/
void Server_handleNotFound(AsyncWebServerRequest* request) {
  String message = F("URL not Found\n\n");

  message += "URI: " + request->url() + "\nMethod: ";
  message += (request->method() == HTTP_GET) ? F("GET") : F("POST");
  message += "\nArguments: " + String(request->args()) + "\n";

  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  request->send(404, F("text/plain"), message);
}

/**
   @brief make json data for WEB page on the ESP32
   @param none
   @return String - json data
*/
String Server_GetJsonData() {
  String uptime = "";
  String string_json = "";
  JsonDocument doc_json;
  Server_GetModuleUptime(uptime);

  doc_json["token"] = Connect.GetToken();
  doc_json["fingerprint"] = Connect.GetFingerprint();
  doc_json["refreshInterval"] = String(Connect.GetRefreshInterval());
  doc_json["photoquality"] = String(73 - SystemCamera.GetPhotoQuality());
  doc_json["framesize"] = String(SystemCamera.GetFrameSize());
  doc_json["brightness"] = String(SystemCamera.GetBrightness());
  doc_json["contrast"] = String(SystemCamera.GetContrast());
  doc_json["saturation"] = String(SystemCamera.GetSaturation());
  doc_json["hmirror"] = Server_TranslateBoolToString(SystemCamera.GetHMirror());
  doc_json["vflip"] = Server_TranslateBoolToString(SystemCamera.GetVFlip());
  doc_json["lensc"] = Server_TranslateBoolToString(SystemCamera.GetLensC());
  doc_json["exposure_ctrl"] = Server_TranslateBoolToString(SystemCamera.GetExposureCtrl());
  doc_json["awb"] = Server_TranslateBoolToString(SystemCamera.GetAwb());
  doc_json["awb_gain"] = Server_TranslateBoolToString(SystemCamera.GetAwbGain());
  doc_json["wb_mode"] = String(SystemCamera.GetAwbMode());
  doc_json["bpc"] = Server_TranslateBoolToString(SystemCamera.GetBpc());
  doc_json["wpc"] = Server_TranslateBoolToString(SystemCamera.GetWpc());
  doc_json["raw_gama"] = Server_TranslateBoolToString(SystemCamera.GetRawGama());
  doc_json["aec2"] = Server_TranslateBoolToString(SystemCamera.GetAec2());
  doc_json["ae_level"] = SystemCamera.GetAeLevel();
  doc_json["aec_value"] = SystemCamera.GetAecValue();
  doc_json["gain_ctrl"] = Server_TranslateBoolToString(SystemCamera.GetGainCtrl());
  doc_json["agc_gain"] = SystemCamera.GetAgcGaint();
  doc_json["led"] = Server_TranslateBoolToString(SystemCamera.GetFlashStatus());
  doc_json["flash"] = Server_TranslateBoolToString(SystemCamera.GetCameraFlashEnable());
  doc_json["flash_time"] = SystemCamera.GetCameraFlashTime();
  doc_json["ssid"] = SystemWifiMngt.GetStaSsid();
  doc_json["bssid"] = SystemWifiMngt.GetStaBssid();
  doc_json["rssi"] = String(WiFi.RSSI());
  doc_json["rssi_percentage"] = String(SystemWifiMngt.Rssi2Percent(WiFi.RSSI()));
  doc_json["tx_power"] = SystemWifiMngt.TranslateTxPower(WiFi.getTxPower());
  doc_json["ip"] = WiFi.localIP().toString();
  doc_json["wifi_mode"] = SystemWifiMngt.GetWiFiMode();
  doc_json["mdns"] = SystemWifiMngt.GetMdns();
  doc_json["service_ap_ssid"] = SystemWifiMngt.GetServiceApSsid();
  doc_json["serviceap"] = Server_TranslateBoolToString(SystemWifiMngt.GetEnableServiceAp());
  doc_json["auth"] = Server_TranslateBoolToString(WebBasicAuth.EnableAuth);
  doc_json["auth_username"] = WebBasicAuth.UserName;
  doc_json["last_upload_status"] = Connect.GetBackendReceivedStatus();
  doc_json["wifi_network_status"] = SystemWifiMngt.GetStaStatus();
  doc_json["log_level"] = String(SystemLog.GetLogLevel());
  doc_json["uptime"] = uptime;
  doc_json["user_name"] = WebBasicAuth.UserName;
  doc_json["hostname"] = Connect.GetPrusaConnectHostname();
  doc_json["ip_cfg"] = SystemWifiMngt.GetNetIpMethod();
  doc_json["net_ip"] = SystemWifiMngt.GetNetStaticIp();
  doc_json["net_mask"] = SystemWifiMngt.GetNetStaticMask();
  doc_json["net_gw"] = SystemWifiMngt.GetNetStaticGateway();
  doc_json["net_dns"] = SystemWifiMngt.GetNetStaticDns();
  doc_json["image_rotation"] = SystemCamera.GetCameraImageRotation();
  doc_json["timelaps"] = Server_TranslateBoolToString(Connect.GetTimeLapsPhotoSaveStatus());
  doc_json["sd_status"] = (SystemLog.GetCardDetectedStatus() == true) ? F("Card detected") : F("No card detected");
  doc_json["sd_total"] = SystemLog.GetCardSizeMB();
  doc_json["sd_free_p"] = SystemLog.GetFreeSpacePercent();
  doc_json["sd_used_p"] = SystemLog.GetUsedSpacePercent();
  doc_json["sw_build"] = SW_BUILD;
  doc_json["sw_ver"] = SW_VERSION;
  doc_json["sw_new_ver"] = FirmwareUpdate.NewVersionFw;

  serializeJson(doc_json, string_json);
  SystemLog.AddEvent(LogLevel_Verbose, string_json);
  return string_json;
}

/**
   @brief If basic auth is enabled, then check if the user is logged in
   @param AsyncWebServerRequest - request
   @return bool - status
*/
bool Server_CheckBasicAuth(AsyncWebServerRequest* request) {
  if ((!request->authenticate(WebBasicAuth.UserName.c_str(), WebBasicAuth.Password.c_str())) && (true == WebBasicAuth.EnableAuth)) {
    SystemLog.AddEvent(LogLevel_Verbose, F("Unauthorized! Sending longin request"));
    request->requestAuthentication();
    return false;
  }

  return true;
}

/**
   @brief Stream JPG image from camera
   @param AsyncWebServerRequest - request
   @return void
*/
void Server_streamJpg(AsyncWebServerRequest *request) {
  AsyncJpegStreamResponse *response = new AsyncJpegStreamResponse(&SystemCamera, &SystemLog);
  if (!response) {
    request->send(501);
    return;
  }
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

/**
   @brief Get module uptime
   @param String - uptime value
   @return void
*/
void Server_GetModuleUptime(String& readableTime) {
  unsigned long currentMillis;
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;

  currentMillis = millis();
  seconds = currentMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  readableTime = String(days) + " days, ";

  if (hours < 10) {
    readableTime += "0";
  }
  readableTime += String(hours) + ":";

  if (minutes < 10) {
    readableTime += "0";
  }
  readableTime += String(minutes) + ":";

  if (seconds < 10) {
    readableTime += "0";
  }
  readableTime += String(seconds);
}

/**
   @brief Convert string bool variable to bool
   @param String - data
   @return bool - status
*/
bool Server_TransfeStringToBool(String data) {
  bool ret = false;
  if (data.indexOf("true") >= 0) {
    ret = true;
  } else if (data.indexOf("false") >= 0) {
    ret = false;
  }

  return ret;
}

String Server_TranslateBoolToString(bool i_data) {
  String ret = "";
  if (true == i_data) {
    ret = "true";
  } else {
    ret = "";
  }

  return ret;
}

/* EOF */
