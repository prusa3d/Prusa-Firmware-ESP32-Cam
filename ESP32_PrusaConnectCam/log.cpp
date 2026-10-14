/**
   @file log.cpp

   @brief log library

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/
#include "log.h"

Logs SystemLog(LOGS_FILE_PATH, LOGS_FILE_MAX_SIZE);

/* IDF log tags of the micro SD card driver. Disabled when the card is removed */
static const char *const Logs_SdLogTags[LOGS_SD_LOG_TAGS_COUNT] = { "sdmmc_common", "sdmmc_cmd", "sdmmc_req", "diskio_sdmmc", "vfs_fat_sdmmc" };

/**
   @info Constructor
   @param none
   @return none
*/
Logs::Logs() {
  FileName = F("log.txt");
  FilePath = F("/");
  LogLevel = LogLevel_Verbose;
  FileMaxSize = 1024;
  NtpTimeSynced = false;
  LogFd = -1;
  LogFileIndex = -1;
  LogFileSize = 0;
  LogFileDirty = false;
  LastFlushTime = 0;
  LogQueue = NULL;
  LoggerRunning = false;
  DroppedMsgCount = 0;
  FlushRequestCount = 0;
  FlushDoneCount = 0;
  LastCardRetryTime = 0;
  SdLogQuiet = false;
  LogCardFullState = false;
}

/**
 * @brief Construct a new Logs:: Logs object with file path and file size. The file name is created at each start: prefix + NNNN + suffix
 *
 * @param String - file path
 * @param uint16_t - file size [kb]
 */
Logs::Logs(String i_FilePath, uint16_t i_FileSize) {
  FileName = "";
  FilePath = i_FilePath;
  LogLevel = LogLevel_Verbose;
  FileMaxSize = i_FileSize;
  NtpTimeSynced = false;
  LogFd = -1;
  LogFileIndex = -1;
  LogFileSize = 0;
  LogFileDirty = false;
  LastFlushTime = 0;
  LogQueue = NULL;
  LoggerRunning = false;
  DroppedMsgCount = 0;
  FlushRequestCount = 0;
  FlushDoneCount = 0;
  LastCardRetryTime = 0;
  SdLogQuiet = false;
  LogCardFullState = false;
}

/**
 * @brief Construct a new Logs:: Logs object  with file path and file name
 *
 * @param String - file path
 * @param String - file name
 */
Logs::Logs(String i_FilePath, String i_FileName) {
  FileName = i_FileName;
  FilePath = i_FilePath;
  LogLevel = LogLevel_Verbose;
  FileMaxSize = 1024;
  NtpTimeSynced = false;
  LogFd = -1;
  LogFileIndex = -1;
  LogFileSize = 0;
  LogFileDirty = false;
  LastFlushTime = 0;
  LogQueue = NULL;
  LoggerRunning = false;
  DroppedMsgCount = 0;
  FlushRequestCount = 0;
  FlushDoneCount = 0;
  LastCardRetryTime = 0;
  SdLogQuiet = false;
  LogCardFullState = false;
}

/**
 * @brief Construct a new Logs:: Logs object with log level, file path and file name
 * 
 * @param LogLevel_enum - log level
 * @param String - file path
 * @param String - file name
 */
Logs::Logs(LogLevel_enum i_LogLevel, String i_FilePath, String i_FileName) {
  FileName = i_FileName;
  FilePath = i_FilePath;
  LogLevel = i_LogLevel;
  FileMaxSize = 1024;
  NtpTimeSynced = false;
  LogFd = -1;
  LogFileIndex = -1;
  LogFileSize = 0;
  LogFileDirty = false;
  LastFlushTime = 0;
  LogQueue = NULL;
  LoggerRunning = false;
  DroppedMsgCount = 0;
  FlushRequestCount = 0;
  FlushDoneCount = 0;
  LastCardRetryTime = 0;
  SdLogQuiet = false;
  LogCardFullState = false;
}

