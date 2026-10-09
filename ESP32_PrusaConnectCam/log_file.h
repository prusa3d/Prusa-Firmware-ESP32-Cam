/**
   @file log_file.h

   @brief Pure logic for the logs (numbering and cleanup of the log files on the micro-SD card, message length, masking of sensitive data)

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#pragma once

#include <stdint.h>
#include <stddef.h>

#define LOG_FILE_INDEX_DIGITS       4                       ///< number of digits in the log file index
#define LOG_FILE_INDEX_MAX          9999                    ///< maximum log file index
#define LOG_FILE_TRUNCATE_TAIL      "...\n"                 ///< tail appended to a truncated message

int32_t LogFile_ParseIndex(const char *, const char *, const char *);
int32_t LogFile_NextIndex(const int32_t *, size_t);
size_t LogFile_SelectToDelete(int32_t *, size_t, size_t);
void LogFile_FormatName(char *, size_t, const char *, uint16_t, const char *);
size_t LogFile_MsgKeepLength(size_t, size_t);
bool LogFile_IsCardFull(int32_t, size_t, int);
void LogFile_MaskSecret(char *);

/* EOF */
