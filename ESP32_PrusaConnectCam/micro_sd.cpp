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
  DetectAfterBoot = false;
  /* recursive: these methods nest (ReinitCard -> InitSdCard -> CheckCardUsedStatus) */
  sdCardMutex = xSemaphoreCreateRecursiveMutex();
}

/**
   @brief Reinit micro SD card
   @param none
   @return none
*/
void MicroSd::ReinitCard() {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  Serial.println(F("Reinit micro SD card!"));
  Serial.println(F("Deinit micro SD card"));
  SD_MMC.end();
  delay(50);
  Serial.println(F("Init micro SD card"));
  InitSdCard();
  xSemaphoreGiveRecursive(sdCardMutex);
}

/**
 * @brief Open file
 * 
 * @param i_file - file
 * @param i_path - path and file name
 * @return true 
 * @return false 
 */
bool MicroSd::OpenFile(File *i_file, String i_path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
    bool status = false;

  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.println("Opening file: " + i_path);
#endif

    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("No SD card detected");
        CardDetected = false;
    } else {

      *i_file = SD_MMC.open(i_path.c_str(), FILE_APPEND);
      if (!*i_file) {
#if (true == CONSOLE_VERBOSE_DEBUG)
        Serial.println("Failed to open file");
#endif
        CardDetected = false;
      } else {
        status = true;
#if (true == CONSOLE_VERBOSE_DEBUG)
        Serial.println("File opened");
#endif
      }
    }
  }
  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
 * @brief Close file
 * 
 * @param i_file - file
 */
void MicroSd::CloseFile(File *i_file) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  if (*i_file) {
    i_file->close();
  }
  xSemaphoreGiveRecursive(sdCardMutex);
}

/**
 * @brief Check if file is opened
 * 
 * @param i_file - file
 * @return true 
 * @return false 
 */
bool MicroSd::CheckOpenFile(File *i_file) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  bool status = true;
  if (!*i_file) {
#if (true == CONSOLE_VERBOSE_DEBUG)    
    Serial.println(F("File not opened!"));
#endif
    status = false;
  }
  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
   @brief Init SD card. And check, if is SD card inserted
   @param none
   @return none
*/
void MicroSd::InitSdCard() {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  /* Start INIT Micro SD card */
  Serial.println(F("Start init micro-SD Card"));

  /* set SD card to 1-line/1-bit mode. GPIO 4 is used for LED and for microSD card. But communication is slower. */
  /* https://github.com/espressif/arduino-esp32/blob/master/libraries/SD_MMC/src/SD_MMC.h */

  SD_MMC.setPins(SD_PIN_CLK, SD_PIN_CMD, SD_PIN_DATA0);

  /* SD_MMC_FREQUENCY: defined only by boards whose default clock fails */
#ifdef SD_MMC_FREQUENCY
  if (!SD_MMC.begin("/sdcard", true, false, SD_MMC_FREQUENCY)) {
#else
  if (!SD_MMC.begin("/sdcard", true)) {
#endif
    Serial.println(F("SD Card Mount Failed"));
    CardDetected = false;
    CardSizeMB = 0;
    xSemaphoreGiveRecursive(sdCardMutex);
    return;
  }

  /* check microSD card and card type */
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println(F("No SD_MMC card attached"));
    CardDetected = false;
    CardSizeMB = 0;
    xSemaphoreGiveRecursive(sdCardMutex);
    return;
  }

  /* print card type */
  Serial.print(F("Found card. Card Type: "));
  if (cardType == CARD_MMC) {
    Serial.println(F("MMC"));
  } else if (cardType == CARD_SD) {
    Serial.println(F("SDSC"));
  } else if (cardType == CARD_SDHC) {
    Serial.println(F("SDHC"));
  } else {
    Serial.println(F("UNKNOWN"));
  }

  CardDetected = true;
  DetectAfterBoot = true;

  /* calculation card size */
  CheckCardUsedStatus();
  xSemaphoreGiveRecursive(sdCardMutex);
}

