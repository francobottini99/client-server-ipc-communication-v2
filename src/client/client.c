#include "client.h"

const char* client_type_to_string[] = {"A", "B", "C"};

void signal_handler(int sig, siginfo_t *info, void* context)
{
    UNUSED(info);
    UNUSED(context);

    if(sig == SIGTERM || sig == SIGINT || sig == SIGHUP || sig == SIGPIPE)
        end();
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

fp_input_result get_input(char* buffer, size_t buffer_size, FILE* fp)
{
    fd_set read_fds;
    
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(client.unix_socket_fd, &read_fds);

    while (1)
    {
        if(select(client.unix_socket_fd + 1, &read_fds, NULL, NULL, NULL) > 0)
        {
            if (FD_ISSET(client.unix_socket_fd, &read_fds))
                end();
            else
            {
                if(fgets(buffer, (int)buffer_size + 1, fp) != NULL)
                    break;
            }
        }
    } 
    
    char* endOfBuffer = buffer + strlen(buffer) - 1;

    if((*endOfBuffer != ASCII_LINE_BREAK && fp == stdin) || (*endOfBuffer != ASCII_LINE_BREAK && *endOfBuffer != EOF && strlen(buffer) - 1 >= buffer_size && fp != stdin))
    {
        while(getchar() != ASCII_LINE_BREAK);
        fprintf(stderr, KRED"\nExceeded max input length !\n\n"KDEF);
        return INP_TO_LONG;
    }

    if(trim_white_space(buffer) == NULL)
        return INP_EMPTY_LINE;

    return INP_READ;
}

void journalctl(void)
{
    char* response = NULL;
    char buffer[STDIN_MAX_SIZE];

    fp_input_result read_input = INP_NULL;

    fprintf(stdout,"\n");

    while (1)
    {   
        memset(buffer, 0, STDIN_MAX_SIZE);

        fprintf(stdout, KGRN"Client-%s~$ journalctl "KDEF, client_type_to_string[client.type]); 
        fflush(stdout);
        
        read_input = get_input((char*) buffer, STDIN_MAX_SIZE, stdin);

        if(read_input == INP_TO_LONG)
            continue;
            
        if(read_input == INP_EMPTY_LINE)
            strcpy(buffer, " ");
        
        error_code result = send_data(client.unix_socket_fd, buffer, strlen(buffer) + 1, NULL);

        if (result != SUCCESS)
            fprintf(stderr, KRED"\nError sending data to server\n"KDEF);
        else
        {
            size_t bytes_receive;

            result = receive_data(client.unix_socket_fd, &response, &bytes_receive, NULL);

            if (result == SUCCESS)
            {
                if (client.type == CLIENT_TYPE_B)
                {
                    char filename[256];
                    FILE *fp;

                    time_t now;
                    time(&now);
                    struct tm *local_time = localtime(&now);

                    strftime(filename, 256, "data/client_b_result_%Y-%m-%d_%H-%M-%S.txt.gz", local_time);

                    fp = fopen(filename, "wb");

                    fwrite(response, sizeof(char), bytes_receive, fp);

                    fclose(fp);

                    printf(KCYN"\nRecibe and save [%ld B] compress file from server\n\n"KDEF, bytes_receive);
                }
                else
                {
                    printf(KYEL"\n%s\n"KDEF, response);
                    printf(KCYN"\nRecibe [%ld B] from server\n\n"KDEF, bytes_receive);
                }
            }
            else
                fprintf(stderr, KRED"\nError receiving data from server\n"KDEF);

            free(response);
        }
    }
}

void system_info(void)
{
    size_t bytes_receive;
    char* response = NULL;

    error_code result = receive_data(client.unix_socket_fd, &response, &bytes_receive, NULL);

    if(result == SUCCESS)
    {
        printf(KYEL"\n%s\n"KDEF, response);
        printf(KCYN"\nRecibe [%ld B] from server\n"KDEF, bytes_receive);
    }
    else
        fprintf(stderr, KRED"\nError receiving data from server\n"KDEF);

    free(response);
}

error_code connect_unix_sockect(const char *socket_path)
{
    struct sockaddr_un server_address;

    client.unix_socket_path = strdup(socket_path);
    client.unix_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (client.unix_socket_fd < 0) 
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    } 

    memset(&server_address, 0, sizeof(server_address));

    server_address.sun_family = AF_UNIX;

    strcpy(server_address.sun_path, client.unix_socket_path);

    if (connect(client.unix_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("connect() failed");
        return ERROR_SOCKET_CONNECTION;
    }

    return SUCCESS;
}

error_code connect_ipv4_sockect(const char *socket_ipv4, const uint16_t socket_port)
{
    struct sockaddr_in server_address;

    client.unix_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client.unix_socket_fd < 0)
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    } 

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(socket_port);

    if (inet_pton(AF_INET, socket_ipv4, &server_address.sin_addr) <= 0) 
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    }

    if (connect(client.unix_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("connect() failed");
        return ERROR_SOCKET_CONNECTION;
    }

    return SUCCESS;
}

