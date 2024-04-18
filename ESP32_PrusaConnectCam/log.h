/**
   @file log.h

   @brief log library

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _LOG_H_
#define _LOG_H_

#include "Arduino.h"

#include "mcu_cfg.h"
#include "var.h"
#include "micro_sd.h"

enum LogLevel_enum {
  LogLevel_Error = 0,       ///< Error
  LogLevel_Warning = 1,     ///< Warning
  LogLevel_Info = 2,        ///< Info
  LogLevel_Verbose = 3      ///< Verbose
};

class Logs : public MicroSd {
private:
  LogLevel_enum LogLevel;   ///< LogLevel
  String FileName;          ///< log File name
  String FilePath;          ///< log file patch
  uint16_t FileMaxSize;     ///< log file max size
  bool NtpTimeSynced;       ///< status NTP time sync

public:
  Logs();
  Logs(String, String);
  Logs(LogLevel_enum, String, String);
  Logs(String, String, uint16_t);
  Logs(LogLevel_enum, String, String, uint16_t);
  ~Logs(){};

  void Init();
  void AddEvent(LogLevel_enum, String, bool = true, bool = true);
  void SetLogLevel(LogLevel_enum);
  void SetFileName(String);
  void SetFilePath(String);
  void SetFileMaxSize(uint16_t);
  void SetNtpTimeSynced(bool);

  String GetFileName();
  String GetFilePath();
  LogLevel_enum GetLogLevel();
  bool GetNtpTimeSynced();

protected:
  String GetSystemTime();
};

extern Logs SystemLog;  ///< log object

#endif

/* EOF */