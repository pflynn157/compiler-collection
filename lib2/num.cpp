#include <cstdlib>
#include <ctime>

extern "C" {

void rand_init() {
    srand(time(NULL));
}

int rand_int(int min, int max) {
    return min + std::rand() % ((max + 1) - min);
}

}
