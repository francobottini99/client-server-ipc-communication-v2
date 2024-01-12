#ifndef __COMMUNICATION_API_H__
#define __COMMUNICATION_API_H__

#include "common.h"

// Max fragment size
#define FRAGMENT_SIZE 4096

// Max header size
#define HEADER_SIZE 150

/**
 * @brief Receive data from socket
 * 
 * @param sockect_fd Socket file descriptor
 * @param data Data to send
 * @param data_size Data size
 * @param end_flag End test flag
 * @return error_code Error code
 */
error_code receive_data(int sockect_fd, char **buffer, size_t* bytes_received, volatile sig_atomic_t *end_flag);

/**
 * @brief Send data to socket
 * 
 * @param sockect_fd Socket file descriptor
 * @param data Data to receive
 * @param data_size Data size
 * @param end_flag End test flag
 * @return error_code Error code
 */
error_code send_data(int sockect_fd, char *data, size_t data_size, volatile sig_atomic_t *end_flag);

#endif //__COMMUNICATION_API_H__