/**
 * @brief Construct a new Logs:: Logs object with file path, file name and file size
 * 
 * @param String - file path
 * @param String - file name
 * @param uint16_t - file size
 */
Logs::Logs(String i_FilePath, String i_FileName, uint16_t i_FileSize) {
  FileName = i_FileName;
  FilePath = i_FilePath;
  LogLevel = LogLevel_Verbose;
  FileMaxSize = i_FileSize;
  NtpTimeSynced = false;
  LogFd = -1;
  LogFileIndex = -1;
  LogFileSize = 0;
  LogFileDirty = false;
  LastFlushTime = 0;
  LogQueue = NULL;
  LoggerRunning = false;
  DroppedMsgCount = 0;
  FlushRequestCount = 0;
  FlushDoneCount = 0;
  LastCardRetryTime = 0;
  SdLogQuiet = false;
  LogCardFullState = false;
}

/**
 * @brief Construct a new Logs:: Logs object with log level, file path, file name and file size
 * 
 * @param LogLevel_enum - log level
 * @param String - file path
 * @param String - file name
 * @param uint16_t - file size
 */
Logs::Logs(LogLevel_enum i_LogLevel, String i_FilePath, String i_FileName, uint16_t i_FileSize) {
  FileName = i_FileName;
  FilePath = i_FilePath;
  LogLevel = i_LogLevel;
  FileMaxSize = i_FileSize;
  NtpTimeSynced = false;
  LogFd = -1;
  LogFileIndex = -1;
  LogFileSize = 0;
  LogFileDirty = false;
  LastFlushTime = 0;
  LogQueue = NULL;
  LoggerRunning = false;
  DroppedMsgCount = 0;
  FlushRequestCount = 0;
  FlushDoneCount = 0;
  LastCardRetryTime = 0;
  SdLogQuiet = false;
  LogCardFullState = false;
}

/**
   @info Init library for logs
   @param LogLevel_enum - log level
   @param String - file path
   @param String - file name
   @return none
*/
void Logs::Init() {
  Serial.println(F("----------------------------------------------------------------"));
  Serial.println(F("Init Logs library"));

  /* init micro SD card */
#if (true == ENABLE_SD_CARD)
  InitSdCard();

  if (true == GetCardDetectedStatus()) {
    /* new log file for this start, with first message after start MCU */
    if (true == LogFileStart(false)) {
      Serial.println("Log file: " + FilePath + FileName);
    } else {
      Serial.println(F("Failed to open log file!"));
    }

  } else {
    Serial.println(F("Micro-SD card not found! Logs only to console"));

    /* the logger task tries to mount the card later, without IDF error messages */
    SetSdLogQuiet(true);
    LastCardRetryTime = millis();
  }
#else
  Serial.println(F("Micro-SD card not enabled! Disable logs to card"));
#endif

  /* create queue for log messages. Until the logger task is running, the messages are printed directly to the console */
  LogQueue = xRingbufferCreateWithCaps(LOGS_QUEUE_SIZE, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);
  if (NULL == LogQueue) {
    Serial.println(F("Logs: queue create failed, synchronous mode"));
  }
}

/**
   @info Start of the logger task. From now on, AddEvent sends log messages to the queue
   @param none
   @return bool - true = queue for log messages exists, logger task can run
*/
bool Logs::LoggerStart() {
  if (NULL == LogQueue) {
    return false;
  }

  LoggerRunning = true;
  return true;
}