/**
   @brief List directory on the micro SD card
   @param fs::FS - card
   @param String - Directory name
   @param uint8_t - levels
   @return none
*/
void MicroSd::ListDir(fs::FS &fs, String DirName, uint8_t levels) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  if (true == CardDetected) {
    Serial.printf("Listing directory: %s\n", DirName.c_str());

    File root = fs.open(DirName.c_str());
    if (!root) {
      Serial.println(F("Failed to open directory"));
      xSemaphoreGiveRecursive(sdCardMutex);
      return;
    }
    if (!root.isDirectory()) {
      Serial.println(F("Not a directory"));
      xSemaphoreGiveRecursive(sdCardMutex);
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
  xSemaphoreGiveRecursive(sdCardMutex);
}

/**
   @brief Check directory on the micro SD card
   @param fs::FS - card
   @param String - dir name
   @return bool - status
*/
bool MicroSd::CheckDir(fs::FS &fs, String path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  bool status = false;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Checking Dir: %s... ", path.c_str());
#endif
  
    if (fs.exists(path.c_str())) {
      status = true;
    }

  }
  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
   @brief List directory on the micro SD card
   @param fs::FS - card
   @param String - dir name
   @return bool - status
*/
bool MicroSd::CreateDir(fs::FS &fs, String path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
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
  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
   @brief remove directory on the micro SD card
   @param fs::FS - card
   @param String - dir name
   @return bool - status
*/
bool MicroSd::RemoveDir(fs::FS &fs, String path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
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
  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
   @brief Read file and print data to console
   @param fs::FS - card
   @param String - file name
   @return none
*/
void MicroSd::ReadFileConsole(fs::FS &fs, String path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  if (true == CardDetected) {
    Serial.printf("Reading file: %s\n", path.c_str());

    File file = fs.open(path.c_str());
    if (!file) {
      Serial.println(F("Failed to open file for reading"));
      xSemaphoreGiveRecursive(sdCardMutex);
      return;
    }

    Serial.print(F("Read from file: "));
    while (file.available()) {
      Serial.write(file.read());
    }
  }
  xSemaphoreGiveRecursive(sdCardMutex);
}

/**
   @brief Write message to file
   @param fs::FS - card
   @param String - file name
   @param String - message
   @return bool - status
*/
bool MicroSd::WriteFile(fs::FS &fs, String path, String message) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
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
  xSemaphoreGiveRecursive(sdCardMutex);
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
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
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
  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
   @brief Added text to end of file
   @param File - file
   @param String - message
   @return bool - status
*/
bool MicroSd::AppendFile(File *i_file, String *i_msg) {
  /* take mutex */
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  bool status = false;

  /* check if card is corrupted */
  if (false == isCardCorrupted()) {
    xSemaphoreGiveRecursive(sdCardMutex);
    return false;
  }
  
  /* check if card is detected */
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("Appending to file:");
#endif

    /* check if file is opened */
    if (!*i_file) {
      Serial.println("File not opened");
      CardDetected = false;

    } else {
      /* write to file */
      if (i_file->print(i_msg->c_str())) {
        if (*i_file) {
          i_file->flush();

          /* check if write was OK */
          if (!i_file->getWriteError()) {
#if (true == CONSOLE_VERBOSE_DEBUG)
            Serial.println("Write OK");
#endif
            status = true;

          } else {
            Serial.println(F("Failed write to file"));

          }
        } else {
          Serial.println(F("File not opened!"));

        }
      } else {
        Serial.println(F("Failed write to file!"));

      }
#if (true == CONSOLE_VERBOSE_DEBUG)
      Serial.println((status == true) ? "Message appended" : "Append Failed");
#endif
    }
  }

  /* give mutex */
  xSemaphoreGiveRecursive(sdCardMutex);
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
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
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

  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
   @brief Delete file on the SD card
   @param fs::FS - card
   @param String - file name
   @return bool - status
*/
bool MicroSd::DeleteFile(fs::FS &fs, String path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
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

  xSemaphoreGiveRecursive(sdCardMutex);
  return status;
}

/**
   @brief Get file size in the kb
   @param fs::FS - card
   @param String - file name
   @return uint32_t - size
*/
uint32_t MicroSd::GetFileSize(fs::FS &fs, String path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
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
      xSemaphoreGiveRecursive(sdCardMutex);
      return 0;
    }

    ret = file.size() / 1024; /* convert from bytes to kb */
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf(" File size: %d\n ", ret);
#endif
  }

  xSemaphoreGiveRecursive(sdCardMutex);
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
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  uint16_t FileCount = 0;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.printf("File name count: %s\n", DirName.c_str());
#endif

    File root = fs.open(DirName.c_str());
    if (!root) {
      Serial.println(F("Failed to open directory"));
      xSemaphoreGiveRecursive(sdCardMutex);
      return 0;
    }
    if (!root.isDirectory()) {
      Serial.println(F("Not a directory"));
      xSemaphoreGiveRecursive(sdCardMutex);
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

  xSemaphoreGiveRecursive(sdCardMutex);
  return FileCount;
}

/**
   @brief Remove files in directory
   @param fs::FS - card
   @param String - dir name
   @param int - max files
   @return bool - status
*/
bool MicroSd::RemoveFilesInDir(fs::FS &fs, String path, int maxFiles) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  bool ret = false;
  File dir = fs.open(path.c_str());
  if (!dir) {
    xSemaphoreGiveRecursive(sdCardMutex);
    return ret;
  }

  int fileCount = 0;
  File file = dir.openNextFile();
  while (file) {
    ret = true;
    String fileName = path + "/" + file.name();
    fs.remove(fileName.c_str());
    Serial.printf("Removing file: %s\n", fileName.c_str());
    fileCount++;
    if (fileCount >= maxFiles) {
      break;
    }
    file = dir.openNextFile();
  }

  xSemaphoreGiveRecursive(sdCardMutex);
  return ret;
}


int MicroSd::CountFilesInDir(fs::FS &fs, String path) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  uint16_t file_count = FileCount(fs, path, "");
  xSemaphoreGiveRecursive(sdCardMutex);
  return file_count;
}

