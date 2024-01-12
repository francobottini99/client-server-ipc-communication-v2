#include "client_utils.h"

char* trim_white_space(char* str)
{
    while(*str == ASCII_SPACE)
        str++;

    if(*str == 0 || *str == ASCII_LINE_BREAK)
        return NULL;

    char* endOfStr = str + strlen(str) - 1;

    while(endOfStr > str && (*endOfStr == ASCII_SPACE || *endOfStr == ASCII_LINE_BREAK))
        endOfStr--;

    endOfStr[1] = ASCII_END_OF_STRING;

    return str;
}