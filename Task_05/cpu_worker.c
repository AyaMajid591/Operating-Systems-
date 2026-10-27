
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    volatile uint64_t value = 0;

    while (1) {
        value += 1;
    }

    return 0;
}
