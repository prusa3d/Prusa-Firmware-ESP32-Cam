/**
   @file server.h

   @brief Library for WEB server

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug

*/

#ifndef _SERVER_H_
#define _SERVER_H_

#include <AsyncEventSource.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebSrv.h>
#include <esp_task_wdt.h>
#include <StringArray.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <Update.h>
#include <ArduinoJson.h>

#include "WebPage.h"
#include "WebPage_Icons.h"
#include "mcu_cfg.h"
#include "var.h"
#include "camera.h"
#include "cfg.h"
#include "jquery.h"
#include "system.h"
#include "log.h"
#include "connect.h"
#include "wifi_mngt.h"
#include "stream.h"

#include "exif.h"

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

#endif

/* EOF */
