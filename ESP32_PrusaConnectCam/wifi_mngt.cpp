/**
   @file wifi_mngt.cpp

   @brief 

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "wifi_mngt.h"

WiFiMngt SystemWifiMngt(&SystemConfig, &SystemLog, &SystemCamera);

/**
   @brief Constructor for WiFiMngt class
   @param Configuration* - pointer to Configuration class
   @param Logs* - pointer to Logs class
   @param Camera* - pointer to Camera class
   @return none
*/  
WiFiMngt::WiFiMngt(Configuration *i_conf, Logs *i_log, Camera *i_cam) {
  config = i_conf;
  log = i_log;
  cam = i_cam;

  Service_LocalIp.fromString(String(SERVICE_LOCAL_IP));
  Service_Gateway.fromString(String(SERVICE_LOCAL_GATEWAY));
  Service_Subnet.fromString(String(SERVICE_LOCAL_MASK));
  Service_Dns.fromString(String(SERVICE_LOCAL_DNS));

  NtpFirstSync = false;
  StartStaWdg = false;
}

/**
   @brief Loaf cfg from EEPROM
   @param none
   @return none
*/
void WiFiMngt::LoadCfgFromEeprom() {
  WifiSsid = config->LoadWifiSsid();
  WifiPassword = config->LoadWifiPassowrd();
  mDNS_record = config->LoadMdnsRecord();
  EnableServiceAp = config->LoadEnableServiceAp();
}

/**
   @brief function for init wifi module
   @param none
   @return none
*/
void WiFiMngt::Init() {
  /* check WI-FI mode */
  system_led.setTimer(STATUS_LED_WIFI_AP);
  log->AddEvent(LogLevel_Info, "WiFi MAC: " + WiFi.macAddress());

  /* Set Wi-Fi networks */
  SetWifiEvents();
  CreateApSsid();

  if (true == GetEnableServiceAp()) {
    log->AddEvent(LogLevel_Info, F("Service AP mode enabled"));
    WiFi.mode(WIFI_AP_STA);
    ServiceMode = true;
    WiFi.softAPConfig(Service_LocalIp, Service_Gateway, Service_Subnet);
    WiFi.softAP(SericeApSsid.c_str(), SERVICE_WIFI_PASS, SERVICE_WIFI_CHANNEL);
    WiFiMode = "AP + Client";
    log->AddEvent(LogLevel_Info, "Service IP Address: http://" + WiFi.softAPIP().toString());
    
  } else {
    log->AddEvent(LogLevel_Warning, F("Service AP mode disabled!"));
    WiFi.mode(WIFI_STA);
    ServiceMode = false;
    WiFiMode = "Client";
  }

  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.setHostname(DEVICE_HOSTNAME);
  FirstConnected = false;
  //WiFi.setTxPower(WIFI_POWER_18_5dBm);

  if (config->CheckActifeWifiCfgFlag() == true) {
    if (true == CheckAvailableWifiNetwork(WifiSsid)) {
      WiFiStaConnect();
      log->AddEvent(LogLevel_Warning, "Connecting to WiFi: " + WifiSsid);

#if (WIFI_CLIENT_WAIT_CON == true)
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        log->AddEvent(LogLevel_Verbose, ".");
      }
      WifiCfg.FirstConnected = true;

      /* Print ESP32 Local IP Address */
      log->AddEvent(LogLevel_Info, "WiFi network IP Address: http://" + WiFi.localIP().toString());
#endif
    } else {
      log->AddEvent(LogLevel_Warning, "Wifi unavailable. Skip connecting to WiFi: " + WifiSsid);
    }
  } else {
    ScanWiFiNetwork();
  }
  TaskAp_previousMillis = millis();

  /* Init MDNS record */
  log->AddEvent(LogLevel_Info, "Starting mDNS record: http://" + mDNS_record + ".local");
  if (!MDNS.begin(mDNS_record)) {
    log->AddEvent(LogLevel_Error, F("Error starting mDNS"));
  } else {
    log->AddEvent(LogLevel_Info, F("Starting mDNS OK"));
  }
  MDNS.addService("http", "tcp", 80);
}

