/**
   @file cfg.h

   @brief Library for save and load MCU configuration

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _CFG_H_
#define _CFG_H_
#include <WiFi.h>
#include <EEPROM.h>

#include "Arduino.h"
#include <ArduinoUniqueID.h>
#include <base64.h>

#include "mcu_cfg.h"
#include "Camera_cfg.h"
#include "var.h"
#include "log.h"

class Configuration {
public:
  Configuration(Logs*);
  ~Configuration(){};
  void Init();
  bool CheckActifeWifiCfgFlag();
  void CheckResetCfg();

  void SaveRefreshInterval(uint8_t);
  void SaveToken(String);
  void SaveFingerprint(String);
  void SavePhotoQuality(uint8_t);
  void SaveFrameSize(uint8_t);
  void SaveBrightness(int8_t);
  void SaveContrast(int8_t);
  void SaveSaturation(int8_t);
  void SaveHmirror(bool);
  void SaveVflip(bool);
  void SaveLensCorrect(bool);
  void SaveExposureCtrl(bool);
  void SaveAwb(bool);
  void SaveAwbGain(bool);
  void SaveAwbMode(uint8_t);
  void SaveBpc(bool);
  void SaveWpc(bool);
  void SaveRawGama(bool);
  void SaveWifiSsid(String);
  void SaveWifiPassword(String);
  void SaveWifiCfgFlag(uint8_t);
  void SaveEnableServiceAp(bool);
  void SaveBasicAuthUsername(String);
  void SaveBasicAuthPassword(String);
  void SaveBasicAuthFlag(bool);
  void SaveCameraFlashEnable(uint8_t);
  void SaveCameraFlashTime(uint16_t);
  void SaveMdnsRecord(String);
  void SaveAec2(bool);
  void SaveAeLevel(int8_t);
  void SaveAecValue(uint16_t);
  void SaveGainCtrl(bool);
  void SaveAgcGain(uint8_t);
  void SaveLogLevel(LogLevel_enum);
  void SavePrusaConnectHostname(String);

  uint8_t LoadRefreshInterval();
  String LoadToken();
  String LoadFingerprint();
  uint8_t LoadPhotoQuality();
  uint8_t LoadFrameSize();
  int8_t LoadBrightness();
  int8_t LoadContrast();
  int8_t LoadSaturation();
  bool LoadHmirror();
  bool LoadVflip();
  bool LoadLensCorrect();
  bool LoadExposureCtrl();
  bool LoadAwb();
  bool LoadAwbGain();
  uint8_t LoadAwbMode();
  bool LoadBpc();
  bool LoadWpc();
  bool LoadRawGama();
  String LoadWifiSsid();
  String LoadWifiPassowrd();
  bool LoadEnableServiceAp();
  String LoadBasicAuthUsername();
  String LoadBasicAuthPassword();
  bool LoadBasicAuthFlag();
  bool LoadCameraFlashEnable();
  uint16_t LoadCameraFlashTime();
  String LoadMdnsRecord();
  bool LoadAec2();
  int8_t LoadAeLevel();
  uint16_t LoadAecValue();
  bool LoadGainCtrl();
  uint8_t LoadAgcGain();
  LogLevel_enum LoadLogLevel();
  String LoadPrusaConnectHostname();

private:
  Logs *Log;              ///< Pointer to Logs object
  String WiFiMacAddress;  ///< WiFi MAC address

  void ReadCfg();
  void DefaultCfg();
  bool CheckFirstMcuStart();
  void SaveFirstMcuStartFlag(uint8_t);
  void GetFingerprint();

  void SaveUint8(uint16_t, uint8_t);
  void SaveInt8(uint16_t, int8_t);
  void SaveBool(uint16_t, bool);
  void SaveUint16(uint16_t, uint16_t);
  void SaveString(uint16_t, uint16_t, String);
  uint16_t LoadUint16(uint16_t);
  String LoadString(uint16_t, uint16_t, bool);
};

extern Configuration SystemConfig;  ///< Configuration object

#endif

/* EOF */
