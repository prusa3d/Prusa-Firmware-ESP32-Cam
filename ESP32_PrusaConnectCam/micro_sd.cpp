/**
   @file micro_sd.cpp

   @brief library for communication with micro-SD card

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "micro_sd.h"

/**
   @brief Constructor
   @param none
   @return none
*/
MicroSd::MicroSd() {
  CardDetected = false;
  CardSize = 0;
  DetectAfterBoot = false;
}

/**
   @brief Reinit micro SD card
   @param none
   @return none
*/
void MicroSd::ReinitCard() {
  Serial.println(F("Reinit micro SD card!"));
  Serial.println(F("Deinit micro SD card"));
  SD_MMC.end();
  delay(50);
  Serial.println(F("Init micro SD card"));
  InitSdCard();
}

/**
   @brief Init SD card. And check, if is SD card inserted
   @param none
   @return none
*/
void MicroSd::InitSdCard() {
  /* Start INIT Micro SD card */
  Serial.println(F("Start init micro-SD Card"));

  /* set SD card to 1-line/1-bit mode. GPIO 4 is used for LED and for microSD card. But communication is slower. */
  /* https://github.com/espressif/arduino-esp32/blob/master/libraries/SD_MMC/src/SD_MMC.h */
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println(F("SD Card Mount Failed"));
    CardDetected = false;
    CardSize = 0;
    //DetectAfterBoot = false;
    return;
  }

  /* check microSD card and card type */
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println(F("No SD_MMC card attached"));
    CardDetected = false;
    CardSize = 0;
    //DetectAfterBoot = false;
    return;
  }

  /* print card type */
  Serial.print(F("Found card. Card Type: "));
  if (cardType == CARD_MMC) {
    Serial.print(F("MMC"));
  } else if (cardType == CARD_SD) {
    Serial.print(F("SDSC"));
  } else if (cardType == CARD_SDHC) {
    Serial.print(F("SDHC"));
  } else {
    Serial.print(F("UNKNOWN"));
  }

  /* calculation card size */
  CardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf(", Card Size: %d MB\n", CardSize);
  CardDetected = true;
  DetectAfterBoot = true;
}

/**
   @brief List directory on the micro SD card
   @param fs::FS - card
   @param String - Directory name
   @param uint8_t - levels
   @return none
*/
void MicroSd::ListDir(fs::FS &fs, String DirName, uint8_t levels) {
  if (true == CardDetected) {
    Serial.printf("Listing directory: %s\n", DirName.c_str());

    File root = fs.open(DirName.c_str());
    if (!root) {
      Serial.println(F("Failed to open directory"));
      return;
    }
    if (!root.isDirectory()) {
      Serial.println(F("Not a directory"));
      return;
    }

    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        Serial.print(F("  DIR : "));
        Serial.println(file.name());
        if (levels) {
          ListDir(fs, file.path(), levels - 1);
        }
      } else {
        Serial.print(F("  FILE: "));
        Serial.print(file.name());
        Serial.print(F("  SIZE: "));
        Serial.println(file.size());
      }
      file = root.openNextFile();
    }
  }
}

/**
   @brief List directory on the micro SD card
   @param fs::FS - card
   @param String - dir name
   @return bool - status
*/
bool MicroSd::CreateDir(fs::FS &fs, String path) {
  bool status = false;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Creating Dir: %s... ", path.c_str());
#endif

    if (fs.mkdir(path.c_str())) {
      status = true;
    }

#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.println((status == true) ? "Created" : "Failed");
#endif
  }
  return status;
}

/**
   @brief remove directory on the micro SD card
   @param fs::FS - card
   @param String - dir name
   @return bool - status
*/
bool MicroSd::RemoveDir(fs::FS &fs, String path) {
  bool status = false;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Removing Dir: %s... ", path.c_str());
#endif

    if (fs.rmdir(path.c_str())) {
      status = true;
    }

#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.println((status == true) ? "Removed" : "Failed");
#endif
  }
  return status;
}

/**
   @brief Read file and print data to console
   @param fs::FS - card
   @param String - file name
   @return none
*/
void MicroSd::ReadFileConsole(fs::FS &fs, String path) {
  if (true == CardDetected) {
    Serial.printf("Reading file: %s\n", path.c_str());

    File file = fs.open(path.c_str());
    if (!file) {
      Serial.println(F("Failed to open file for reading"));
      return;
    }

    Serial.print(F("Read from file: "));
    while (file.available()) {
      Serial.write(file.read());
    }
  }
}

