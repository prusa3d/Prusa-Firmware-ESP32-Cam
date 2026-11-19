/**
   @file log_file.cpp

   @brief Pure logic for the logs (numbering and cleanup of the log files on the micro-SD card, message length, masking of sensitive data)

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "log_file.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/**
   @brief Function for parse the log file index from the file name
   @param const char * - file name without path
   @param const char * - file name prefix
   @param const char * - file name suffix
   @return int32_t - index 0-9999, or -1 when the name is not prefix + 4 digits + suffix
*/
int32_t LogFile_ParseIndex(const char *i_name, const char *i_prefix, const char *i_suffix) {
  size_t name_len = strlen(i_name);
  size_t prefix_len = strlen(i_prefix);
  size_t suffix_len = strlen(i_suffix);

  if (name_len != (prefix_len + LOG_FILE_INDEX_DIGITS + suffix_len)) {
    return -1;
  }

  if (0 != strncmp(i_name, i_prefix, prefix_len)) {
    return -1;
  }

  if (0 != strcmp(i_name + prefix_len + LOG_FILE_INDEX_DIGITS, i_suffix)) {
    return -1;
  }

  int32_t index = 0;
  for (size_t i = 0; i < LOG_FILE_INDEX_DIGITS; i++) {
    char c = i_name[prefix_len + i];
    if ((c < '0') || (c > '9')) {
      return -1;
    }
    index = (index * 10) + (c - '0');
  }

  return index;
}

/**
   @brief Function for get the next log file index
   @param const int32_t * - indexes of the existing log files
   @param size_t - count of the indexes
   @return int32_t - highest index + 1, 0 for an empty list, -1 when the highest index is 9999
*/
int32_t LogFile_NextIndex(const int32_t *i_indexes, size_t i_count) {
  int32_t max_index = -1;

  for (size_t i = 0; i < i_count; i++) {
    if (i_indexes[i] > max_index) {
      max_index = i_indexes[i];
    }
  }

  if (max_index >= LOG_FILE_INDEX_MAX) {
    return -1;
  }

  return max_index + 1;
}

/**
   @brief Compare function for qsort, ascending order
   @param const void * - first item
   @param const void * - second item
   @return int - <0, 0, >0
*/
static int LogFile_CompareIndex(const void *i_a, const void *i_b) {
  int32_t a = *(const int32_t *)i_a;
  int32_t b = *(const int32_t *)i_b;

  return (a > b) - (a < b);
}

/**
   @brief Function for select the log files to delete before a new log file is created
   @param int32_t * - indexes of the existing log files, sorted ascending by this function
   @param size_t - count of the indexes
   @param size_t - maximum count of the log files including the new one
   @return size_t - count of the first items to delete
*/
size_t LogFile_SelectToDelete(int32_t *io_indexes, size_t i_count, size_t i_max_count) {
  if (i_count > 1) {
    qsort(io_indexes, i_count, sizeof(int32_t), LogFile_CompareIndex);
  }

  if ((i_count + 1) <= i_max_count) {
    return 0;
  }

  size_t delete_count = i_count + 1 - i_max_count;
  if (delete_count > i_count) {
    delete_count = i_count;
  }

  return delete_count;
}

/**
   @brief Function for format the log file name
   @param char * - output buffer
   @param size_t - output buffer size
   @param const char * - file name prefix
   @param uint16_t - file index
   @param const char * - file name suffix
   @return none
*/
void LogFile_FormatName(char *o_buf, size_t i_size, const char *i_prefix, uint16_t i_index, const char *i_suffix) {
  snprintf(o_buf, i_size, "%s%04u%s", i_prefix, (unsigned int)i_index, i_suffix);
}

/**
   @brief Function for get the length of the message to keep
   @param size_t - message length
   @param size_t - maximum message length
   @return size_t - message length, or maximum length - 4 when the message is too long. The caller appends "...\n"
*/
size_t LogFile_MsgKeepLength(size_t i_len, size_t i_max_len) {
  if (i_len <= i_max_len) {
    return i_len;
  }

  return i_max_len - (sizeof(LOG_FILE_TRUNCATE_TAIL) - 1);
}

/**
   @brief Function for check, if the failed write to the log file means a full card
   @param int32_t - return value of write()
   @param size_t - length of the data for write
   @param int - errno after write(), the caller sets errno to 0 before write()
   @return bool - true = card is full (ENOSPC, or short write without error), false = card removed or write failed
*/
bool LogFile_IsCardFull(int32_t i_ret, size_t i_len, int i_errno) {
  if (ENOSPC == i_errno) {
    return true;
  }

  /* short write with error (EIO) means removed card, VFS returns the written bytes and sets errno */
  return ((i_ret >= 0) && ((size_t)i_ret < i_len) && (0 == i_errno));
}

/**
   @brief Function for mask sensitive data (token, password) in the log. All characters are replaced with '*', the length is kept
   @param char * - sensitive data, changed in place
   @return none
*/
void LogFile_MaskSecret(char *io_secret) {
  if (NULL == io_secret) {
    return;
  }

  for (size_t i = 0; '\0' != io_secret[i]; i++) {
    io_secret[i] = '*';
  }
}

/* EOF */
