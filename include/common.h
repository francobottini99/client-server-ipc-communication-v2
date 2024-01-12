#ifndef __COMMON_H__
#define __COMMON_H__

#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <zlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Define colors codes for terminal
#ifndef TERMINAL_TEXT_COLORS
#define TERMINAL_TEXT_COLORS
    #define KDEF  "\x1B[0m"     // Default
    #define KRED  "\x1B[31m"    // Red
    #define KGRN  "\x1B[32m"    // Green
    #define KYEL  "\x1B[33m"    // Yellow
    #define KBLU  "\x1B[34m"    // Blue
    #define KMAG  "\x1B[35m"    // Magenta
    #define KCYN  "\x1B[36m"    // Cyan
    #define KWHT  "\x1B[37m"    // White
#endif

// Define ASCII characters for general use
#ifndef ASCII_CHARACTERS
#define ASCII_CHARACTERS
    #define ASCII_END_OF_STRING 0       // '\0'
    #define ASCII_MONEY_SIGN    36      // '$'
    #define ASCII_MIDDLE_DASH   45      // '-'
    #define ASCII_SPACE         32      // ' '
    #define ASCII_LINE_BREAK    10      // '\n'
    #define ASCII_AMPERSAND     38      // '&' 
    #define ASCII_PLECA         124     // '|' 
    #define ASCII_PERCENT       37      // '%'  
    #define ASCII_GREATER_THAN  62      // '>'  
    #define ASCII_LESS_THAN     60      // '<'  
#endif

// Define macro to unused variables
#define UNUSED(x) (void)(x)

// Define unix socket path
#define UNIX_SOCKET_PATH "tmp/sckt"

// Define IPV4 and IPV6 sockets ports
#define IPV4_SOCKET_PORT 5000
#define IPV6_SOCKET_PORT 5001

/**
 * @brief Error codes for communication functions
 * 
 */
typedef enum 
{
    SUCCESS = 0,                    // Success
    ERROR_SOCKET_CREATION = -1,     // Socket creation failed
    ERROR_SOCKET_BIND = -2,         // Socket bind failed
    ERROR_SOCKET_LISTEN = -3,       // Socket listen failed
    ERROR_SOCKET_ACCEPT = -4,       // Socket accept failed
    ERROR_SOCKET_CONNECTION = -5,   // Socket connection failed
    ERROR_SOCKET_SEND = -6,         // Socket send failed
    ERROR_SOCKET_RECEIVE = -7,      // Socket receive failed
    ERROR_THREAD_FAILED = -8,       // Thread creation failed
    ERROR_SOCKET_DISCONNECT = -9,   // Socket lost connection
    END_SIGNAL = -10                // End signal received
} error_code;

/**
 * @brief Client types
 * 
 */
typedef enum
{
    CLIENT_TYPE_A,  // Client A
    CLIENT_TYPE_B,  // Client B
    CLIENT_TYPE_C   // Client C
} client_type;

/**
 * @brief Sockects types
 * 
*/
typedef enum
{
    SOCKECT_TYPE_UNIX,  // UNIX socket
    SOCKECT_TYPE_IPV4,  // IPV4 socket
    SOCKECT_TYPE_IPV6   // IPV6 socket
} sockect_type;

// Aux array to convert client type to string
extern const char* client_type_to_string[];

#endif // __COMMON_H__