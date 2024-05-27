/**
   @file log.cpp

   @brief log library

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/
#include "log.h"

Logs SystemLog(LOGS_FILE_PATH, LOGS_FILE_NAME, LOGS_FILE_MAX_SIZE);

/**
   @info Constructor
   @param none
   @return none
*/
Logs::Logs() {
  FileName = "log.txt";
  FilePath = "/";
  LogLevel = LogLevel_Verbose;
  FileMaxSize = 1024;
  NtpTimeSynced = false;
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
  InitSdCard();

  if (true == GetCardDetectedStatus()) {
    /* check maximum log file size */
    CheckMaxLogFileSize();

    /* added first message to log file after start MCU */
    String msg = F("----------------------------------------------------------------\n");
    msg += F("Start MCU!\nSW Version: ");
    msg += String(SW_VERSION);
    msg += F(" ,Build: ");
    msg += String(SW_BUILD);
    msg += "\n";
    msg += F("Verbose mode: ");
    msg += (true == CONSOLE_VERBOSE_DEBUG) ? "true" : "false";
    msg += "\n";
    msg += F("Log level: ");
    msg += String(LogLevel);
    msg += "\n";
    AppendFile(SD_MMC, FilePath + FileName, msg);

  } else {
    Serial.println(F("Micro-SD card not found! Disable logs"));
  }
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
  if (LogLevel >= level) {
    String LogMsg = "";

    if (true == date) {
      LogMsg += GetSystemTime();
      LogMsg += " - ";
    }
    LogMsg += msg;
    if (true == newLine) {
      LogMsg += "\n";
    }

    AppendFile(SD_MMC, FilePath + FileName, LogMsg);
    Serial.print(LogMsg);
  }
#if (true == CONSOLE_VERBOSE_DEBUG)
  else {
    Serial.println(msg);
  }
#endif
}

 void Logs::AddEvent(LogLevel_enum level, const __FlashStringHelper *msg, String parameters, bool newLine, bool date) {
  if (LogLevel >= level) {
    String LogMsg = "";

    if (true == date) {
      LogMsg += GetSystemTime();
      LogMsg += " - ";
    }
    LogMsg += msg;
    LogMsg += parameters;
    if (true == newLine) {
      LogMsg += "\n";
    }

    AppendFile(SD_MMC, FilePath + FileName, LogMsg);
    Serial.print(LogMsg);
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
  AddEvent(LogLevel_Info, "System time: " + GetSystemTime());
}

/**
   @info Get file name
   @param none
   @return String - file name
*/
String Logs::GetFileName() {
  return FileName;
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
  uint32_t FileSize = GetFileSize(SD_MMC, FilePath + FileName);
  Serial.printf("Log file size: %d bytes\n", FileSize);
  if (FileSize >= LOGS_FILE_MAX_SIZE) {
    uint16_t file_count = FileCount(SD_MMC, FilePath, FileName);
    Serial.printf("Maximum log file size.\nFile count: %d\n", file_count);
    RenameFile(SD_MMC, FilePath + FileName, FilePath + FileName + String(file_count));
  }
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

/* EOF */