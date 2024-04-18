/**
   @file micro_sd.h

   @brief library for communication with micro-SD card

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug

    WARNING: ALL data pins must be pulled up to 3.3V with an external 10k Ohm resistor! Note to ESP32 pin 2 (D0): Add a 1K Ohm pull-up resistor to 3.3V after flashing
    SD Card | ESP32
    D2       12
    D3       13
    CMD      15
    VSS      GND
    VDD      3.3V
    CLK      14
    VSS      GND
    D0       2  (add 1K pull up after flashing)
    D1       4
*/

#ifndef _MICRO_SD_H_
#define _MICRO_SD_H_

#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"

#include "mcu_cfg.h"
#include "var.h"

class MicroSd {
private:
  bool CardDetected;      ///< Card detected status
  uint16_t CardSize;      ///< Card size
  bool DetectAfterBoot;   ///< Card detect after boot

public:
  MicroSd();
  ~MicroSd(){};

  void InitSdCard();
  void ReinitCard();

  void ListDir(fs::FS &, String, uint8_t);
  bool CreateDir(fs::FS &, String);
  bool RemoveDir(fs::FS &, String);
  void ReadFileConsole(fs::FS &, String);
  bool WriteFile(fs::FS &, String, String);
  bool AppendFile(fs::FS &, String, String);
  bool RenameFile(fs::FS &, String, String);
  bool DeleteFile(fs::FS &, String);
  uint32_t GetFileSize(fs::FS &, String);
  uint16_t FileCount(fs::FS &, String, String);

  bool GetCardDetectedStatus();
  uint16_t GetCardSize();
  bool GetCardDetectAfterBoot();
};

#endif

/* EOF */