/**
   @info One cycle of the logger task. Only the logger task prints log messages to the console and writes them to the log file
   @param none
   @return none
*/
void Logs::LoggerProcess() {
  size_t item_size = 0;
  char *item = (char *)xRingbufferReceive(LogQueue, &item_size, pdMS_TO_TICKS(LOGS_FLUSH_TIMEOUT));

  /* report dropped messages before next message */
  uint32_t dropped = DroppedMsgCount.exchange(0);
  if (dropped > 0) {
    LoggerWriteStatus(String(dropped) + " log messages dropped");
  }

  /* write log message. When the queue is empty, save data to the card */
  if (NULL != item) {
    LoggerWrite(item, item_size);
    vRingbufferReturnItem(LogQueue, item);
  } else {
    LogFileFlush();
  }

  /* request from LoggerFlush(): write all log messages from the queue and save the log file now */
  uint32_t flush_request = FlushRequestCount;
  if (flush_request != FlushDoneCount) {
    while (NULL != (item = (char *)xRingbufferReceive(LogQueue, &item_size, 0))) {
      LoggerWrite(item, item_size);
      vRingbufferReturnItem(LogQueue, item);
    }
    LogFileFlush();
    FlushDoneCount = flush_request;
  }

#if (true == ENABLE_SD_CARD)
  /* mount the micro SD card again, when the log file is not opened */
  if (LogFd < 0) {
    LogCardRetry();
  }
#endif
}

/**
   @info Write all log messages from the queue to the console and to the log file now. Used before MCU restart, otherwise the last log messages are lost.
         The logger task writes the messages, the calling task waits until it is done, maximum i_timeout
   @param uint32_t - maximum waiting time [ms]
   @return none
*/
void Logs::LoggerFlush(uint32_t i_timeout) {
  /* synchronous mode, the messages are already on the console */
  if (false == LoggerRunning) {
    return;
  }

  uint32_t flush_request = ++FlushRequestCount;
  uint32_t start_time = millis();
  while (((int32_t)(FlushDoneCount - flush_request) < 0) && ((millis() - start_time) < i_timeout)) {
    delay(10);
  }
}

/**
   @info Send log message to the queue for the logger task. Without waiting, when the queue is full, the message is dropped
   @param String * - log message
   @return none
*/
void Logs::LogEnqueue(String *i_msg) {
  if (0 == i_msg->length()) {
    return;
  }

  /* cut too long message */
  size_t keep_len = LogFile_MsgKeepLength(i_msg->length(), LOGS_MSG_MAX_LEN);
  if (keep_len < i_msg->length()) {
    i_msg->remove(keep_len);
    *i_msg += LOG_FILE_TRUNCATE_TAIL;
  }

  if (true == LoggerRunning) {
    if (pdFALSE == xRingbufferSend(LogQueue, i_msg->c_str(), i_msg->length(), 0)) {
      DroppedMsgCount++;
    }
  } else {
    /* synchronous mode, logger task is not running */
    Serial.print(*i_msg);
  }
}

/**
   @info Write log message to the console and to the log file. Called only from the logger task
   @param const char * - log message
   @param size_t - length of the log message
   @return none
*/
void Logs::LoggerWrite(const char *i_data, size_t i_len) {
  Serial.write((const uint8_t *)i_data, i_len);

#if (true == ENABLE_SD_CARD)
  /* append log message to log file */
  SdLock();
  if (LogFd >= 0) {
    errno = 0;
    ssize_t ret = write(LogFd, i_data, i_len);
    if (ret == (ssize_t)i_len) {
      LogFileSize += i_len;
      LogFileDirty = true;

      /* card was full and has free space again */
      if (true == LogCardFullState) {
        LogCardFullState = false;
        LoggerWriteStatus("Micro-SD card has free space, logs to card again");
      }

      /* save data to the card at least every LOGS_FLUSH_TIMEOUT */
      if ((millis() - LastFlushTime) >= LOGS_FLUSH_TIMEOUT) {
        LogFileFlush();
      }

      /* continue in the new log file */
      CheckMaxLogFileSize();
    } else if (true == LogFile_IsCardFull((int32_t)ret, i_len, errno)) {
      LogCardFull();
    } else {
      LogCardLost();
    }
  }
  SdUnlock();
#endif
}

