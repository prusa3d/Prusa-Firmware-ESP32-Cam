/**
   @file log.h

   @brief log library

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#pragma once

#include <Arduino.h>
#include <atomic>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <freertos/ringbuf.h>
#include <esp_task_wdt.h>
#include <esp_log.h>

#include "micro_sd.h"
#include "log_level.h"
#include "log_file.h"

#define LOGS_SD_LOG_TAGS_COUNT      5                       ///< count of the IDF log tags of the micro SD card driver

class Logs : public MicroSd {
private:
  LogLevel_enum LogLevel;                             ///< LogLevel
  String FileName;                                    ///< log File name
  String FilePath;                                    ///< log file patch
  uint16_t FileMaxSize;                               ///< log file max size
  bool NtpTimeSynced;                                 ///< status NTP time sync
  int LogFd;                                          ///< log file descriptor, -1 = file closed
  int32_t LogFileIndex;                               ///< index of the log file, -1 = not yet determined in this start
  uint32_t LogFileSize;                               ///< log file size [B]
  bool LogFileDirty;                                  ///< log file has data without fsync
  uint32_t LastFlushTime;                             ///< last fsync of the log file
  RingbufHandle_t LogQueue;                           ///< queue for log messages
  std::atomic<bool> LoggerRunning;                    ///< logger task is running, false = synchronous mode
  std::atomic<uint32_t> DroppedMsgCount;              ///< count of the dropped log messages
  std::atomic<uint32_t> FlushRequestCount;            ///< count of the requests for write all log messages now (LoggerFlush)
  std::atomic<uint32_t> FlushDoneCount;               ///< last request for write all log messages done by the logger task
  uint32_t LastCardRetryTime;                         ///< last attempt for mount the micro SD card
  bool SdLogQuiet;                                    ///< IDF log of the micro SD card driver is disabled
  bool LogCardFullState;                              ///< micro SD card is full, log file is closed
  esp_log_level_t SdLogLevel[LOGS_SD_LOG_TAGS_COUNT]; ///< saved IDF log levels of the micro SD card driver

  void LogEnqueue(String *);
  void LoggerWrite(const char *, size_t);
  void LoggerWriteStatus(const String &);
  bool LogFileStart(bool);
  bool LogFileWriteHeader(const String &);
  void LogFileFlush();
  void LogCardLost();
  void LogCardUnmount();
  void LogCardFull();
  void LogCardRetry();
  void SetSdLogQuiet(bool);

public:
  Logs();
  Logs(String, uint16_t);
  Logs(String, String);
  Logs(LogLevel_enum, String, String);
  Logs(String, String, uint16_t);
  Logs(LogLevel_enum, String, String, uint16_t);
  ~Logs(){};

  void Init();
  bool LoggerStart();
  void LoggerProcess();
  void LoggerFlush(uint32_t);
  void LogOpenFile();
  void LogCloseFile();
  void LogCheckOpenedFile();
  void AddEvent(LogLevel_enum, String, bool = true, bool = true);
  void AddEvent(LogLevel_enum, const __FlashStringHelper*, String, bool = true, bool = true);
  void SetLogLevel(LogLevel_enum);
  void SetFileName(String);
  void SetFilePath(String);
  void SetFileMaxSize(uint16_t);
  void SetNtpTimeSynced(bool);

  String GetFileName();
  String GetFilePath();
  LogLevel_enum GetLogLevel();
  bool GetNtpTimeSynced();
  void CheckMaxLogFileSize();
  void CheckCardSpace();
  bool GetLogFileOpened();

  String GetSystemTime();
  String MaskSensitiveData(const String &);
};

extern Logs SystemLog;  ///< log object

/* EOF */