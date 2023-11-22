#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint64_t **address_list;
static size_t address_count;

void gc_init() {
    address_list = malloc(sizeof(uint64_t*)*20);
    address_count = 0;
}

void *gc_alloc(int size) {
    void *ptr = malloc(size);
    address_list[address_count] = ptr;
    ++address_count;
    return ptr;
}

uint8_t *gc_alloc_i8(int size) {
    void *ptr = malloc(sizeof(uint8_t)*size);
    address_list[address_count] = ptr;
    ++address_count;
    return ptr;
}

uint16_t *gc_alloc_i6(int size) {
    void *ptr = malloc(sizeof(uint16_t)*size);
    address_list[address_count] = ptr;
    ++address_count;
    return ptr;
}

uint32_t *gc_alloc_i32(int size) {
    void *ptr = malloc(sizeof(uint32_t)*size);
    address_list[address_count] = ptr;
    ++address_count;
    return ptr;
}

uint64_t *gc_alloc_i64(int size) {
    void *ptr = malloc(sizeof(uint64_t)*size);
    address_list[address_count] = ptr;
    ++address_count;
    return ptr;
}

void gc_destroy() {
    for (int i = 0; i<address_count; i++) {
        free(address_list[i]);
    }
    free(address_list);
}

extern int __main(char **argv, int argc);

int main(int argc, char **argv) {
    gc_init();
    int ret = __main(argv, argc);
    gc_destroy();
    return ret;
}


