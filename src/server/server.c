#include "server.h"

const char* client_type_to_string[] = {"A", "B", "C"};

volatile sig_atomic_t finished = 0;

// Mutex for concurrent access to handle thread list
pthread_mutex_t mutex;

void signal_handler(int sig, siginfo_t *info, void* context)
{
    UNUSED(info);
    UNUSED(context);

    if(sig == SIGTERM || sig == SIGINT || sig == SIGHUP || sig == SIGPIPE)
        finished = 1;
}

void signal_handler_init(void)
{
    struct sigaction sa = 
    {
        .sa_sigaction = signal_handler,
        .sa_flags = SA_SIGINFO
    };
    sigemptyset(&sa.sa_mask);

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
}

void client_a_handle(int client_fd)
{
    char* data = NULL;
    char* result = NULL;

    while (1)
    {
        size_t bytes_received;
        size_t bytes_sent;

        error_code in = receive_data(client_fd, &data, &bytes_received, &finished);
    
        if (in == ERROR_SOCKET_DISCONNECT || in == END_SIGNAL) 
            break;
        else if (in == SUCCESS)
        {
            printf(KYEL"\nRecibe [%ld B] Client %s (FD: %d)\n"KDEF, bytes_received, client_type_to_string[CLIENT_TYPE_A], client_fd);

            result = journalctl_execute(data, client_fd);

            bytes_sent = strlen(result) + 1;
            
            if(send_data(client_fd, result, bytes_sent, &finished) == SUCCESS)
                printf(KCYN"\nSend [%ld B] Client %s (FD: %d)\n"KDEF, bytes_sent, client_type_to_string[CLIENT_TYPE_A], client_fd);
            else
                fprintf(stderr, KRED"\nError sending data to client %s (FD: %d) \n"KDEF, client_type_to_string[CLIENT_TYPE_A], client_fd);

            free(result);
        }
    }
}

void client_b_handle(int client_fd)
{
    char* data = NULL;
    char* result = NULL;

    while (1)
    {
        size_t bytes_received;
        size_t bytes_sent;

        error_code in = receive_data(client_fd, &data, &bytes_received, &finished);
    
        if (in == ERROR_SOCKET_DISCONNECT || in == END_SIGNAL) 
            break;
        else if (in == SUCCESS)
        {
            FILE *fp;
            char filename[256];

            printf(KYEL"\nRecibe [%ld B] Client %s (FD: %d)\n"KDEF, bytes_received, client_type_to_string[CLIENT_TYPE_B], client_fd);

            sprintf(filename, "%s_%d.txt.gz", COMPRESS_TMP_OUTPUT, client_fd);

            result = journalctl_execute(data, client_fd);

            compress_and_save_data(result, filename);

            free(result);

            fp = fopen(filename, "rb");

            fseek(fp, 0L, SEEK_END);
            bytes_sent = (size_t) ftell(fp);
            fseek(fp, 0L, SEEK_SET);

            result = calloc(bytes_sent, sizeof(char));

            fread(result, sizeof(char), bytes_sent, fp);

            if(send_data(client_fd, result, bytes_sent, &finished) == SUCCESS)
                printf(KCYN"\nSend [%ld B] Client %s (FD: %d)\n"KDEF, bytes_sent, client_type_to_string[CLIENT_TYPE_B], client_fd);
            else
                fprintf(stderr, KRED"\nError sending data to client %s (FD: %d) \n"KDEF, client_type_to_string[CLIENT_TYPE_B], client_fd);
            
            free(result);
            fclose(fp);
            remove(filename);
        }
    }
}

void client_c_handle(int client_fd)
{
    char* result = get_system_info();

    size_t bytes_sent = strlen(result) + 1;

    if(send_data(client_fd, result, bytes_sent, &finished) == SUCCESS)
        printf(KCYN"\nSend [%ld B] Client %s (FD: %d)\n"KDEF, bytes_sent, client_type_to_string[CLIENT_TYPE_C], client_fd);
    else
        fprintf(stderr, KRED"\nError sending data to client %s (FD: %d) \n"KDEF, client_type_to_string[CLIENT_TYPE_C], client_fd);

    free(result);
}

void *connection_handler(void *args) 
{    
    int *client_fd = (int*)args; 

    int type = connection_start(*client_fd);

    if (type < 0)
    {
        close(*client_fd);
        free(args);

        return NULL;
    }

    switch ((client_type)type)
    {
    case CLIENT_TYPE_A:
        client_a_handle(*client_fd);
        break;
    
    case CLIENT_TYPE_B:
        client_b_handle(*client_fd);
        break;

    case CLIENT_TYPE_C:
        client_c_handle(*client_fd);
        break;
    
    default:
        break;
    }

    connection_end(client_fd, type);

    return NULL;
}

int connection_start(int client_fd)
{
    char* buffer = NULL;
    client_type type;
    error_code result = receive_data(client_fd, &buffer, NULL, &finished);
    
    if (result == SUCCESS) 
        type = atoi(buffer);
    else
    {
        fprintf(stderr, KRED"Fail client connection\n"KDEF);
        return -1;
    }

    printf(KGRN"\nClient %s (FD: %d) connect !\n"KDEF, client_type_to_string[type], client_fd);

    return type;
}