/**
   @brief Write message to file
   @param fs::FS - card
   @param String - file name
   @param String - message
   @return bool - status
*/
bool MicroSd::WriteFile(fs::FS &fs, String path, String message) {
  bool status = false;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Writing file: %s... ", path.c_str());
#endif

    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file) {
#if (true == CONSOLE_VERBOSE_DEBUG)
      Serial.printf("Failed to open file for writing");
#endif
    } else {
      if (file.print(message.c_str())) {
        status = true;
      }

#if (true == CONSOLE_VERBOSE_DEBUG)
      Serial.println((status == true) ? "File written" : "Write Failed");
#endif
    }
  }
  return status;
}

/**
   @brief Added text to end of file
   @param fs::FS - card
   @param String - file name
   @param String - message
   @return bool - status
*/
bool MicroSd::AppendFile(fs::FS &fs, String path, String message) {
  bool status = false;

  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Appending to file: %s... ", path.c_str());
#endif

    File file = fs.open(path.c_str(), FILE_APPEND);
    if (!file) {
#if (true == CONSOLE_VERBOSE_DEBUG)
      Serial.println("Failed to open file for appending");
#endif
      CardDetected = false;
    } else {
      if (file.print(message.c_str())) {
        status = true;
      } 

#if (true == CONSOLE_VERBOSE_DEBUG)
      Serial.println((status == true) ? "Message appended" : "Append Failed");
#endif
    }
  }
  return status;
}

/**
   @brief Rename file on the SD card
   @param fs::FS - card
   @param String - origin file name
   @param String - new file name
   @return bool - status
*/
bool MicroSd::RenameFile(fs::FS &fs, String path1, String path2) {
  bool status = false;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Renaming file %s to %s... ", path1.c_str(), path2.c_str());
#endif
    if (fs.rename(path1.c_str(), path2.c_str())) {
      status = true;
    }

#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.println((status == true) ? "File renamed" : "Rename Failed");
#endif
  }

  return status;
}

/**
   @brief Delete file on the SD card
   @param fs::FS - card
   @param String - file name
   @return bool - status
*/
bool MicroSd::DeleteFile(fs::FS &fs, String path) {
  bool status = false;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Deleting file: %s... ", path.c_str());
#endif
    if (fs.remove(path.c_str())) {
      status = true;
    }

#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.println((status == true) ? "File deleted" : "Delete Failed");
#endif
  }

  return status;
}

/**
   @brief Get file size in the kb
   @param fs::FS - card
   @param String - file name
   @return uint32_t - size
*/
uint32_t MicroSd::GetFileSize(fs::FS &fs, String path) {
  uint32_t ret = 0;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Getting file size: %s... ", path.c_str());
#endif
    File file = fs.open(path.c_str(), FILE_APPEND);
    if (!file) {
#if (true == CONSOLE_VERBOSE_DEBUG)
      Serial.println("Failed to open file for appending");
#endif
      return 0;
    }

    ret = file.size() / 1024; /* convert from bytes to kb */
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf(" File size: %d\n ", ret);
#endif
  }

  return ret; /* kb*/
}

/**
   @brief Check file count with partial match
   @param fs::FS - card
   @param String - dir name
   @param String - file name
   @return int16_t - count
*/
uint16_t MicroSd::FileCount(fs::FS &fs, String DirName, String FileName) {
  uint16_t FileCount = 0;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("File name count: %s\n", DirName.c_str());
#endif

    File root = fs.open(DirName.c_str());
    if (!root) {
      Serial.println(F("Failed to open directory"));
      return 0;
    }
    if (!root.isDirectory()) {
      Serial.println(F("Not a directory"));
      return 0;
    }

    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
#if (true == CONSOLE_VERBOSE_DEBUG)
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.print(file.size());
#endif
        if (String(file.name()).indexOf(FileName) != -1) {
          FileCount++;
#if (true == CONSOLE_VERBOSE_DEBUG)
          Serial.print(" - MATCH");
#endif
        }
#if (true == CONSOLE_VERBOSE_DEBUG)
        Serial.println("");
#endif
      }
      file = root.openNextFile();
    }
  }

  return FileCount;
}

/**
   @brief Get card detected status
   @param none
   @return bool - status
*/
bool MicroSd::GetCardDetectedStatus() {
  return CardDetected;
}

/**
   @brief Get card size
   @param none
   @return uint16_t - size
*/
uint16_t MicroSd::GetCardSize() {
  return CardSize;
}

/**
   @brief Get card detect after boot
   @param none
   @return bool - status
*/
bool MicroSd::GetCardDetectAfterBoot() {
  return DetectAfterBoot;
}

/* EOF */