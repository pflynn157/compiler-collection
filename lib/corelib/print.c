#include <stdarg.h>

extern void output(char *input, int len);
extern int strlen(char *input);

void print_int(int val) {
    if (val < 10 && val >= 0) {
        char c = val + '0';
        output(&c, 1);
        return;
    }
    
    // Count number of digits
    int digits = 1;
    int old_val = val;
    while (old_val >= 10) {
        old_val = old_val / 10;
        ++digits;
    }
    
    // Create a string
    char number[digits];
    int index = digits - 1;
    old_val = val;
    while (old_val >= 10) {
        int num = old_val % 10;
        old_val = old_val / 10;
        number[index] = num + '0';
        --index;
    }
    number[index] = old_val + '0';
    
    // Print it out
    output(number, digits);
}

void print(char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);

    for (int i = 0; i<strlen(fmt); i++) {
        switch (fmt[i]) {
            case 's': {
                char *s2 = va_arg(argp, char*);
                output(s2, strlen(s2));
            } break;
            
            case 'd': {
                int val = va_arg(argp, int);
                print_int(val);
            } break;
            
            default: {}
        }
    }
    
    char *nl = "\n";
    output(nl, 1);
    
    va_end(argp);
}

