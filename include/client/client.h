#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "client_utils.h"
#include "communication_api.h"

// Max input allow for stdin
#define STDIN_MAX_SIZE 256

/**
 * @brief Input read results
 * 
 */
typedef enum
{
    INP_NULL = -3,      // Null input
    INP_TO_LONG = -2,   // Input too long
    INP_END = -1,       // End
    INP_EMPTY_LINE = 0, // Empty line
    INP_READ = 1        // Input read
} fp_input_result;

/**
 * @brief Client representation data
 * 
 */
struct 
{
    char *unix_socket_path;  // Socket path
    int unix_socket_fd;      // Socket file descriptor
    client_type type;   // Client type
} client;

/**
 * @brief Signal handler
 * 
 * @param sig Signal number
 * @param info Signal info
 * @param context Context
 */
void signal_handler(int sig, siginfo_t *info, void* context);

/**
 * @brief Initialize signal handler
 * 
 */
void signal_handler_init(void);

/**
 * @brief Get input from file pointer
 * 
 * @param buffer Buffer to store input
 * @param buffer_size Buffer size
 * @param fp File pointer
 * @return fp_input_result Input read result
 */
fp_input_result get_input(char* buffer, size_t buffer_size, FILE* fp);

/**
 * @brief Interaction with server journalctl with stdin inputs
 * 
 */
void journalctl(void);

/**
 * @brief Obtain server system info
 * 
 */
void system_info(void);

/**
 * @brief Connect to UNIX socket
 * 
 * @param socket_path Socket path
 * @return error_code Error code
 */
error_code connect_unix_sockect(const char *socket_path);

/**
 * @brief Connect to IPV4 socket
 * 
 * @param socket_ipv4 Socket ipv4
 * @param socket_port Socket port
 * @return error_code Error code
*/
error_code connect_ipv4_sockect(const char *socket_ipv4, const uint16_t socket_port);

/**
 * @brief Connect to IPV6 socket
 * 
 * @param socket_ipv6 Socket ipv6
 * @param socket_port Socket port
 * @return error_code Error code
*/
error_code connect_ipv6_sockect(const char *socket_ipv6, const uint16_t socket_port);

/**
 * @brief Initialize client
 * 
 * @param type Client type
 * @param sock_type Socket type
 * @param arg Socket path or socket ipv4 or socket ipv6
 */
void init(client_type clie_type, sockect_type sock_type, const char* arg);

/**
 * @brief End client
 * 
 */
void end(void);

#endif //__CLIENT_H__