void connection_end(int *client_fd, client_type type)
{
    printf(KRED"\nClient %s (FD: %d) disconnect !\n"KDEF, client_type_to_string[type], *client_fd);
    
    close(*client_fd);
    free(client_fd);

    if(!finished)
    {
        pthread_mutex_lock(&mutex);
        handler_destroy(pthread_self());
        pthread_mutex_unlock(&mutex);
    }
}

error_code create_unix_socket(const char *socket_path)
{
    struct sockaddr_un server_address;

    server.unix_socket_path = strdup(socket_path);
    server.unix_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server.unix_socket_fd < 0) 
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sun_family = AF_UNIX;

    strcpy(server_address.sun_path, server.unix_socket_path);

    if (bind(server.unix_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("bind() failed");
        return ERROR_SOCKET_BIND;
    }

    if (listen(server.unix_socket_fd, 1) < 0) 
    {
        perror("listen() failed");
        return ERROR_SOCKET_LISTEN;
    }

    return SUCCESS;
}

error_code create_ipv4_socket(const uint16_t socket_port)
{
    struct sockaddr_in server_address;

    server.ipv4_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server.ipv4_socket_fd < 0) 
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(socket_port);

    if (bind(server.ipv4_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("bind() failed");
        return ERROR_SOCKET_BIND;
    }

    if (listen(server.ipv4_socket_fd, 1) < 0) 
    {
        perror("listen() failed");
        return ERROR_SOCKET_LISTEN;
    }

    return SUCCESS;
}

error_code create_ipv6_socket(const uint16_t socket_port)
{
    struct sockaddr_in6 server_address;

    server.ipv6_socket_fd = socket(AF_INET6, SOCK_STREAM, 0);

    if (server.ipv6_socket_fd < 0) 
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(socket_port);

    if (bind(server.ipv6_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("bind() failed");
        return ERROR_SOCKET_BIND;
    }

    if (listen(server.ipv6_socket_fd, 1) < 0) 
    {
        perror("listen() failed");
        return ERROR_SOCKET_LISTEN;
    }

    return SUCCESS;
}


error_code accept_connection(int *client_fd)
{
    *client_fd = -1;

    while (*client_fd < 0 && !finished)
    {
        struct timeval timeout = {0, 100000};
        fd_set socket_set;
        int max_socket;

        FD_ZERO(&socket_set);

        FD_SET(server.ipv4_socket_fd, &socket_set);
        FD_SET(server.ipv6_socket_fd, &socket_set);
        FD_SET(server.unix_socket_fd, &socket_set);

        max_socket = (server.ipv4_socket_fd > server.ipv6_socket_fd) ? server.ipv4_socket_fd : server.ipv6_socket_fd;
        max_socket = (server.unix_socket_fd > max_socket) ? server.unix_socket_fd : max_socket;

        if (select(max_socket + 1, &socket_set, NULL, NULL, &timeout) > 0)
        {
            if (FD_ISSET(server.ipv4_socket_fd, &socket_set))
                *client_fd = accept(server.ipv4_socket_fd, NULL, NULL);

            if (FD_ISSET(server.ipv6_socket_fd, &socket_set))
                *client_fd = accept(server.ipv6_socket_fd, NULL, NULL);

            if (FD_ISSET(server.unix_socket_fd, &socket_set))
                *client_fd = accept(server.unix_socket_fd, NULL, NULL);
        }
    }

    if(finished)
        return END_SIGNAL;

    return SUCCESS;
}

void init(void)
{
    struct stat st = {0};

    if (stat("tmp", &st) == -1) 
    {
        if (mkdir("tmp", 0777) != 0) 
        {
            fprintf(stderr, "Error: could not create directory 'tmp'\n");
            exit(EXIT_FAILURE);
        }
    }

    error_code result = create_unix_socket(UNIX_SOCKET_PATH);

    if (result != SUCCESS) 
    {
        fprintf(stderr, "Server creation failed with error code %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = create_ipv4_socket(IPV4_SOCKET_PORT);

    if (result != SUCCESS)
    {
        fprintf(stderr, "Server creation failed with error code %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = create_ipv6_socket(IPV6_SOCKET_PORT);

    if (result != SUCCESS)
    {
        fprintf(stderr, "Server creation failed with error code %d\n", result);
        exit(EXIT_FAILURE);
    }

    signal_handler_init();

    pthread_mutex_init(&mutex, NULL);

    printf(KBLU"\nServer start (FD: %d) !\n"KDEF, server.unix_socket_fd);
}

void end(void)
{
    handler_wait_all();
    handler_destroy_all();

    close(server.unix_socket_fd);
    close(server.ipv4_socket_fd);
    close(server.ipv6_socket_fd);

    unlink(server.unix_socket_path);
    
    free(server.unix_socket_path);

    pthread_mutex_destroy(&mutex);

    rmdir("tmp");

    printf(KBLU"\nServer stop (FD: %d) !\n"KDEF, server.unix_socket_fd);

    exit(EXIT_SUCCESS);
}

int main(void)
{
    init();

    int *client_fd = calloc(1, sizeof(int));   
    
    error_code result;

    do 
    {
        result = accept_connection(client_fd);

        if(result == SUCCESS)
        {
            pthread_t* tid = handler_create();

            if (pthread_create(tid, NULL, connection_handler, (void *)client_fd) != 0)
            {
                perror("pthread_create() failed");
                close(*client_fd);
                free(client_fd);
            }

            client_fd = calloc(1, sizeof(int));
        }
    } while(result != END_SIGNAL);

    end();

    return EXIT_SUCCESS;
}