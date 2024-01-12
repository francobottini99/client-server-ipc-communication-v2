#include "communication_api.h"

// Data fragment size (without header) 
#define DATA_FRAGMENT_SIZE FRAGMENT_SIZE - HEADER_SIZE

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

/**
 * @brief Generate checksum
 *
 * @param data Data to generate checksum
 * @param length Data length
 * @return int Checksum
 */
int generate_checksum(void *data, size_t length) 
{
    const uint8_t* bytes = (const uint8_t*) data;

    uint32_t crc = 0xffffffff;

    for (size_t i = 0; i < length; ++i) 
    {
        crc ^= bytes[i];

        for (int j = 0; j < 8; ++j) 
        {
            uint32_t mask = -(crc & 1);

            crc = (crc >> 1) ^ (0xedb88320 & mask);
        }
    }

    return (int)~crc;
}

/**
 * @brief Validate checksum
 *
 * @param data Data to validate checksum
 * @param length Data length
 * @param checksum Checksum
 * @return int Checksum
 */
int validate_checksum(void *data, size_t length, int checksum) 
{
    return generate_checksum(data, length) == checksum;
}

/**
 * @brief Defragment data
 *
 * @param first First fragment
 * @return char* Defragmented data
 */
char* defragment(fragments* first)
{
    fragments* current = first;
    size_t data_size = 0;

    while(current)
    {
        data_size += current->content_size;
        current = current->next;
    }

    char* data = calloc(data_size, sizeof(char));

    current = first;

    size_t offset = 0;

    while(current)
    {
        memcpy(data + offset, current->data, current->content_size);

        offset += current->content_size;
        current = current->next;
    }

    return data;
}

/**
 * @brief Get relative size of fragment payload
 *
 * @param data Data
 * @param data_size Data size
 * @param fragment_size Fragment size
 * @return size_t Relative size
 */
size_t get_relative_size(char* data, size_t data_size, size_t fragment_size)
{
    size_t count = 0;
    size_t acumulate = 0;

    do
    {
        char c = data[count];

        if(c >= 100)
            acumulate += 4;
        else if(c >= 10)
            acumulate += 3;
        else if(c >= 0)
            acumulate += 2;
        else if(c <= -100)
            acumulate += 5;
        else if(c <= -10)
            acumulate += 4;
        else
            acumulate += 3;
        
        count++;
    } while (acumulate < fragment_size && count < data_size);

    return count;
}

/**
 * @brief Fragment data
 *
 * @param data Data to fragment
 * @param data_size Data size
 * @param fragment_size Fragment size
 * @return fragments* First fragment
 */
fragments* fragment(char* data, size_t data_size, size_t fragment_size)
{
    fragments* first = calloc(1, sizeof(fragments));
    fragments* current = first;

    current->next = NULL;
    current->total_size = data_size;

    size_t remaining_data_size = data_size;
    size_t current_data_size = 0;

    while(remaining_data_size > 0)
    {
        current_data_size = remaining_data_size > fragment_size ? get_relative_size(data + data_size - remaining_data_size, fragment_size, fragment_size) : get_relative_size(data + data_size - remaining_data_size, remaining_data_size, fragment_size);

        memcpy(current->data, data + data_size - remaining_data_size, current_data_size);

        current->content_size = current_data_size;

        current->checksum = generate_checksum(current->data, current->content_size);

        remaining_data_size -= current_data_size;

        if(remaining_data_size > 0)
        {
            current->last = 0;
            current->next = calloc(1, sizeof(fragments));
            
            current->next->total_size = current->total_size;

            current = current->next; 

            current->next = NULL;  
        }
    }

    current->last = 1;

    return first;
}

/**
 * @brief Free fragment list
 *
 * @param first First fragment
 */
void free_package_list(fragments* first) 
{
    fragments* current = first;
    fragments* aux;

    while(current)
    {
        aux = current;
        current = current->next;
        free(aux);
    }
}

/**
 * @brief Encode fragment to JSON
 *
 * @param package Fragment
 * @return char* JSON string
 */
