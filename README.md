# Client-Server Sockets

This project implements an inter-process communication system using sockets in a client-server architecture.

### Author:
- **Franco Nicolas Bottini**

## Compilation Instructions

To compile the project, after cloning the repository, generate the Makefile using the CMake script and run:

```bash
$ git clone https://github.com/francobottini99/LINUXCLIENTSERVER2-2023.git
$ cd LINUXCLIENTSERVER2-2023
$ cmake .
$ make
```

This will produce two executables located in the `/bin` directory: `client` and `server`.

> **Note**: The `ZLIB` package must be installed on your system to compile the project.

## Client

The `client` binary creates processes that communicate with the server through sockets. Clients can connect to the server using three different types of sockets: *UNIX* (0), *IPV4* (1), and *IPV6* (2). Additionally, there are three types of clients, each differing in the type of task they request from the server:

- **CLIENT_A** (0): Prompts the user to input a command via the console, which is then sent to the server to interact with `journalctl` and display the result. This repeats until either the client or server instance ends.

- **CLIENT_B** (1): Prompts the user to input a command via the console, which is sent to the server to interact with `journalctl`. The result is compressed and stored in the `/data` directory as `client_b_result_[yyyy]_[mm]_[dd]_[HH]_[mm]_[ss]`. This repeats until either the client or server instance ends.

- **CLIENT_C** (2): Requests a system data report from the server and prints it to the console once received. This client runs only once and then terminates.

### Usage Examples

```bash
$ ./bin/Client 0 0          # Runs CLIENT_A connecting via UNIX socket
$ ./bin/Client 0 1 [IPV4]   # Runs CLIENT_A connecting via IPV4 socket
$ ./bin/Client 0 2 [IPV6]   # Runs CLIENT_A connecting via IPV6 socket

$ ./bin/Client 1 0          # Runs CLIENT_B connecting via UNIX socket
$ ./bin/Client 1 1 [IPV4]   # Runs CLIENT_B connecting via IPV4 socket
$ ./bin/Client 1 2 [IPV6]   # Runs CLIENT_B connecting via IPV6 socket

$ ./bin/Client 2 0          # Runs CLIENT_C connecting via UNIX socket
$ ./bin/Client 2 1 [IPV4]   # Runs CLIENT_C connecting via IPV4 socket
$ ./bin/Client 2 2 [IPV6]   # Runs CLIENT_C connecting via IPV6 socket
```

When using IPV4 and IPV6 sockets, specify the server's IP address as the third argument to establish the connection. You can run multiple client processes simultaneously.

## Server

The `server` binary creates the sockets to which clients connect and handles their requests. To run the server, use:

```bash
$ ./bin/server  # Starts the server
```

Only one server process can run on the system at a time.

## Functionality

When the server starts, it creates a **UNIX** socket and waits for client connections. Upon connection, the client sends a message indicating its type, and the server spawns a child thread to handle the client's requests. The main thread remains available to accept additional clients. The child thread continues to process the client's requests until the client disconnects, at which point the thread terminates. The server can handle multiple clients simultaneously.

### Communication Protocol

Client and server communication uses a **communication API** that transmits data via a buffer and follows this protocol:

1. **Data Fragmentation & Encapsulation**: 
   - Data to be transmitted is fragmented into `N` fixed-size packets. Each packet is encapsulated in a structure that includes an `array` named `data`, headers such as a checksum, total size, fragment size, and a flag indicating if it's the last fragment. A pointer to the next fragment in the list is also included.
   - Example structure:

```c
/**
 * @brief Data fragment
 *
 */
typedef struct fragments 
{
    int checksum;                   // Checksum of data
    size_t total_size;              // Total size of all fragments
    size_t content_size;            // Total size of this fragment
    uint8_t last;                   // Last fragment flag
    char data[DATA_FRAGMENT_SIZE];  // Data of fragment
    struct fragments* next;         // Next fragment
} fragments;
```

2. **JSON Formatting**: 
   - Each packet is formatted into a **JSON** string. For instance, transmitting `-n 10` generates:

```json
{
    "checksum": 1982818317,
    "total_size": 6,
    "content_size": 6,
    "last": 1,
    "data": [45,110,32,49,48,0]
}
```

3. **Data Transmission**: 
   - The JSON string is sent via the socket. The receiver deserializes the string, reconstructs the packet, and verifies the checksum. If valid, the packet is stored; otherwise, it is discarded and a retransmission request is sent. This continues until all fragments are received.

4. **Defragmentation**: 
   - Once all fragments are received and verified, they are reassembled to recreate the original data.
