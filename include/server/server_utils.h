#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include "common.h"

// Joutnalctl temporary file path to save error
#define JOURNAL_TMP_ERROR "tmp/err"

// Joutnalctl temporary file path to save output
#define JOURNAL_TMP_OUTPUT "tmp/out"

// Joutnalctl temporary file path to save compression result
#define COMPRESS_TMP_OUTPUT "tmp/compress"

/**
 * @brief Get system information
 * 
 * @return char* System information
 */
char* get_system_info(void);

/**
 * @brief Compress and save data
 * 
 * @param data Data to compress
 * @param filename File name
 * @return int Compression result. 0 if success, -1 if error
 */
int compress_and_save_data(const char *data, const char* filename);

/**
 * @brief Execute journalctl command
 * 
 * @param command Command to execute
 * @param client_fd Executor client file descriptor
 * @return char* Result
 */
char* journalctl_execute(const char* command, int client_fd);

/**
 * @brief Read file
 * 
 * @param file File path
 * @return char* File content
 */
char* read_file(const char* file);

#endif // __SERVER_UTILS_H__