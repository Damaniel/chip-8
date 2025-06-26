#include "../include/chip-8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand(time(NULL));
    Chip8 *device = new Chip8();
    device->init();
    device->load_file("ibm_logo.ch8", PROGRAM_OFFSET);
    device->run();

    return 0;
}