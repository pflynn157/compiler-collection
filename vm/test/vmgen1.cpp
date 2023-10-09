#include <iostream>
#include <cstdio>

int main() {
    FILE *file = fopen("first.bin", "wb");
    
    //-------------------------------------------------------------
    // Set A
    // %0 = alloc set
    // 0x30 00
    fputc(0x30, file);
    fputc(0x00, file);
    
    // set.addi i8 %0 #1
    // 0x40 0x01 00 01
    fputc(0x40, file);
    fputc(0x01, file);
    fputc(0x00, file);
    fputc(0x01, file);
    
    // set.addi i8 %0 #2
    // 0x40 0x01 00 02
    fputc(0x40, file);
    fputc(0x01, file);
    fputc(0x00, file);
    fputc(0x02, file);
    
    // set.addi i8 %0 #3
    // 0x40 0x01 00 03
    fputc(0x40, file);
    fputc(0x01, file);
    fputc(0x00, file);
    fputc(0x03, file);
    
    // set.print i8 %0
    // 0x41 0x01 00
    fputc(0x41, file);
    fputc(0x01, file);
    fputc(0x00, file);
    
    //-------------------------------------------------------------
    // Set B
    // %1 = alloc set
    // 0x30 01
    fputc(0x30, file);
    fputc(0x01, file);
    
    // set.addi i8 %1 #2
    // 0x40 0x01 01 02
    fputc(0x40, file);
    fputc(0x01, file);
    fputc(0x01, file);
    fputc(0x02, file);
    
    // set.addi i8 %1 #3
    // 0x40 0x01 01 03
    fputc(0x40, file);
    fputc(0x01, file);
    fputc(0x01, file);
    fputc(0x03, file);
    
    // set.addi i8 %1 #4
    // 0x40 0x01 01 04
    fputc(0x40, file);
    fputc(0x01, file);
    fputc(0x01, file);
    fputc(0x04, file);
    
    // set.print i8 %1
    // 0x41 0x01 01
    fputc(0x41, file);
    fputc(0x01, file);
    fputc(0x01, file);
    
    //-------------------------------------------------------------
    // Set C
    // %2 = alloc set
    // 0x30 02
    fputc(0x30, file);
    fputc(0x02, file);
    
    // set.union i8 %2 %0 %1 
    // 0x50 0x01 02 00 01
    fputc(0x50, file);
    fputc(0x01, file);
    fputc(0x02, file);
    fputc(0x00, file);
    fputc(0x01, file);
    
    // set.print i8 %2
    // 0x41 0x01 02
    fputc(0x41, file);
    fputc(0x01, file);
    fputc(0x02, file);
    
    //-------------------------------------------------------------
    // Set D
    // %3 = alloc set
    // 0x30 03
    fputc(0x30, file);
    fputc(0x03, file);
    
    // set.inter i8 %3 %0 %1 
    // 0x51 0x01 03 00 01
    fputc(0x51, file);
    fputc(0x01, file);
    fputc(0x03, file);
    fputc(0x00, file);
    fputc(0x01, file);
    
    // set.print i8 %3
    // 0x41 0x01 03
    fputc(0x41, file);
    fputc(0x01, file);
    fputc(0x03, file);
    
    fclose(file);
    return 0;
}

