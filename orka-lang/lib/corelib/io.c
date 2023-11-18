#include <stdint.h>
#include <string.h>
#include <stdio.h>

void println(const char *line)
{
    puts(line);
}

void printDouble(double n) {
    printf("%.2lf\n", n);
}

void printFloat(float n) {
    printf("%.2f\n", n);
}

// TODO: Move this elsewhere
typedef struct
{
    char *array;
    int size;
} CharArray;

void printCharArray(CharArray *array)
{
    //syscall_str4(1, 1, array->array, array->size);
}

