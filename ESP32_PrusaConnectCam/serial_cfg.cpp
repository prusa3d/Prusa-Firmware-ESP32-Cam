/**
   @file serial_cfg.cpp

   @brief 

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "serial_cfg.h"

SerialCfg SystemSerialCfg(&SystemConfig, &SystemLog, &SystemWifiMngt, &Connect, &SystemCamera);

/**
   @brief Constructor
   @param Configuration * - pointer to Configuration object
   @param Logs * - pointer to Logs object
   @param WiFiMngt * - pointer to WiFiMngt object
   @param PrusaConnect * - pointer to PrusaConnect object
   @return none
*/
SerialCfg::SerialCfg(Configuration *i_conf, Logs *i_log, WiFiMngt *i_wifi, PrusaConnect *i_connect, Camera *i_cam) {
  config = i_conf;
  log = i_log;
  wifim = i_wifi;
  connect = i_connect;
  cam = i_cam;
}

/**
   @brief Initialize serial port
   @param none
   @return none
*/
void SerialCfg::ProcessIncommingData() {

  static String inputBuffer = "";

  while (Serial.available() > 0) {
    char incomingChar = Serial.read();
#if (CONSOLE_VERBOSE_DEBUG == true)
    Serial.print("0x");
    Serial.print(incomingChar, HEX);
    Serial.print(" ");
#endif

    if ((incomingChar == '\n') || (incomingChar == '\r')) {
      if (inputBuffer.length() > 1) {
        ParseIncommingData(inputBuffer);
      }
      inputBuffer = "";
    } else {
      inputBuffer += incomingChar;
    }
  }
}

/**
   @brief Parse incomming data from serial port
   @param String command - incomming command
   @return none
*/
void SerialCfg::ParseIncommingData(String command) {

String lastTwoChars = command.substring(command.length() - 2);

  /* remove end of line symbol */
  while (!command.isEmpty() && ((command.endsWith("\r") || command.endsWith("\n")))) {
    command.remove(command.length() - 1);
  }

  /* check command */
  if (command.startsWith("setwifissid:") && command.endsWith(";")) {
    /* remove prefix "setwifissid:" and end of command symbol ";" */
    wifi_ssid = command.substring(12, command.length() -1);
    log->AddEvent(LogLevel_Info, F("--> Console set WiFi SSID: "), wifi_ssid);
    wifim->SetStaSsid(wifi_ssid);

  } else if (command.startsWith("setwifipass:") && command.endsWith(";")) {
    /* remove prefix "setwifipass:" and end of command symbol ";" */
    wifi_pass = command.substring(12, command.length() -1);
    log->AddEvent(LogLevel_Info, F("--> Console set WiFi password: "), wifi_pass);
    wifim->SetStaPassword(wifi_pass);
  
  } else if (command.startsWith("setauthtoken:") && command.endsWith(";")) {
    /* remove prefix "setauthtoken:" and end of command symbol ";" */
    auth_token = command.substring(13, command.length() -1);
    log->AddEvent(LogLevel_Info, F("--> Console set auth TOKEN for backend: "), auth_token);
    connect->SetToken(auth_token);

  } else if (command.startsWith("wificonnect") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, F("--> Console connecting to wifi..."));
    wifim->ConnectToSta();

  } else if (command.startsWith("getwifimode") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, F("--> Console print WiFi mode..."));
    Serial.print("wifimode:" + wifim->GetWiFiMode() + ";");

  } else if (command.startsWith("getwifistastatus") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, F("--> Console print STA status..."));
    Serial.print("wifistastatus:" + wifim->GetStaStatus() + ";");
  
  } else if (command.startsWith("getwifistaip") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, F("--> Console print STA IP..."));
    Serial.print("wifistaip:" + wifim->GetStaIp() + ";");

  } else if (command.startsWith("getserviceapssid") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, F("--> Console print service WiFi AP SSID..."));
    Serial.print("getserviceapssid:" + wifim->GetServiceApSsid() + ";");
  
  } else if (command.startsWith("otaupdate") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Warning, F("--> Start OTA update process!"));
    FirmwareUpdate.StartOtaUpdate = true;

  } else if (command.startsWith("resolution:") && command.endsWith(";")) {
    uint8_t resolution = command.substring(11, command.length() -1).toInt();
    log->AddEvent(LogLevel_Info, F("--> Console set photo resolution: "), String(resolution));
    if ((resolution >= 0) && (resolution <= 6)) {
      cam->SetFrameSize(resolution);
    } else {
      log->AddEvent(LogLevel_Warning, F("--> Bad resolution size!"));
    }
  } else if (command.startsWith("photoquality:") && command.endsWith(";")) {
    uint8_t quality = command.substring(13, command.length() -1).toInt();
    log->AddEvent(LogLevel_Info, F("--> Console set photo quality: "), String(quality));
    if ((quality >= 10) && (quality <= 63)) {
      cam->SetPhotoQuality(quality);
    } else {
      log->AddEvent(LogLevel_Warning, F("--> Bad photo quality!"));
    }

  } else if (command.startsWith("setflash") && command.endsWith(";")) {
    cam->SetCameraFlashEnable(!cam->GetCameraFlashEnable());
    cam->SetFlashStatus(false);
    log->AddEvent(LogLevel_Warning, "--> Console set FLASH: " + String(cam->GetCameraFlashEnable()));
    
  } else if (command.startsWith("setlight") && command.endsWith(";")) {
    cam->SetCameraFlashEnable(false);
    cam->SetFlashStatus(!cam->GetFlashStatus());
    log->AddEvent(LogLevel_Warning, "--> Console set LIGHT: " + String(cam->GetFlashStatus()));    

  } else if (command.startsWith("mcureboot") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Warning, F("--> Reboot MCU!"));
    ESP.restart();

  } else if (command.startsWith("commandslist") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Warning, F("--> Available commands"));
    PrintAvailableCommands();

  } else {
    log->AddEvent(LogLevel_Warning, "--> Unknown command: " + command + "!");
    PrintAvailableCommands();
    return;
  }
}