/**
   @brief function for management wifi network
   @param none
   @return none
*/
void WiFiMngt::WifiManagement() {
  /* check disable service AP */
  unsigned long currentMillis = millis();
  if (currentMillis - TaskAp_previousMillis >= STA_AP_MODE_TIMEOUT) {
    TaskAp_previousMillis = currentMillis;
    if ((true == config->CheckActifeWifiCfgFlag()) && (true == ServiceMode) && (WL_CONNECTED == WiFi.status())
        && (false == FirmwareUpdate.Processing) && (false == cam->GetStreamStatus())) {
      if (WiFi.softAPgetStationNum() == 0) {
        log->AddEvent(LogLevel_Info, F("Disable service AP mode"));
        WiFi.mode(WIFI_STA);
        esp_wifi_set_ps(WIFI_PS_NONE);
        WiFiStaConnect();
        //WiFi.begin(WifiSsid, WifiPassword);
        WiFiMode = "Client";

#if (WIFI_CLIENT_WAIT_CON == true)
        while (WiFi.status() != WL_CONNECTED) {
          delay(1000);
          log->AddEvent(LogLevel_Verbose, ".");
        }
        log->AddEvent(LogLevel_Verbose, "Connected to WiFi");
#endif
        ServiceMode = false;
        WiFi.softAPdisconnect(true);
      } else {
        log->AddEvent(LogLevel_Info, "Client [" + String(WiFi.softAPgetStationNum()) + "] still is connected to Wi-Fi AP!");
      }
    }
  }

  /* check first NTP time sync after boot */
  if ((true == config->CheckActifeWifiCfgFlag()) && (false == NtpFirstSync) && (WL_CONNECTED == WiFi.status())) {
    SyncNtpTime();
  }
}

/**
   @brief CB function for reconnect to wifi network
   @param none
   @return none
*/
void WiFiMngt::WiFiReconnect() {
  if ((WiFi.status() != WL_CONNECTED) && (FirstConnected == true)) {
    log->AddEvent(LogLevel_Warning, F("Reconnecting to WiFi. STA"));
    WiFi.disconnect();
    log->AddEvent(LogLevel_Warning, F("Disconnect from WiFi"));
    WiFi.reconnect();
    log->AddEvent(LogLevel_Warning, F("Reconnecting to WiFi. STA"));
  } else if (WiFi.status() == WL_CONNECTED) {
    char cstr[150];
    sprintf(cstr, "Wifi connected. SSID: %s, BSSID: %s, RSSI: %d dBm, IP: %s, TX power: %s", WiFi.SSID().c_str(), WiFi.BSSIDstr().c_str(), WiFi.RSSI(), WiFi.localIP().toString().c_str(), TranslateTxPower(WiFi.getTxPower()).c_str());  //print 3 digits
    log->AddEvent(LogLevel_Info, "WiFi status: " + String(cstr));
  }

  if (Connect.GetBackendAvailabilitStatus() == BackendUnavailable) {
    log->AddEvent(LogLevel_Warning, F("Reconnecting to WiFi. STA. Problem with connecting to backend!"));
    WiFi.disconnect();
    WiFi.reconnect();
    Connect.SetBackendAvailabilitStatus(WaitForFirstConnection);
  }
}

