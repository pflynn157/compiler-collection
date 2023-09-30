#include <stdint.h>

extern void syscall_str4(int code, int fd, const char *buf, int size);
extern int strlen(const char *line);
extern int stringcmp(const char *l1, const char *l2);

// Needed for the floating point stuff
int precision = 6;

void printInt(int num)
{
    // For some reason, weird things happen with just 0
    if (num == 0) {
        syscall_str4(1, 1, "0", 1);
        return;
    }
    
    if (num < 0) {
        num *= -1;
        syscall_str4(1, 1, "-", 1);
    }

    // Count the number of digits
    int size = 0;
    int num2 = num;
    while (num2 != 0) {
        num2 = num2 / 10;
        ++size;
    }
    
    // Convert to a string
    char buf1[size];
    int index = size - 1;
    num2 = num;
    
    while (num2 != 0) {
        int digit = num2 % 10;
        num2 = num2 / 10;
        buf1[index] = digit + '0';
        --index;
    }
    
    // Print
    syscall_str4(1, 1, (char *)buf1, size);
}

char getHex(int num)
{
    char hex;
    switch (num) {
        case 10: hex = 'A'; break;
        case 11: hex = 'B'; break;
        case 12: hex = 'C'; break;
        case 13: hex = 'D'; break;
        case 14: hex = 'E'; break;
        case 15: hex = 'F'; break;
        default: {
            hex = num + '0';
        }
    }
    return hex;
}

void printHex(int num)
{
    if (num == 0) {
        syscall_str4(1, 1, "0", 1);
        return;
    }
    
    if (num <= 15) {
        char hex = getHex(num);
        syscall_str4(1, 1, &hex, 1);
        return;
    }
    
    // Determine length
    int length = 1;
    int num2 = num;
    while (num2 > 15) {
        length += 1;
        num2 /= 16;
    }
    
    char number[length];
    int index = length - 1;
    
    // Now print
    while (num > 15) {
        int digit = num % 16;
        num /= 16;
        
        number[index] = getHex(digit);
        --index;
    }
    
    number[index] = getHex(num);
    
    // Print
    syscall_str4(1, 1, (char *)number, length);
}

void printf(const char *line, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5)
{
    int size = strlen(line);
    char buffer[size];
    int index = 0;
    int argIndex = 0;
    
    for (int i = 0; i<size; i++) {
        if (line[i] == '%' && line[i+1] == 'd') {
            syscall_str4(1, 1, (char *)buffer, index);
            for (int j = 0; j<index; j++) buffer[j] = 0;
            index = 0;
            
            switch (argIndex) {
                case 0: printInt(arg1); break;
                case 1: printInt(arg2); break;
                case 2: printInt(arg3); break;
                case 3: printInt(arg4); break;
                case 4: printInt(arg5); break;
                default: {}
            }
            
            ++i;
            ++argIndex;
        } else if (line[i] == '%' && line[i+1] == 'x') {
            syscall_str4(1, 1, (char *)buffer, index);
            for (int j = 0; j<index; j++) buffer[j] = 0;
            index = 0;
            
            switch (argIndex) {
                case 0: printHex(arg1); break;
                case 1: printHex(arg2); break;
                case 2: printHex(arg3); break;
                case 3: printHex(arg4); break;
                case 4: printHex(arg5); break;
                default: {}
            }
            
            ++i;
            ++argIndex;
        } else if (line[i] == '%' && line[i+1] == 'c') {
            syscall_str4(1, 1, (char *)buffer, index);
            for (int j = 0; j<index; j++) buffer[j] = 0;
            index = 0;
            
            char buf[1];
            
            switch (argIndex) {
                case 0: buf[0] = (char)arg1; break;
                case 1: buf[0] = (char)arg2; break;
                case 2: buf[0] = (char)arg3; break;
                case 3: buf[0] = (char)arg4; break;
                case 4: buf[0] = (char)arg5; break;
                default: {}
            }
            
            syscall_str4(1, 1, (char *)buf, 1);
            ++i;
            ++argIndex;
        } else if (line[i] == '\\' && line[i+1] == 'n') {
            buffer[index] = '\n';
            ++index;
        } else {
            buffer[index] = line[i];
            ++index;
        }
    }
    
    syscall_str4(1, 1, (char *)buffer, index);
}

void printDouble(double num)
{
   char flt_num[64];
   int i = 0;
   
   if (num < 0.0) {
       flt_num[i++] = '-';
       num *= -1;
   }
   
   int whole = num;     // Gets us the whole number part
   num -= whole;        // Gets us the decimal part
   
   // Print the whole number part
   if (whole == 0) {
       flt_num[i++] = '0';
   } else {
       // Count the number of digits
       int digits = 0;
       int whole2 = whole;
       while (whole2 != 0) {
           whole2 /= 10;
           ++digits;
       }
       
       char buf[digits];
       int j = 0;
       while (whole != 0) {
           int digit = whole % 10;
           whole /= 10;
           buf[j++] = digit + '0';
       }
       
       for (int ii = digits - 1; ii>=0; ii--) flt_num[i++] = buf[ii];
   }
   
   flt_num[i++] = '.';
   
   // Print the decimal part
   for (int j = 0; j<precision; j++) {
       num *= 10.0;
       int digit = num;
       flt_num[i++] = digit + '0';
       num -= digit;
   }
   
   flt_num[i++] = '\n';
   //flt_num[i++] = '\0';
   
   syscall_str4(1, 1, (char *)flt_num, i);
}

void printFloat(float num)
{
    printDouble(num);
}

void setPrecision(int p)
{
    precision = p;
}