/**
   @brief Print available commands
   @param none
   @return none
*/
void SerialCfg::PrintAvailableCommands() {
  Serial.println(F("-----------------------------------"));
  Serial.println(F("Available commands: "));
  Serial.println(F("      Command           -    Description"));
  Serial.println(F("setwifissid:SSID;       - set WiFi SSID"));
  Serial.println(F("setwifipass:PASS;       - set WiFi password"));
  Serial.println(F("setauthtoken:TOKEN;     - set auth TOKEN for backend"));
  Serial.println(F("wificonnect;            - connect to WiFi network"));
  Serial.println(F("getwifimode;            - get WiFi mode (AP/STA)"));
  Serial.println(F("getwifistastatus;       - get STA status (connected/disconnected)"));
  Serial.println(F("getwifistaip;           - get STA IP address"));
  Serial.println(F("getserviceapssid;       - get service WiFi AP SSID"));
  Serial.println(F("otaupdate;              - start OTA update process"));
  Serial.println(F("resolution:INDEX;       - set photo resolution"));
  Serial.println(F(" |   Index - Resolution"));
  Serial.println(F(" |->   0   - 320x240"));
  Serial.println(F(" |->   1   - 352x288"));
  Serial.println(F(" |->   2   - 640x480"));
  Serial.println(F(" |->   3   - 800x600"));
  Serial.println(F(" |->   4   - 1024x768"));
  Serial.println(F(" |->   5   - 1280x1024"));
  Serial.println(F(" |->   6   - 1600x1200"));
  Serial.println(F("photoquality:QUALITY;   - set photo quality. 10-63 lower number means higher quality "));
  Serial.println(F("setflash;               - enable/disable LED flash"));
  Serial.println(F("setlight;               - enable/disable LED light"));
  Serial.println(F("mcureboot;              - reboot MCU"));
  Serial.println(F("commandslist;           - print available commands"));
  Serial.println(F("-----------------------------------"));
}

/* EOF */