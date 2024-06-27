/**
   @file wifi_mngt.h

   @brief Library for configuration module via serial port

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#pragma once

#include <Arduino.h>

#include "mcu_cfg.h"
#include "var.h"
#include "log.h"
#include "wifi_mngt.h"
#include "cfg.h"
#include "connect.h"
#include "camera.h"

class WiFiMngt;
class PrusaConnect;

class SerialCfg {
private:
  Configuration *config;    ///< pointer to Configuration object
  Logs *log;                ///< pointer to Logs object
  WiFiMngt *wifim;          ///< pointer to WiFiMngt object
  PrusaConnect *connect;    ///< pointer to PrusaConnect object
  Camera *cam;              ///< pointer to camera object

  String wifi_ssid;         ///< wifi ssid
  String wifi_pass;         ///< wifi password
  String auth_token;        ///< auth token

public:
  SerialCfg(Configuration*, Logs*, WiFiMngt*, PrusaConnect*, Camera*);
  ~SerialCfg(){};

  void ProcessIncommingData();
  void ParseIncommingData(String);
  void PrintAvailableCommands();
};

extern SerialCfg SystemSerialCfg; ///< SerialCfg object

/* EOF */