/**
   @brief Function for sets WiFi CB calls
   @param none
   @return none
*/
void WiFiMngt::SetWifiEvents() {
  WiFi.onEvent(WiFiMngt_WiFiEventScanDone, WiFiEvent_t::ARDUINO_EVENT_WIFI_SCAN_DONE);
  WiFi.onEvent(WiFiMngt_WiFiEventStationStart, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_START);
  WiFi.onEvent(WiFiMngt_WiFiEventStationStop, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_STOP);
  WiFi.onEvent(WiFiMngt_WiFiEventStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiMngt_WiFiEventStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(WiFiMngt_WiFiEventGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiMngt_WiFiEventLostIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_LOST_IP);
  WiFi.onEvent(WiFiMngt_WiFiEventApStart, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_START);
  WiFi.onEvent(WiFiMngt_WiFiEventApStop, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STOP);
  WiFi.onEvent(WiFiMngt_WiFiEventApStaConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(WiFiMngt_WiFiEventApStaDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  WiFi.onEvent(WiFiMngt_WiFiEventApStaIpAssigned, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
  WiFi.onEvent(WiFiMngt_WiFiEventApStaProbeReqRecved, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED);
}

/**
   @brief Function for connect to STA
   @param none
   @return none
   @note https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/wifi.html#sta-connection
*/
void WiFiMngt::WiFiStaConnect() {
  if (config->CheckActifeWifiCfgFlag() == true) {
    system_led.setTimer(STATUS_LED_STA_CONNECTING);
    if (false == WiFiStaMultipleNetwork) {
      WiFi.begin(WifiSsid, WifiPassword);
      log->AddEvent(LogLevel_Info, F("Connecting to STA SSID"));
    } else if (true == WiFiStaMultipleNetwork) {
      WiFi.begin(WifiSsid, WifiPassword, 0, WiFiStaNetworkBssid);
      log->AddEvent(LogLevel_Info, F("Connecting to STA BSSID"));
    }
    WiFi.setAutoReconnect(true);
  }
}

/**
   @brief Function for set time from NTP server
   @param none
   @return none
*/
void WiFiMngt::SyncNtpTime() {
  if (WL_CONNECTED == WiFi.status()) {
    /* configure NTP server and timezone to UTC */
    configTime(NTP_GTM_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);  // UTC
    log->AddEvent(LogLevel_Info, F("Waiting for NTP time sync: "));
    log->SetNtpTimeSynced(false);

    /* wait maximum 10s for time sync */
    unsigned long startMillis = millis();
    struct tm timeinfo;
    while ((millis() - startMillis) < 10000) {
      if (getLocalTime(&timeinfo)) {
        log->SetNtpTimeSynced(true);
        break;
      }
      yield();
      delay(500);
    }

    /* report sync status */
    if (true == log->GetNtpTimeSynced()) {
      log->AddEvent(LogLevel_Info, F("Sync NTP time done. Set UTC timezone"));
      NtpFirstSync = true;
    } else {
      log->AddEvent(LogLevel_Info, F("Sync NTP time fail"));
    }
  }
}

/**
   @brief Function for scan wifi network
   @param none
   @return none
*/
void WiFiMngt::ScanWiFiNetwork() {
  ScanWifiNetwork("");
}

/**
   @brief Function for scan wifi network and check available wifi network
   @param String - searched wifi network
   @return uint8_t - count of found wifi networks with same SSID
*/
uint8_t WiFiMngt::ScanWifiNetwork(String ssid) {
  log->AddEvent(LogLevel_Info, F("Scan WI-FI networks"));
  log->AddEvent(LogLevel_Info, "Check available WI-FI network: " + ssid);
  uint8_t ret = 0;        ///< total wifi network count
  int bestSignal = -100;  ///< wifi network with best signal (when is available multiple networks with same SSID)
  uint8_t bssid[6] = { 0 };

  /* scan WI-FI networks */
  int n = WiFi.scanNetworks();
  log->AddEvent(LogLevel_Verbose, F("Scan done"));
  JsonDocument doc_json;
  JsonArray wifiArray = doc_json.to<JsonArray>();
  WifiScanJson = "";

  /* make json with each found WI-FI networks */
  if (n == 0) {
    log->AddEvent(LogLevel_Info, F("No networks found!"));
  } else {
    log->AddEvent(LogLevel_Info, String(n) + " networks found");
    log->AddEvent(LogLevel_Info, F("Nr | SSID                             | RSSI | CH | BSSID             | Encryption"));

    for (int i = 0; i < n; ++i) {
      /* check available wifi network */
      if (WiFi.SSID(i) == ssid) {
        ret++;

        if (WiFi.RSSI(i) > bestSignal) {
          bestSignal = WiFi.RSSI(i);
          uint8_t *tmp = WiFi.BSSID(i);
          memcpy(bssid, tmp, 6);
        }
      }

      /* make json with available wifi networks */
      JsonObject wifi_arr = wifiArray.add<JsonObject>();
      wifi_arr["ssid"] = WiFi.SSID(i);
      wifi_arr["bssid"] = WiFi.BSSIDstr(i);
      wifi_arr["rssi"] = String(WiFi.RSSI(i));
      wifi_arr["rssi_percentage"] = String(Rssi2Percent(WiFi.RSSI(i)));
      wifi_arr["channel"] = String(WiFi.channel(i));
      wifi_arr["encryption"] = TranslateWiFiEncrypion(WiFi.encryptionType(i));

      // Print SSID and RSSI for each network found
      char formattedString[100] = { '\0' };
      sprintf(formattedString, "%2d | %-32.32s | %4d | %2d | %-17s | %s", i + 1,
              WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i), WiFi.BSSIDstr(i).c_str(), TranslateWiFiEncrypion(WiFi.encryptionType(i)).c_str());
      log->AddEvent(LogLevel_Info, formattedString);
    }
  }
  serializeJson(doc_json, WifiScanJson);

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  log->AddEvent(LogLevel_Verbose, WifiScanJson);

  /* print status */
  if (1 <= ret) {
    log->AddEvent(LogLevel_Info, "SSID: " + ssid + " found, " + String(ret) + "x");
    if (1 < ret) {
      memcpy(WiFiStaNetworkBssid, bssid, 6);
      WiFiStaMultipleNetwork = true;
      char mac[18] = { 0 };
      sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", WiFiStaNetworkBssid[0], WiFiStaNetworkBssid[1], WiFiStaNetworkBssid[2], WiFiStaNetworkBssid[3], WiFiStaNetworkBssid[4], WiFiStaNetworkBssid[5]);
      log->AddEvent(LogLevel_Info, "WiFi roaming found! Connecting to " + String(mac) + " -> " + String(bestSignal) + "dBm, " + String(WiFiStaMultipleNetwork));
    }
  } else {
    log->AddEvent(LogLevel_Info, "SSID: " + ssid + " not found");
  }

  return ret;
}

/**
   @brief Function for check available wifi network
   @param String - ssid
   @return bool - true if wifi network is available
*/
bool WiFiMngt::CheckAvailableWifiNetwork(String i_data) {
  uint8_t tmp = ScanWifiNetwork(i_data);
  bool ret = false;
  if (tmp >= 1) {
    ret = true;
  }

  return ret;
}

/**
   @brief Function for create AP SSID name
   @param none
   @return none
*/
void WiFiMngt::CreateApSsid() {
  String name = SERVICE_WIFI_SSID;
#if (true == SERVICE_WIFI_SSID_UID)
  name += "_";
  /* copy just last 3 numbers from ID */
  for (size_t i = UniqueIDsize - 3; i < UniqueIDsize; i++) {
    name += String(UniqueID[i]);
    if (i < (UniqueIDsize - 1))
      name += ".";
  }
#endif

  SericeApSsid = name;
  log->AddEvent(LogLevel_Info, "Service AP SSID: " + SericeApSsid);
}

/**
 * @brief WiFi watchdog for check connection to STA
 * 
 */
void WiFiMngt::WiFiWatchdog() {

  /* when is enabled wifi configuration, and is not connected to wifi network, and is available at least one wifi network */
  if ((true == config->CheckActifeWifiCfgFlag()) && (WL_CONNECTED != WiFi.status()) && (true == GetFirstConnection())) {
    log->AddEvent(LogLevel_Warning, F("WiFi WDG. STA connection lost."));
    unsigned long currentMillis = millis();
    
    if (false == StartStaWdg) {
      if (ScanWifiNetwork(WifiSsid) >= 1) {
        StartStaWdg = true;
        TaskWdg_previousMillis = currentMillis;
        log->AddEvent(LogLevel_Warning, F("WiFi STA connection lost. Start watchdog timer!"));
      }
    }

    if ((true == StartStaWdg) && (currentMillis - TaskWdg_previousMillis >= WIFI_STA_WDG_TIMEOUT)) {
      log->AddEvent(LogLevel_Warning, F("WiFi STA connection lost. WDG timer expired. Restart MCU!"));
      /* restart MCU, or disconnect and connect to WiFi again ? From my point of view, and testing, restart MCU is better */
      ESP.restart();
    }
  } else if (true == StartStaWdg) {
    log->AddEvent(LogLevel_Info, F("WiFi STA connection OK. Stop watchdog timer!"));
    StartStaWdg = false;
    TaskWdg_previousMillis = millis();
  }
}

/**
   @brief Convert WiFi signal RSSI to percent
   @param int - rssi
   @return int - percent
*/
int WiFiMngt::Rssi2Percent(int rssi) {
  int signalStrength = 0;
  if (0 == rssi) {
    signalStrength = 0;
  } else {
    if (rssi <= -100)
      signalStrength = 0;
    else if (rssi >= -50)
      signalStrength = 100;
    else
      signalStrength = 2 * (rssi + 100);
  }
  return signalStrength;
}

/**
   @brief function for translate from wifi_power_t to string TX power
   @param wifi_power_t - value
   @return String - value
*/
String WiFiMngt::TranslateTxPower(wifi_power_t data) {
  String ret = "";
  switch (data) {
    case WIFI_POWER_MINUS_1dBm:
      ret = F("-1dBm");
      break;
    case WIFI_POWER_2dBm:
      ret = F("2dBm");
      break;
    case WIFI_POWER_5dBm:
      ret = F("5dBm");
      break;
    case WIFI_POWER_7dBm:
      ret = F("7dBm");
      break;
    case WIFI_POWER_8_5dBm:
      ret = F("8.5dBm");
      break;
    case WIFI_POWER_11dBm:
      ret = F("11dBm");
      break;
    case WIFI_POWER_13dBm:
      ret = F("13dBm");
      break;
    case WIFI_POWER_15dBm:
      ret = F("15dBm");
      break;
    case WIFI_POWER_17dBm:
      ret = F("17dBm");
      break;
    case WIFI_POWER_18_5dBm:
      ret = F("18.5dBm");
      break;
    case WIFI_POWER_19dBm:
      ret = F("19dBm");
      break;
    case WIFI_POWER_19_5dBm:
      ret = F("19.5dBm");
      break;
  }

  return ret;
}

/**
   @brief function for translate from wl_status_t to string
   @param wl_status_t - value
   @return String - value
   @note https://github.com/esp8266/Arduino/blob/0c897c37a6eab3eab34147219617945a32a9b155/libraries/ESP8266WiFi/src/include/wl_definitions.h#L50
*/
String WiFiMngt::TranslateWiFiStatus(wl_status_t i_wifi_status) {
  String ret = "";
  switch (i_wifi_status) {
    case WL_IDLE_STATUS:
      ret = F("Idle");
      break;
    case WL_NO_SSID_AVAIL:
      ret = F("No SSID available");
      break;
    case WL_SCAN_COMPLETED:
      ret = F("Scan completed");
      break;
    case WL_CONNECTED:
      ret = F("Connected");
      break;
    case WL_CONNECT_FAILED:
      ret = F("Connect failed");
      break;
    case WL_CONNECTION_LOST:
      ret = F("Connection lost");
      break;
    case WL_DISCONNECTED:
      ret = F("Disconnected");
      break;
    case WL_NO_SHIELD:
      ret = F("No WiFi shield");
      break;
    default:
      ret = F("Unkcnown status");
      ret += String(i_wifi_status);
  }
  return ret;
}

/**
   @brief function for translate from wifi_auth_mode_t to string
   @param wifi_auth_mode_t - value
   @return String - value
*/
String WiFiMngt::TranslateWiFiEncrypion(wifi_auth_mode_t i_data) {
  String ret = "";
  switch (i_data) {
    case WIFI_AUTH_OPEN:
      ret = F("open");
      break;
    case WIFI_AUTH_WEP:
      ret = F("WEP");
      break;
    case WIFI_AUTH_WPA_PSK:
      ret = F("WPA");
      break;
    case WIFI_AUTH_WPA2_PSK:
      ret = F("WPA2");
      break;
    case WIFI_AUTH_WPA_WPA2_PSK:
      ret = F("WPA+WPA2");
      break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      ret = F("WPA2-EAP");
      break;
    case WIFI_AUTH_WPA3_PSK:
      ret = F("WPA3");
      break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
      ret = F("WPA2+WPA3");
      break;
    case WIFI_AUTH_WAPI_PSK:
      ret = F("WAPI");
      break;
    default:
      ret = F("unknown");
  }

  return ret;
}

/**
   @brief function for get service AP SSID name
   @param none
   @return String - value
*/
String WiFiMngt::GetServiceApSsid() {
  return SericeApSsid;
}

/**
   @brief function for get STA SSID name
   @param none
   @return String - value
*/
String WiFiMngt::GetStaSsid() {
  return WifiSsid;
}

/**
   @brief function for get STA BSSID 
   @param none
   @return String - value
*/
String WiFiMngt::GetStaBssid() {
  return WiFi.BSSIDstr();
}

/**
   @brief function for get STA password
   @param none
   @return String - value
*/
String WiFiMngt::GetStaPassword() {
  return WifiPassword;
}

/**
   @brief function for get STA status
   @param none
   @return String - value
*/
String WiFiMngt::GetStaStatus() {
  return TranslateWiFiStatus(WiFi.status());
}

/**
   @brief function for get STA IP
   @param none
   @return String - value
*/
String WiFiMngt::GetStaIp() {
  return WiFi.localIP().toString();
}

/**
   @brief function for get available wifi networks
   @param none
   @return String - value
*/
String WiFiMngt::GetAvailableWifiNetworks() {
  return WifiScanJson;
}

/**
   @brief function for get Wi-Fi mode
   @param none
   @return String - value
*/  
String WiFiMngt::GetWiFiMode() {
  return WiFiMode;
}

/**
 * @brief function for get Wi-Fi MAC address
 * 
 * @return String - value
 */
String WiFiMngt::GetWifiMac() {
  return String(WiFi.macAddress());
}

/**
   @brief function for get mDNS record
   @param none
   @return String - value
*/
String WiFiMngt::GetMdns() {
  return mDNS_record;
}

/**
 * @brief function for get status about enable service AP mode
 * 
 * @return true 
 * @return false 
 */
bool WiFiMngt::GetEnableServiceAp() {
  return EnableServiceAp;
}

/**
   @brief function for get first time NTP sync status
   @param none
   @return bool - value
*/
bool WiFiMngt::GetNtpFirstTimeSync() {
  return NtpFirstSync;
}

/**
 * @brief get first connection status
 * 
 * @return bool - status 
 */
bool WiFiMngt::GetFirstConnection() {
  return FirstConnected;
}

/**
   @brief function for set STA credentials
   @param String - ssid
   @param String - password
   @return none
*/
void WiFiMngt::SetStaCredentials(String i_ssid, String i_pass) {
  WifiSsid = i_ssid;
  config->SaveWifiSsid(WifiSsid);

  WifiPassword = i_pass;
  config->SaveWifiPassword(WifiPassword);

  config->SaveWifiCfgFlag(CFG_WIFI_SETTINGS_SAVED);
}

/**
   @brief function for set STA SSID
   @param String - STA SSID name
   @return none
*/
void WiFiMngt::SetStaSsid(String i_ssid) {
  WifiSsid = i_ssid;
  config->SaveWifiSsid(WifiSsid);
}

/**
   @brief function for set STA password
   @param String - STA password
   @return none
*/
void WiFiMngt::SetStaPassword(String i_pass) {
  WifiPassword = i_pass;
  config->SaveWifiPassword(WifiPassword);
}

/**
 * @brief Set enable service AP mode after MCU boot
 * 
 * @param i_data 
 */
void WiFiMngt::SetEnableServiceAp(bool i_data) {
  EnableServiceAp = i_data;
  config->SaveEnableServiceAp(EnableServiceAp);
}

/**
   @brief function for connect to STA
   @param none
   @return none
*/
void WiFiMngt::ConnectToSta() {
  config->SaveWifiCfgFlag(CFG_WIFI_SETTINGS_SAVED);
}

/**
   @brief function for get active wifi configuration flag
   @param none
   @return bool - value
*/
bool WiFiMngt::GetkActifeWifiCfgFlag() {
  return config->CheckActifeWifiCfgFlag();
}

/**
   @brief function for set mDNS record
   @param String - mDNS record
   @return none
*/
void WiFiMngt::SetMdns(String i_data) {
  mDNS_record = i_data;
  config->SaveMdnsRecord(mDNS_record);
}

/**
   @brief function for set first connection flag
   @param bool - data
   @return none
*/
void WiFiMngt::SetFirstConnection(bool i_data) {
  FirstConnected = i_data;
}

/* ----------------------- Static function ----------------------- */

/**
   @brief CB function event about scan wifi networks done
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventScanDone(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi networks scan done"));
}

/**
   @brief CB function for event start station
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventStationStart(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi STA start"));
}

/**
   @brief CB function for event stop station
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventStationStop(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi STA stop"));
}

/**
   @brief CB function for event got information about check station connected
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi connected to STA"));
}

/**
   @brief CB function for got IP after connecting to wifi network
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  system_led.setTimer(STATUS_LED_STA_CONNECTED);
  SystemLog.AddEvent(LogLevel_Info, "WiFi Got IP address: " + WiFi.localIP().toString());
  SystemLog.AddEvent(LogLevel_Info, "WiFi Got mask: " + WiFi.subnetMask().toString());
  SystemLog.AddEvent(LogLevel_Info, "WiFi Got gateway: " + WiFi.gatewayIP().toString());
  SystemLog.AddEvent(LogLevel_Info, "WiFi Got DNS 1: " + WiFi.dnsIP(0).toString());
  SystemLog.AddEvent(LogLevel_Info, "WiFi Got DNS 2: " + WiFi.dnsIP(1).toString());
  SystemWifiMngt.SetFirstConnection(true);

  /* update device information */
  Connect.UpdateDeviceInformation();

  /* check first NTP time sync after boot */
  if (false == SystemWifiMngt.GetNtpFirstTimeSync()) {
    SystemWifiMngt.SyncNtpTime();
  }
}

/**
   @brief CB function for lost IP after connecting to wifi network
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventLostIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi lost IP address"));
}

/**
   @brief CB function for got information about disconnecting from wifi network

   REASON_UNSPECIFIED              = 1,
   REASON_AUTH_EXPIRE              = 2,
   REASON_AUTH_LEAVE               = 3,
   REASON_ASSOC_EXPIRE             = 4,
   REASON_ASSOC_TOOMANY            = 5,
   REASON_NOT_AUTHED               = 6,
   REASON_NOT_ASSOCED              = 7,
   REASON_ASSOC_LEAVE              = 8,
   REASON_ASSOC_NOT_AUTHED         = 9,
   REASON_DISASSOC_PWRCAP_BAD      = 10,
   REASON_DISASSOC_SUPCHAN_BAD     = 11,
   REASON_IE_INVALID               = 13,
   REASON_MIC_FAILURE              = 14,
   REASON_4WAY_HANDSHAKE_TIMEOUT   = 15,
   REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,
   REASON_IE_IN_4WAY_DIFFERS       = 17, 
   REASON_GROUP_CIPHER_INVALID     = 18,
   REASON_PAIRWISE_CIPHER_INVALID  = 19,
   REASON_AKMP_INVALID             = 20,
   REASON_UNSUPP_RSN_IE_VERSION    = 21,
   REASON_INVALID_RSN_IE_CAP       = 22,
   REASON_802_1X_AUTH_FAILED       = 23,
   REASON_CIPHER_SUITE_REJECTED    = 24,
   REASON_BEACON_TIMEOUT           = 200,
   REASON_NO_AP_FOUND              = 201,
   REASON_AUTH_FAIL                = 202,
   REASON_ASSOC_FAIL               = 203,
   REASON_HANDSHAKE_TIMEOUT        = 204,

   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
   @note https://github.com/espressif/arduino-esp32/blob/04963009eedfbc1e0ea2e1378ae69e7cebda6fd6/tools/sdk/include/esp32/esp_event_legacy.h
*/
void WiFiMngt_WiFiEventStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Warning, String("WiFi disconnected from access point. Reason: ") + String(info.wifi_sta_disconnected.reason));
  system_led.setTimer(STATUS_LED_ERROR);
}

