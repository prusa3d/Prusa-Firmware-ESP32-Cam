/**
   @file server.h

   @brief Library for WEB server

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug

*/

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "micro_sd.h"
#include "log.h"
#include "WebPage.h"
#include "WebPage_Icons.h"
#include "mcu_cfg.h"
#include "var.h"
#include "camera.h"
#include "cfg.h"
#include "jquery.h"
#include "system.h"
#include "connect.h"
#include "wifi_mngt.h"
#include "WebStream.h"

extern AsyncWebServer server;  ///< global variable for web server

void Server_LoadCfg();
void Server_InitWebServer();
void Server_InitWebServer_JsonData();
void Server_InitWebServer_WebPages();
void Server_InitWebServer_Icons();
void Server_InitWebServer_Actions();
void Server_InitWebServer_Sets();
void Server_InitWebServer_Update();
void Server_InitWebServer_Stream();

void Server_pause();
void Server_resume();

void Server_handleCacheRequest(AsyncWebServerRequest*, const char*, const char*);
void Server_handleNotFound(AsyncWebServerRequest *);
String Server_GetJsonData();
bool Server_CheckBasicAuth(AsyncWebServerRequest *);

void Server_streamJpg(AsyncWebServerRequest *);

void Server_GetModuleUptime(String &);
bool Server_TransfeStringToBool(String);

String Server_TranslateBoolToString(bool);

/* EOF */
