#include "vm.hpp"

int main() {
    VM vm;
    vm.load("first.bin", true);
    vm.run(true);
    return 0;
}