/**
   @brief Check card used status
   @param none
   @return bool - status
*/
void MicroSd::CheckCardUsedStatus() {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);

  CardSizeMB = SD_MMC.cardSize()  / (1024 * 1024);
  CardTotalMB = SD_MMC.totalBytes() / (1024 * 1024);
  CardUsedMB = SD_MMC.usedBytes() / (1024 * 1024);

  /* guard divide by zero (no card) and unsigned wrap (used > size) */
  if (0 == CardSizeMB) {
    CardFreeMB = 0;
    FreeSpacePercent = 0;
    UsedSpacePercent = 0;
  } else {
    CardFreeMB = (CardUsedMB >= CardSizeMB) ? 0 : (CardSizeMB - CardUsedMB);
  FreeSpacePercent = (CardFreeMB * 100) / CardSizeMB;
  UsedSpacePercent = 100 - FreeSpacePercent;
  }

#if (true == CONSOLE_VERBOSE_DEBUG)  
  Serial.printf("Card size: %d MB, Total: %d MB, Used: %d MB, Free: %d GB, Free: %d %% \n", CardSizeMB, CardTotalMB, CardUsedMB, CardFreeMB, FreeSpacePercent);
#endif

  xSemaphoreGiveRecursive(sdCardMutex);
}

/**
 * @brief Function to check if card is corrupted
 * 
 * @return true 
 * @return false 
 */
bool MicroSd::isCardCorrupted() {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
  bool ret = true;
  if (true == CardDetected) {
#if (true == CONSOLE_VERBOSE_DEBUG)   
    //Serial.println(F("Checking card..."));
#endif

    /* use == 0 is valid (empty card); size == 0 means no card */
    uint64_t size = SD_MMC.cardSize();
    uint64_t use = SD_MMC.usedBytes();

#if (true == CONSOLE_VERBOSE_DEBUG)   
    Serial.printf("Card size: %llu, Used: %llu\n", size, use);
#endif

    if (size == 0) {
      Serial.println(F("No card detected!"));
      CardDetected = false;
      ret = false;

    } else if (use >= size) {
      Serial.println(F("No space left on device!"));
      CardDetected = false;
      ret = false;
    }

  } else {
    ret = false;
  }

  xSemaphoreGiveRecursive(sdCardMutex);
  return ret;
}

