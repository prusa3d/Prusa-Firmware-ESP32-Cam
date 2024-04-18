/**
   @file wifi_mngt.h

   @brief 

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _WIFI_MNGT_H_
#define _WIFI_MNGT_H_

#include <WiFi.h>
#include "Arduino.h"
#include <esp_task_wdt.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include "esp32-hal-cpu.h"

#include "mcu_cfg.h"
#include "var.h"
#include "log.h"
#include "system.h"
#include "camera.h"
#include "sys_led.h"

/* https://github.com/espressif/arduino-esp32/blob/51cb927712e512664a0a0f7b1219fdc18e11b857/libraries/WiFi/src/WiFiGeneric.h#L52 */
/* https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/network/esp_wifi.html#_CPPv429wifi_event_sta_disconnected_t */
void WiFiMngt_WiFiEventScanDone(WiFiEvent_t, WiFiEventInfo_t);
void WiFiMngt_WiFiEventStationStart(WiFiEvent_t, WiFiEventInfo_t);
void WiFiMngt_WiFiEventStationStop(WiFiEvent_t, WiFiEventInfo_t);
void WiFiMngt_WiFiEventStationConnected(WiFiEvent_t, WiFiEventInfo_t);
void WiFiMngt_WiFiEventStationDisconnected(WiFiEvent_t, WiFiEventInfo_t);
void WiFiMngt_WiFiEventGotIP(WiFiEvent_t, WiFiEventInfo_t);
void WiFiMngt_WiFiEventLostIP(WiFiEvent_t, WiFiEventInfo_t);
void WiFiMngt_WiFiEventApStart(WiFiEvent_t , WiFiEventInfo_t);
void WiFiMngt_WiFiEventApStop(WiFiEvent_t , WiFiEventInfo_t);
void WiFiMngt_WiFiEventApStaConnected(WiFiEvent_t , WiFiEventInfo_t);
void WiFiMngt_WiFiEventApStaDisconnected(WiFiEvent_t , WiFiEventInfo_t);
void WiFiMngt_WiFiEventApStaIpAssigned(WiFiEvent_t , WiFiEventInfo_t);
void WiFiMngt_WiFiEventApStaProbeReqRecved(WiFiEvent_t , WiFiEventInfo_t);

class WiFiMngt {
private:
  String WifiSsid;                    ///< WI-FI SSID
  String WifiPassword;                ///< WI-FI password
  bool ServiceMode;                   ///< flag for enable service AP mode after MCU start during STA_AP_MODE_TIMEOUT ms
  String WiFiMode;                    ///< Wi-Fi mode (AP, STA, STA_AP)
  String SericeApSsid;                ///< Service AP SSID
  bool FirstConnected;                ///< flag about first connecting to WiFi network status
  bool NtpFirstSync;                  ///< flag about first NTP sync status

  uint8_t WiFiStaNetworkBssid[6];     ///< BSSID of the network

  bool WiFiStaMultipleNetwork;        ///< flag about multiple STA networks
  bool StartStaWdg;                   ///< flag about start STA watchdog

  IPAddress Service_LocalIp;          ///< Service IP when si module in the AP mode
  IPAddress Service_Gateway;          ///< Service gateway when si module in the AP mode
  IPAddress Service_Subnet;           ///< Service mask when si module in the AP mode
  IPAddress Service_Dns;              ///< Service DNS when is module in the AP mode

  unsigned long TaskAp_previousMillis;  ///< previous time for task AP
  unsigned long TaskWdg_previousMillis; ///< previous time for task STA watchdog

  String mDNS_record;                 ///< mDNS record
  String WifiScanJson;                ///< global variable with wifi networks

  Configuration *config;              ///< pointer to configuration class
  Logs *log;                          ///< pointer to log class
  Camera *cam;                        ///< pointer to camera class

public:
  WiFiMngt(Configuration*, Logs*, Camera*);
  ~WiFiMngt() {};

  void LoadCfgFromEeprom();
  void Init();
  void WifiManagement();
  void WiFiReconnect();
  void SetWifiEvents();
  void WiFiStaConnect();
  void SyncNtpTime();

  void ScanWiFiNetwork();
  uint8_t ScanWifiNetwork(String);
  bool CheckAvailableWifiNetwork(String);
  int Rssi2Percent(int);
  String TranslateTxPower(wifi_power_t);
  String TranslateWiFiStatus(wl_status_t);
  String TranslateWiFiEncrypion(wifi_auth_mode_t );
  void CreateApSsid();
  void WiFiWatchdog();

  String GetServiceApSsid();
  String GetStaSsid();
  String GetStaBssid();
  String GetStaPassword();
  String GetStaStatus();
  String GetStaIp();
  String GetAvailableWifiNetworks();
  String GetWiFiMode();
  String GetWifiMac();
  String GetMdns();
  bool GetkActifeWifiCfgFlag();
  bool GetNtpFirstTimeSync();
  bool GetFirstConnection();

  void SetStaCredentials(String, String);
  void SetStaSsid(String);
  void SetStaPassword(String);
  void ConnectToSta();
  void SetMdns(String);
  void SetFirstConnection(bool);
};

extern WiFiMngt SystemWifiMngt;   ///< global variable for wifi management

#endif

/* EOF */