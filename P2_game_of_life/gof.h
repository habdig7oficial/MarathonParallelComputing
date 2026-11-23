#include "limits.h"
#include "stdlib.h"
#include <stdint.h>


extern "C" {
    void print_gen(uint8_t *, uint8_t *, unsigned int);
    void print(const uint8_t *, uint8_t, uint8_t, uint8_t);
    void next(uint8_t, uint8_t, uint8_t *, const uint8_t *, uint8_t, unsigned int);
}
