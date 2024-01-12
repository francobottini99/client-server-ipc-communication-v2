#ifndef __SERVER_H__
#define __SERVER_H__

#include "server_threads_handle.h"
#include "server_utils.h"
#include "communication_api.h"

/**
 * @brief Server representation data
 * 
 */
struct 
{
    char *unix_socket_path; // UNIX socket path
    int unix_socket_fd;     // UNIX socket file descriptor
    int ipv4_socket_fd;     // IPV4 socket file descriptor
    int ipv6_socket_fd;     // IPV6 socket file descriptor
} server;

// Flag to indicate if server is finished
extern volatile sig_atomic_t finished;

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
 * @brief Handle request of clients type A
 * 
 * @param client_fd Client file descriptor
 */
void client_a_handle(int client_fd);

/**
 * @brief Handle request of clients type B
 * 
 * @param client_fd Client file descriptor
 */
void client_b_handle(int client_fd);

/**
 * @brief Handle request of clients type C
 * 
 * @param client_fd Client file descriptor
 */
void client_c_handle(int client_fd);

/**
 * @brief Create and asign new handle thread for a new client
 * 
 * @param args Client file descriptor
 */
void *connection_handler(void *args);

/**
 * @brief Start new connection with client
 * 
 * @param client_fd Client file descriptor
 * @return int Connection result
 */
int connection_start(int client_fd);

/**
 * @brief End connection with client
 * 
 * @param client_fd Client file descriptor
 * @param type Client type
 */
void connection_end(int *client_fd, client_type type);

/**
 * @brief Create UNIX server socket
 * 
 * @param socket_path Socket path
 * @return error_code Error code
 */
error_code create_unix_socket(const char *socket_path);

/**
 * @brief Create IPV4 server socket
 * 
 * @param socket_port Socket port
 * @return error_code Error code
 */
error_code create_ipv4_socket(const uint16_t socket_port);

/**
 * @brief Create IPV6 server socket
 * 
 * @param socket_port Socket port
 * @return error_code Error code
 */
error_code create_ipv6_socket(const uint16_t socket_port);

/**
 * @brief Accept new client connection
 * 
 * @param client_fd Client file descriptor
 * @return error_code Error code
 */
error_code accept_connection(int *client_fd);

/**
 * @brief Initialize server
 * 
 */
void init(void);

/**
 * @brief End server
 * 
 */
void end(void);

#endif // __SERVER_H__