/**
   @brief CB function for start AP
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventApStart(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi AP start"));
}

/**
   @brief CB function for stop AP
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventApStop(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi AP stop"));
}

/**
   @brief CB function for AP STA connected
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventApStaConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t mac[6] = { 0 };
  char msg[100] = { '\0' };
  memcpy(mac, info.wifi_ap_staconnected.mac, 6);
  sprintf(msg, "WiFi AP STA. station connected to AP, MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  SystemLog.AddEvent(LogLevel_Info, msg);
}

/**
   @brief CB function for AP STA disconnected
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventApStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t mac[6] = { 0 };
  char msg[100] = { '\0' };
  memcpy(mac, info.wifi_ap_stadisconnected.mac, 6);
  // String(info.wifi_ap_stadisconnected.reason)
  sprintf(msg, "WiFi AP STA. station dicconnected from AP, MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  SystemLog.AddEvent(LogLevel_Info, msg);
}

/**
   @brief CB function for AP STA IP assigned
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventApStaIpAssigned(WiFiEvent_t event, WiFiEventInfo_t info) {
  IPAddress stationIP = info.wifi_ap_staipassigned.ip.addr;
  SystemLog.AddEvent(LogLevel_Info, "WiFi AP STA. IP assigned to connected station. IP: " + stationIP.toString());
}

/**
   @brief CB function for AP STA probe request received
   @param WiFiEvent_t
   @param WiFiEventInfo_t
   @return none
*/
void WiFiMngt_WiFiEventApStaProbeReqRecved(WiFiEvent_t event, WiFiEventInfo_t info) {
  SystemLog.AddEvent(LogLevel_Info, F("WiFi AP STA receive probe request packet in soft-AP interface"));
}

/* EOF */