error_code connect_ipv6_sockect(const char *socket_ipv6, const uint16_t socket_port)
{
    struct sockaddr_in6 server_address;

    client.unix_socket_fd = socket(AF_INET6, SOCK_STREAM, 0);

    if (client.unix_socket_fd < 0)
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    } 

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin6_family = AF_INET6;
    server_address.sin6_port = htons(socket_port);

    if (inet_pton(AF_INET6, socket_ipv6, &server_address.sin6_addr) <= 0) 
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    }

    if (connect(client.unix_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("connect() failed");
        return ERROR_SOCKET_CONNECTION;
    }

    return SUCCESS;
}

void init(client_type clie_type, sockect_type sock_type, const char* arg)
{
    struct stat st = {0};

    if (stat("data", &st) == -1) 
    {
        if (mkdir("data", 0777) != 0) 
        {
            fprintf(stderr, "Error: could not create directory 'data'\n");
            exit(EXIT_FAILURE);
        }
    }

    error_code result;
    
    switch (sock_type)
    {
    case SOCKECT_TYPE_UNIX:
        result = connect_unix_sockect(arg);
        break;

    case SOCKECT_TYPE_IPV4:
        result = connect_ipv4_sockect(arg, IPV4_SOCKET_PORT);
        break;

    case SOCKECT_TYPE_IPV6:
        result = connect_ipv6_sockect(arg, IPV6_SOCKET_PORT);
        break;

    default:
        fprintf(stderr, KRED"\nBad socket type select\n"KDEF);
        end();
        break;
    }

    if (result != SUCCESS)
    {
        fprintf(stderr, KRED"\nConnection to server failed with error code %d\n"KDEF, result);
        end();
    }

    client.type = clie_type;
    
    char aux[2];

    sprintf(aux, "%d", clie_type);

    result = send_data(client.unix_socket_fd, aux, strlen(aux) + 1, NULL);

    if (result != SUCCESS) 
    {
        fprintf(stderr, KRED"\nConnection to server failed with error code %d\n"KDEF, result);
        end();
    }

    signal_handler_init();
}

void end(void) 
{
    printf(KRED"\n\nClient end !\n"KDEF);

    close(client.unix_socket_fd);
    free(client.unix_socket_path);

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    if(argc > 4 || argc < 3)
    {
        fprintf(stderr, KRED"\nBad number of arguments !\n"KDEF);
        exit(EXIT_FAILURE);
    }

    int cli_type = atoi(argv[1]);

	if((*argv[1] != '0' && cli_type == 0) || cli_type < 0 || cli_type > 2)
	{
		fprintf(stderr, KRED"Bad argument client type!"KDEF);
		exit(EXIT_FAILURE);
	}

    int sock_type = atoi(argv[2]);

	if((*argv[2] != '0' && sock_type == 0) || sock_type < 0 || sock_type > 2)
	{
		fprintf(stderr, KRED"Bad argument sockect type!"KDEF);
		exit(EXIT_FAILURE);
	}

    if(sock_type == 0 && argc > 3)
    {
        fprintf(stderr, KRED"\nUnix sockect not required extra argument!\n"KDEF);
        exit(EXIT_FAILURE);
    }

    if(argc == 3)
        init((client_type)cli_type, (sockect_type)sock_type, UNIX_SOCKET_PATH);
    else
        init((client_type)cli_type, (sockect_type)sock_type, argv[3]);

    switch ((client_type)cli_type)
    {
    case CLIENT_TYPE_A:
    case CLIENT_TYPE_B:
        journalctl();
        break;
    
    case CLIENT_TYPE_C:
        system_info();
        break;

    default:
        break;
    }

    end();

    return EXIT_SUCCESS;
}