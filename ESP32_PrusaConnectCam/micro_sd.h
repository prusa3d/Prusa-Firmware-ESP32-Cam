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
#include "module_templates.h"
#include "var.h"

class MicroSd {
private:
  bool CardDetected;              ///< Card detected status
  bool DetectAfterBoot;           ///< Card detect after boot
  uint32_t CardSizeMB;            ///< Card size
  uint32_t CardTotalMB;           ///< Card total size
  uint32_t CardUsedMB;            ///< Card used size
  uint32_t CardFreeMB;            ///< Card free size
  uint8_t FreeSpacePercent;       ///< Free space in percent
  uint8_t UsedSpacePercent;       ///< Used space in percent
  File file;                      ///< File object
  SemaphoreHandle_t sdCardMutex;  ///< Mutex for SD card

public:
  MicroSd();
  ~MicroSd(){};

  void InitSdCard();
  void ReinitCard();
  bool OpenFile(File*, String);
  void CloseFile(File*);
  bool CheckOpenFile(File*);

  void ListDir(fs::FS &, String, uint8_t);
  bool CheckDir(fs::FS &, String);
  bool CreateDir(fs::FS &, String);
  bool RemoveDir(fs::FS &, String);
  void ReadFileConsole(fs::FS &, String);
  bool WriteFile(fs::FS &, String, String);
  bool AppendFile(fs::FS &, String, String);
  bool AppendFile(File*, String*);
  bool RenameFile(fs::FS &, String, String);
  bool DeleteFile(fs::FS &, String);
  uint32_t GetFileSize(fs::FS &, String);
  uint16_t FileCount(fs::FS &, String, String);
  bool RemoveFilesInDir(fs::FS &, String, int );
  int CountFilesInDir(fs::FS &, String );

  bool WritePicture(String, uint8_t *, size_t);
  bool WritePicture(String, uint8_t *, size_t, const uint8_t *, size_t);

  void CheckCardUsedStatus();
  bool isCardCorrupted();

  bool GetCardDetectedStatus();
  bool GetCardDetectAfterBoot();
  uint16_t GetCardSizeMB();
  uint16_t GetCardTotalMB();
  uint16_t GetCardUsedMB();
  uint16_t GetCardFreeMB();
  uint32_t GetFreeSpaceMB();
  uint8_t GetFreeSpacePercent();
  uint8_t GetUsedSpacePercent();
};

#endif

/* EOF */