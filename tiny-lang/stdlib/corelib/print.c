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

char get_hex(int num)
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

void print_hex(int num)
{
    if (num == 0) {
        output("0", 1);
        return;
    }
    
    if (num <= 15) {
        char hex = get_hex(num);
        output(&hex, 1);
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
        
        number[index] = get_hex(digit);
        --index;
    }
    
    number[index] = get_hex(num);
    
    // Print
    output((char *)number, length);
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
            
            case 'b': {
                int val = va_arg(argp, int);
                if (val) output("true", 4);
                else output("false", 5);
            } break;
            
            case 'c': {
                char val = va_arg(argp, int);
                output(&val, 1);
            } break;
            
            case 'x': {
                int val = va_arg(argp, int);
                print_hex(val);
            } break;
            
            default: {}
        }
    }
    
    char *nl = "\n";
    output(nl, 1);
    
    va_end(argp);
}

//
// TODO: These are here for historical purposes right now
//
int precision = 6;

void print_double(double num)
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
   
   output((char *)flt_num, i);
}

void print_float(float num)
{
    print_double(num);
}

void set_precision(int p)
{
    precision = p;
}