/**
   @info Write status message of the logger to the console and to the log file. Not use the queue
   @param const String & - status message
   @return none
*/
void Logs::LoggerWriteStatus(const String &i_msg) {
  String log_msg = GetSystemTime() + " - " + i_msg + "\n";
  LoggerWrite(log_msg.c_str(), log_msg.length());
}

/**
   @info Open the log file. New log file for the first open in this start and for continue (i_continue), otherwise reopen the log file of this start
   @param bool - true = continue in the new log file, when the log file has maximum size
   @return bool - log file opened
*/
bool Logs::LogFileStart(bool i_continue) {
#if (true == ENABLE_SD_CARD)
  bool first_open = (LogFileIndex < 0);
  bool new_file = ((true == first_open) || (true == i_continue));
  String file_name = FileName;
  int32_t file_index = LogFileIndex;
  String dir_path = String(SD_CARD_MOUNT_POINT) + FilePath;

  SdLock();
  if ((false == GetCardDetectedStatus()) || (LogFd >= 0)) {
    bool ret = (LogFd >= 0);
    SdUnlock();
    return ret;
  }

  if (true == new_file) {
    /* find log files in the directory */
    std::vector<int32_t> indexes;
    DIR *dir = opendir(dir_path.c_str());
    if (NULL == dir) {
      SdUnlock();
      return false;
    }

    struct dirent *entry = NULL;
    while (NULL != (entry = readdir(dir))) {
      int32_t index = LogFile_ParseIndex(entry->d_name, LOGS_FILE_PREFIX, LOGS_FILE_SUFFIX);
      if (index >= 0) {
        indexes.push_back(index);
      }
    }
    closedir(dir);

    /* index of the new log file. After 9999 remove all log files and start from 0 */
    int32_t next_index = LogFile_NextIndex(indexes.data(), indexes.size());
    size_t delete_count = 0;
    if (next_index < 0) {
      next_index = 0;
      delete_count = indexes.size();
    } else {
      delete_count = LogFile_SelectToDelete(indexes.data(), indexes.size(), LOGS_FILE_MAX_COUNT);
    }

    /* remove the oldest log files */
    char name[32];
    for (size_t i = 0; i < delete_count; i++) {
      LogFile_FormatName(name, sizeof(name), LOGS_FILE_PREFIX, (uint16_t)indexes[i], LOGS_FILE_SUFFIX);
      unlink((dir_path + name).c_str());
    }

    LogFile_FormatName(name, sizeof(name), LOGS_FILE_PREFIX, (uint16_t)next_index, LOGS_FILE_SUFFIX);
    file_name = name;
    file_index = next_index;
  }

  /* open log file */
  LogFd = open((dir_path + file_name).c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
  if (LogFd < 0) {
    SdUnlock();
    return false;
  }

  String previous_file = FileName;
  FileName = file_name;
  LogFileIndex = file_index;

  struct stat file_stat;
  LogFileSize = (0 == fstat(LogFd, &file_stat)) ? (uint32_t)file_stat.st_size : 0;
  LogFileDirty = false;
  LastFlushTime = millis();

  /* first message in the new log file */
  bool ret = true;
  if (true == first_open) {
    String header = F("----------------------------------------------------------------\n");
    header += F("Start MCU!\nSW Version: ");
    header += String(SW_VERSION);
    header += F(" ,Build: ");
    header += String(SW_BUILD);
    header += "\n";
    header += F("Verbose mode: ");
    header += (true == CONSOLE_VERBOSE_DEBUG) ? "true" : "false";
    header += "\n";
    header += F("Log level: ");
    header += String(LogLevel);
    header += "\n";
    ret = LogFileWriteHeader(header);
  } else if (true == i_continue) {
    ret = LogFileWriteHeader(GetSystemTime() + " - Log continued from " + previous_file + "\n");
  }

  SdUnlock();
  return ret;
#else
  return false;
#endif
}

/**
   @info Write first message to the new log file. Only to the file, not to the console
   @param const String & - message
   @return bool - status
*/
bool Logs::LogFileWriteHeader(const String &i_msg) {
  bool ret = false;
#if (true == ENABLE_SD_CARD)
  SdLock();
  if (LogFd >= 0) {
    errno = 0;
    ssize_t written = write(LogFd, i_msg.c_str(), i_msg.length());
    if (written == (ssize_t)i_msg.length()) {
      LogFileSize += i_msg.length();
      LogFileDirty = true;
      ret = true;
    } else if (true == LogFile_IsCardFull((int32_t)written, i_msg.length(), errno)) {
      LogCardFull();
    } else {
      LogCardLost();
    }
  }
  SdUnlock();
#endif
  return ret;
}

/**
   @info Save data of the log file to the card (fsync), when the log file has data without fsync
   @param none
   @return none
*/
void Logs::LogFileFlush() {
#if (true == ENABLE_SD_CARD)
  SdLock();
  if ((LogFd >= 0) && (true == LogFileDirty)) {
    if (0 == fsync(LogFd)) {
      LogFileDirty = false;
    } else {
      LogCardLost();
    }
    LastFlushTime = millis();
  }
  SdUnlock();
#endif
}

/**
   @info Micro SD card removed or write to the log file failed. Close the log file and unmount the card. Next messages are only on the console
   @param none
   @return none
*/
void Logs::LogCardLost() {
#if (true == ENABLE_SD_CARD)
  SdLock();
  LogCloseFile();
  LogCardUnmount();
  LastCardRetryTime = millis();
  SdUnlock();

  /* log file is closed, message is only on the console */
  LoggerWriteStatus("Micro-SD card removed or write failed");
#if (false == ENABLE_SD_CARD_HOTPLUG)
  LoggerWriteStatus("Micro-SD card hotplug is disabled. Logs only to console, the card is used again after restart");
#endif
  SetSdLogQuiet(true);
#endif
}

/**
   @info Unmount the micro SD card after an error. Without ENABLE_SD_CARD_HOTPLUG the card is only marked as not detected, because SD_MMC.end()/begin() during runtime breaks the camera on ESP32
   @param none
   @return none
*/
void Logs::LogCardUnmount() {
#if (true == ENABLE_SD_CARD_HOTPLUG)
  DeinitSdCard();
#else
  SetCardDetectedStatus(false);
#endif
}

/**
   @info Micro SD card is full. Close the log file, but the card stays mounted (timelapse, web). The logger tries to open the log file again every LOGS_CARD_RETRY
   @param none
   @return none
*/
void Logs::LogCardFull() {
#if (true == ENABLE_SD_CARD)
  SdLock();
  LogCloseFile();
  LastCardRetryTime = millis();
  SdUnlock();

  /* message only once, when the card is full */
  if (false == LogCardFullState) {
    LogCardFullState = true;
    LoggerWriteStatus("Micro-SD card full, logs only to console");
  }
#endif
}

/**
   @info Mount the micro SD card again every LOGS_CARD_RETRY and continue in the log file of this start
   @param none
   @return none
*/
void Logs::LogCardRetry() {
#if (true == ENABLE_SD_CARD)
  if ((millis() - LastCardRetryTime) < LOGS_CARD_RETRY) {
    return;
  }
  LastCardRetryTime = millis();

  SdLock();
#if (true == ENABLE_SD_CARD_HOTPLUG)
  /* mount the card again. Without hotplug the card is mounted only at start */
  if (false == GetCardDetectedStatus()) {
    ReinitCard(false);
  }
#endif

  if (true == GetCardDetectedStatus()) {
    bool first_file = (LogFileIndex < 0);
    SetSdLogQuiet(false);

    if (true == LogFileStart(false)) {
      /* full card: without message, the next successful write prints the message */
      if (false == LogCardFullState) {
        LoggerWriteStatus((true == first_file) ? "Micro-SD card detected" : "Micro-SD card reconnected");
      }
    } else if ((true == GetCardDetectedStatus()) && (false == LogCardFullState)) {
      /* log file can not be opened, try it again later. Full card stays mounted */
      LogCardUnmount();
      SetSdLogQuiet(true);
    }
  }
  SdUnlock();
#endif
}

/**
   @info Disable or enable the IDF log of the micro SD card driver. Attempts to mount the removed card do not print errors
   @param bool - true = disable log, false = restore saved log levels
   @return none
*/
void Logs::SetSdLogQuiet(bool i_quiet) {
  if (i_quiet == SdLogQuiet) {
    return;
  }

  for (size_t i = 0; i < LOGS_SD_LOG_TAGS_COUNT; i++) {
    if (true == i_quiet) {
      SdLogLevel[i] = esp_log_level_get(Logs_SdLogTags[i]);
      esp_log_level_set(Logs_SdLogTags[i], ESP_LOG_NONE);
    } else {
      esp_log_level_set(Logs_SdLogTags[i], SdLogLevel[i]);
    }
  }
  SdLogQuiet = i_quiet;
}

/**
 * @brief Open log file
 * 
 */
void Logs::LogOpenFile() {
#if (true == ENABLE_SD_CARD)
  /* continue in the log file of this start */
  LogFileStart(false);
#endif
}

/**
 * @brief Close log file
 *
 */
void Logs::LogCloseFile() {
#if (true == ENABLE_SD_CARD)
  SdLock();
  if (LogFd >= 0) {
    close(LogFd);
    LogFd = -1;
  }
  LogFileDirty = false;
  SdUnlock();
#endif
}

/**
 * @brief Function for check opened log file. The status of the log file is the file descriptor, see GetLogFileOpened
 *
 */
void Logs::LogCheckOpenedFile() {
}

/**
   @info set log level
   @param LogLevel_enum - log level
   @return none
*/
void Logs::SetLogLevel(LogLevel_enum level) {
  LogLevel = level;
}

/**
   @info Add new log event
   @param LogLevel_enum - log level
   @param String - log message
   @param bool - new line
   @param bool - date
   @return none
*/
void Logs::AddEvent(LogLevel_enum level, String msg, bool newLine, bool date) {
  /* check log level */
  if (LogLevel >= level) {

    /* create log message */
    String log_msg = "";
    if (true == date) {
      log_msg += GetSystemTime();
      log_msg += " - ";
    }
    log_msg += msg;
    if (true == newLine) {
      log_msg += "\n";
    }

    /* send log message to the logger task */
    LogEnqueue(&log_msg);
  }
#if (true == CONSOLE_VERBOSE_DEBUG)
  else {
    Serial.println(msg);
  }
#endif
}

/**
   @info Add new log event
   @param LogLevel_enum - log level
   @param const __FlashStringHelper - log message
   @param String - parameters
   @param bool - new line
   @param bool - date
   @return none
*/
void Logs::AddEvent(LogLevel_enum level, const __FlashStringHelper *msg, String parameters, bool newLine, bool date) {
  /* check log level */
  if (LogLevel >= level) {

    /* create log message */
    String log_msg = "";
    if (true == date) {
      log_msg += GetSystemTime();
      log_msg += " - ";
    }
    log_msg += msg;
    log_msg += parameters;
    if (true == newLine) {
      log_msg += "\n";
    }

    /* send log message to the logger task */
    LogEnqueue(&log_msg);
  }
#if (true == CONSOLE_VERBOSE_DEBUG)
  else {
    Serial.println(msg);
  }
#endif
 }

/**
   @info Set file name
   @param String - file name
   @return none
*/
void Logs::SetFileName(String i_data) {
  FileName = i_data;
}

/**
   @info Set file path
   @param String - file path
   @return none
*/
void Logs::SetFilePath(String i_data) {
  FilePath = i_data;
}

/**
   @info Set file max size
   @param uint16_t - file max size
   @return none
*/
void Logs::SetFileMaxSize(uint16_t i_data) {
  FileMaxSize = i_data;
}

/**
   @info Set NTP time synced
   @param bool - NTP time synced
   @return none
*/
void Logs::SetNtpTimeSynced(bool i_data) {
  NtpTimeSynced = i_data;
  AddEvent(LogLevel_Info, F("System time: "), GetSystemTime());
}

/**
   @info Get file name
   @param none
   @return String - file name
*/
String Logs::GetFileName() {
  /* the logger task changes the file name under the mutex for SD card */
  SdLock();
  String ret = FileName;
  SdUnlock();
  return ret;
}

/**
   @info Get file path
   @param none
   @return String - file path
*/
String Logs::GetFilePath() {
  return FilePath;
}

/**
   @info Get log level
   @param none
   @return LogLevel_enum - log level
*/
LogLevel_enum Logs::GetLogLevel() {
  return LogLevel;
}

/**
   @info Get NTP time synced
   @param none
   @return bool - NTP time synced
*/
bool Logs::GetNtpTimeSynced() {
  return NtpTimeSynced;
}

/**
   @info Check maximum log file size
   @param none
   @return none
*/
void Logs::CheckMaxLogFileSize() {
#if (true == ENABLE_SD_CARD)
  /* called from the logger task after each write, therefore without AddEvent */
  SdLock();
  if ((LogFd >= 0) && (FileMaxSize > 0) && (LogFileSize >= ((uint32_t)FileMaxSize * 1024))) {
    LogCloseFile();
    if (true == LogFileStart(true)) {
      LoggerWriteStatus("Maximum log file size. Log continued in the file: " + FileName);
    } else if ((LogFd < 0) && (false == LogCardFullState) && (true == GetCardDetectedStatus())) {
      /* new log file can not be opened, the card was probably removed */
      LogCardLost();
    }
  }
  SdUnlock();
#endif
}

/**
   @info Check card space and print card status and log file size
   @param none
   @return none
*/
void Logs::CheckCardSpace() {
#if (true == ENABLE_SD_CARD)
  CheckCardUsedStatus();
  AddEvent(LogLevel_Verbose, "Card size: " + String(GetCardSizeMB()) + " MB, Used: " + String(GetCardUsedMB()) + " MB, Free: " + String(GetCardFreeMB()) + " MB");
  AddEvent(LogLevel_Verbose, F("Log file size: "), String(LogFileSize / 1024) + "/" + String(FileMaxSize) + " KB");
#endif
}

/**
   @info Get log file opened
   @param none
   @return bool - log file opened
*/
bool Logs::GetLogFileOpened() {
  return (LogFd >= 0);
}

/**
   @info Get system time
   @param none
   @return String - time
*/
String Logs::GetSystemTime() {
  String ret = "0000-00-00_00-00-00";
  if (true == NtpTimeSynced) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
#if (true == CONSOLE_VERBOSE_DEBUG)
      Serial.println(F("Failed to obtain time"));
#endif
      return ret;
    }

    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d_%H-%M-%S", &timeinfo);
#if (true == CONSOLE_VERBOSE_DEBUG)
    Serial.println(timeString);
#endif
    ret = String(timeString);
  }
  return ret;
}

/**
   @info Mask sensitive data (token, password) for the log. Same as Configuration::LoadString, the data are visible only with CONSOLE_VERBOSE_DEBUG
   @param const String & - sensitive data
   @return String - data with all characters replaced by '*'
*/
String Logs::MaskSensitiveData(const String &i_data) {
  String ret = i_data;
#if (false == CONSOLE_VERBOSE_DEBUG)
  LogFile_MaskSecret(ret.begin());
#endif
  return ret;
}

/* EOF */