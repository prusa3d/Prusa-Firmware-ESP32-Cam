/**
   @file serial_cfg.cpp

   @brief 

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "serial_cfg.h"

SerialCfg SystemSerialCfg(&SystemConfig, &SystemLog, &SystemWifiMngt, &Connect);

/**
   @brief Constructor
   @param Configuration * - pointer to Configuration object
   @param Logs * - pointer to Logs object
   @param WiFiMngt * - pointer to WiFiMngt object
   @param PrusaConnect * - pointer to PrusaConnect object
   @return none
*/
SerialCfg::SerialCfg(Configuration *i_conf, Logs *i_log, WiFiMngt *i_wifi, PrusaConnect *i_connect) {
  config = i_conf;
  log = i_log;
  wifim = i_wifi;
  connect = i_connect;
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
    log->AddEvent(LogLevel_Info, "--> Console set WiFi SSID: " + wifi_ssid);
    wifim->SetStaSsid(wifi_ssid);

  } else if (command.startsWith("setwifipass:") && command.endsWith(";")) {
    /* remove prefix "setwifipass:" and end of command symbol ";" */
    wifi_pass = command.substring(12, command.length() -1);
    log->AddEvent(LogLevel_Info, "--> Console set WiFi password: " + wifi_pass);
    wifim->SetStaPassword(wifi_pass);
  
  } else if (command.startsWith("setauthtoken:") && command.endsWith(";")) {
    /* remove prefix "setauthtoken:" and end of command symbol ";" */
    auth_token = command.substring(13, command.length() -1);
    log->AddEvent(LogLevel_Info, "--> Console set auth TOKEN for backend: " + auth_token);
    connect->SetToken(auth_token);

  } else if (command.startsWith("wificonnect") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, "--> Console connecting to wifi...");
    wifim->ConnectToSta();

  } else if (command.startsWith("getwifimode") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, "--> Console print WiFi mode...");
    Serial.print("wifimode:" + wifim->GetWiFiMode() + ";");

  } else if (command.startsWith("getwifistastatus") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, "--> Console print STA status...");
    Serial.print("wifistastatus:" + wifim->GetStaStatus() + ";");
  
  } else if (command.startsWith("getwifistaip") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, "--> Console print STA IP...");
    Serial.print("wifistaip:" + wifim->GetStaIp() + ";");

  } else if (command.startsWith("getserviceapssid") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Info, "--> Console print service WiFi AP SSID...");
    Serial.print("getserviceapssid:" + wifim->GetServiceApSsid() + ";");

  } else if (command.startsWith("mcureboot") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Warning, "--> Reboot MCU!");
    ESP.restart();

  } else if (command.startsWith("commandslist") && command.endsWith(";")) {
    log->AddEvent(LogLevel_Warning, "--> Available commands");
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
  Serial.println("Available commands: ");
  Serial.println("setwifissid:SSID; - set WiFi SSID");
  Serial.println("setwifipass:PASS; - set WiFi password");
  Serial.println("setauthtoken:TOKEN; - set auth TOKEN for backend");
  Serial.println("wificonnect; - connect to WiFi network");
  Serial.println("getwifimode; - get WiFi mode (AP/STA)");
  Serial.println("getwifistastatus; - get STA status (connected/disconnected)");
  Serial.println("getwifistaip; - get STA IP address");
  Serial.println("getserviceapssid;- get service WiFi AP SSID");
  Serial.println("mcureboot; - reboot MCU");
  Serial.println("commandslist; - print available commands");
}

/* EOF */