/**
   @brief Write picture to the SD card
   @param fs::FS - card
   @param String - file name
   @return String - data
*/  
bool MicroSd::WritePicture(String i_PhotoName, uint8_t *i_PhotoData, size_t i_PhotoLen) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);
#if (true == CONSOLE_VERBOSE_DEBUG)  
  Serial.println(F("WritePicture"));
#endif
  bool ret_stat = false;

  File file = SD_MMC.open(i_PhotoName, FILE_WRITE);

  if (file) {
    size_t ret = 0;

    ret = file.write(i_PhotoData, i_PhotoLen);
    if (ret != i_PhotoLen) {
      Serial.println(F("Failed. Error while writing to file"));
    } else {
      Serial.printf("Saved as %s\n", i_PhotoName.c_str());
      ret_stat = true;
    }
    file.close();
  } else {
    Serial.printf("Failed. Could not open file: %s\n", i_PhotoName.c_str());
  }
  
  xSemaphoreGiveRecursive(sdCardMutex);
 return ret_stat;
}

/**
   @brief Write picture to the SD card with EXIF data
   @param fs::FS - card
   @param String - file name
   @param uint8_t - data
   @param size_t - data length
   @param const uint8_t - EXIF data
   @param size_t - EXIF data length
   @return bool - status
*/
bool MicroSd::WritePicture(String i_PhotoName, uint8_t *i_PhotoData, size_t i_PhotoLen, const uint8_t *i_PtohoExif, size_t i_PhotoExifLen) {
  xSemaphoreTakeRecursive(sdCardMutex, portMAX_DELAY);

#if (true == CONSOLE_VERBOSE_DEBUG)  
  Serial.println(F("WritePicture EXIF"));
#endif
  bool ret_stat = false;

  File file = SD_MMC.open(i_PhotoName, FILE_WRITE);

  if (file) {
    size_t ret = 0;

    ret = file.write(i_PtohoExif, i_PhotoExifLen);
    ret += file.write(i_PhotoData, i_PhotoLen);
    if (ret != (i_PhotoLen + i_PhotoExifLen)) {
#if (true == CONSOLE_VERBOSE_DEBUG)        
      Serial.println(F("Failed. Error while writing to file"));
#endif
      ret_stat = false;
    } else {
#if (true == CONSOLE_VERBOSE_DEBUG)  
      Serial.printf("Saved as %s\n", i_PhotoName.c_str());
#endif
      ret_stat = true;
    }
    file.close();
  } else {
#if (true == CONSOLE_VERBOSE_DEBUG)  
    Serial.printf("Failed. Could not open file: %s\n", i_PhotoName.c_str());
#endif
    ret_stat = false;
  }
  
  xSemaphoreGiveRecursive(sdCardMutex);
 return ret_stat;
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
uint16_t MicroSd::GetCardSizeMB() {
  return CardSizeMB;
}

/**
   @brief Get card detect after boot
   @param none
   @return bool - status
*/
bool MicroSd::GetCardDetectAfterBoot() {
  return DetectAfterBoot;
}

/**
   @brief Get card total MB
   @param none
   @return uint16_t - size
*/
uint16_t MicroSd::GetCardTotalMB() {
  return CardTotalMB;
}

/**
   @brief Get card used MB
   @param none
   @return uint16_t - size
*/
uint16_t MicroSd::GetCardUsedMB() {
  return CardUsedMB;
}

/**
   @brief Get card free MB
   @param none
   @return uint16_t - size
*/
uint16_t MicroSd::GetCardFreeMB() {
  return CardFreeMB;
}

/**
   @brief Get free space percent
   @param none
   @return uint8_t - percent
*/
uint8_t MicroSd::GetFreeSpacePercent() {
  return FreeSpacePercent;
}

/**
   @brief Get used space percent
   @param none
   @return uint8_t - percent
*/
uint8_t MicroSd::GetUsedSpacePercent() {
  return UsedSpacePercent;
}

/* EOF */