#include "../include/chip-8.h"

int main(void) {
    Chip8 *device = new Chip8();
    device->init();
    device->load_file("ibm_logo.ch8", PROGRAM_OFFSET);
    device->run();

    return 0;
}