#include <stdint.h>

uint8_t *malloc(int64_t size);

int strlen(const char *line)
{
    int index = 0;
    char c = line[index];
    while (c) {
        ++index;
        c = line[index];
    }
    return index;
}

int stringcmp(const char *str1, const char *str2)
{
    int length = strlen(str1);
    if (length != strlen(str2)) return 0;
    
    for (int i = 0; i<length; i++) {
        if (str1[i] != str2[i]) return 0;
    }
    
    return 1;
}

char *strcat_char(const char *str, char c)
{
    int len = strlen(str);
    char *new_str = malloc(len + 1);
    for (int i = 0; i<len; i++) new_str[i] = str[i];
    new_str[len] = c;
    new_str[len+1] = '\0';
    return new_str;
}

char *strcat_str(const char *str, const char *str2)
{
    int len1 = strlen(str);
    int len2 = strlen(str2);
    
    char *new_str = malloc(len1 + len2 + 1);
    int i;
    for (i = 0; i<len1; i++) new_str[i] = str[i];
    int index = i;
    for (i = 0; i<len2; i++) {
        new_str[index] = str2[i];
        ++index;
    }
    
    new_str[index] = '\0';
    return new_str;
}