char* encode_json(fragments* package)
{
    size_t json_size = (size_t) snprintf(NULL, 0, "{\"checksum\":%d,\"total_size\":%zu,\"content_size\":%zu,\"last\":%u,\"data\":[", 
                                package->checksum, package->total_size, package->content_size, package->last);

    size_t data_size = (size_t) snprintf(NULL, 0, "\"%d\"", package->data[0]);

    for (size_t i = 1; i < package->content_size; i++)
        data_size += (size_t) snprintf(NULL, 0, ",\"%d\"", package->data[i]);

    json_size += data_size + 2;

    char* json_string = calloc(json_size + 1, sizeof(char));

    size_t offset = (size_t) snprintf(json_string, json_size + 1, "{\"checksum\":%d,\"total_size\":%zu,\"content_size\":%zu,\"last\":%u,\"data\":[%d", 
                             package->checksum, package->total_size, package->content_size, package->last, package->data[0]);

    for (size_t i = 1; i < package->content_size; i++)
        offset += (size_t) snprintf(json_string + offset, json_size + 1 - offset, ",%d", package->data[i]);

    offset += (size_t) snprintf(json_string + offset, json_size + 1 - offset, "]}");

    return json_string;
}

/**
 * @brief Decode JSON to fragment
 *
 * @param json_string JSON string
 * @return fragments* Fragment
 */
fragments* decode_json (const char* json_string)
{
    fragments *package = calloc(1, sizeof(fragments));
    char* ptr;

    package->next = NULL;

    ptr = strstr(json_string, "\"checksum\":");
    if (ptr != NULL)
        package->checksum = atoi(ptr + strlen("\"checksum\":"));
    
    ptr = strstr(json_string, "\"total_size\":");
    if (ptr != NULL) 
        package->total_size = (size_t) atoll(ptr + strlen("\"total_size\":"));

    ptr = strstr(json_string, "\"content_size\":");
    if (ptr != NULL)
        package->content_size = (size_t) atoll(ptr + strlen("\"content_size\":"));

    ptr = strstr(json_string, "\"last\":");
    if (ptr != NULL)
        package->last = (uint8_t) atoi(ptr + strlen("\"last\":"));

    ptr = strstr(json_string, ",\"data\":[");
    if (ptr != NULL)
    {
        ptr += strlen(",\"data\":[");

        for (size_t i = 0; i < package->content_size; i++) 
        {
            sscanf(ptr, "%hhd", (signed char*)&package->data[i]);

            ptr = strchr(ptr, ',');

            if (ptr == NULL || *(ptr + 1) == ']')
                break;

            ptr++;
        }
    }

    return package;
}

error_code receive_data(int sockect_fd, char **buffer, size_t* bytes_received, volatile sig_atomic_t *end_flag)
{
    fragments* first = NULL, *current = NULL, *prev = NULL;
    int resend;

    if(bytes_received)
        *bytes_received = 0;

    while (1)
    {
        struct timeval timeout = {0, 100000};
        fd_set read_fds;

        FD_ZERO(&read_fds);
        FD_SET(sockect_fd, &read_fds);

        if(end_flag && *end_flag)
            return END_SIGNAL;

        if (select(sockect_fd + 1, &read_fds, NULL, NULL, &timeout) > 0)
        {
            if(FD_ISSET(sockect_fd, &read_fds))
            {
                char* json_package = calloc(FRAGMENT_SIZE + 1, sizeof(char));

                if(recv(sockect_fd, json_package, FRAGMENT_SIZE, 0) == 0)
                    return ERROR_SOCKET_DISCONNECT;

                current = decode_json(json_package);

                if(!first)
                    first = current;
                else
                    prev->next = current;

                free(json_package);

                if(!validate_checksum(current->data, current->content_size, current->checksum))
                {
                    resend = 1;
                    send(sockect_fd, &resend, sizeof(int), 0);

                    if(prev)
                        prev->next = NULL;
                    else
                        first = NULL;

                    continue;
                }

                resend = 0;
                send(sockect_fd, &resend, sizeof(int), 0);

                if(bytes_received)
                    *bytes_received += current->content_size;

                if(current->last)
                    break;

                prev = current;
            }
        }
    }

    *buffer = defragment(first);

    free_package_list(first);

    return SUCCESS;
}

error_code send_data(int sockect_fd, char *data, size_t data_size, volatile sig_atomic_t *end_flag) 
{
    fragments* first = fragment(data, data_size, DATA_FRAGMENT_SIZE);
    fragments* current = first;
    int resend;
    int retries;
    size_t sending_bytes = 0;

    while(current)
    {
        char* json_package = encode_json(current);
        
        retries = 0;

        do
        {
            if(end_flag && *end_flag)
                return END_SIGNAL;

            if (send(sockect_fd, json_package, FRAGMENT_SIZE, 0) < 0) 
            {
                if(retries > 3)
                {
                    free_package_list(first);
                    return ERROR_SOCKET_SEND;
                }
                else
                    retries++;
            }

            recv(sockect_fd, &resend, sizeof(int), 0);
        } while (resend);
        
        sending_bytes += current->content_size;
        
        current = current->next;
    }

    free_package_list(first);

    return SUCCESS;
}