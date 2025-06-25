#include <stdio.h>
#include "../include/chip-8.h"

Chip8 g_device;

// The data for the default CHIP-8 hex font
unsigned char font_data[] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xf0, 0x10, 0xf0, 0x80, 0xf0,
    0xf0, 0x10, 0xf0, 0x10, 0xf0,
    0x90, 0x90, 0xf0, 0x10, 0x10,
    0xf0, 0x80, 0xf0, 0x10, 0xf0,
    0xf0, 0x80, 0xf0, 0x10, 0xf0,
    0xf0, 0x10, 0x20, 0x40, 0x40,
    0xf0, 0x90, 0xf0, 0x90, 0xf0,
    0xf0, 0x90, 0xf0, 0x10, 0xf0,
    0xf0, 0x90, 0xf0, 0x90, 0x90,
    0xe0, 0x90, 0xe0, 0x90, 0xe0,
    0xf0, 0x80, 0x80, 0x80, 0xf0,
    0xe0, 0x90, 0x90, 0x90, 0xe0,
    0xf0, 0x80, 0xf0, 0x80, 0xf0,
    0xf0, 0x80, 0xf0, 0x80, 0x80
};

/* Clears the 4k of memory space and 512 byte stack and loads the font data */
void init_mem(Chip8 *c8) {
    int i;

    for (i = 0; i < MEM_SIZE; ++i) {
        c8->mem[i] = 0;
    }
    for (i = 0; i < STACK_SIZE; ++i) {
        c8->stack[i] = 0;
    }
    for (i = 0; i < FONT_SIZE; ++i) {
        c8->mem[i + FONT_OFFSET] = font_data[i];
    }
}

void init_registers(Chip8 *c8) {
    int i;
    for (i = 0; i < NUM_REGISTERS; ++i) {
        c8->reg.v[i] = 0;
    }
    c8->reg.i = 0;
    c8->reg.sp = 0;
    c8->reg.pc = 0;
}

void init_device(Chip8 *c8) {
    // Clear the memory
    init_mem(c8);
    // Clear the registers
    init_registers(c8);
}

int load_file(Chip8 *c8, char *filename, short offset) {
    // Load the file as binary starting at the specified offset
    // returns 0 on success, non zero if the file attempts to load
    // past the end of the 4k memory area, or if the file can't be
    // opened at all
    unsigned char c;
    int idx = offset;
    FILE *fp = fopen(filename, "rb");

    if (!fp) {
        printf("Unable to open file!\n");
        fclose(fp);
        return -1;
    }

    // Loop through and read all of the data in the file
    c = fgetc(fp);
    while(!feof(fp)) {
        if (idx >= MEM_SIZE) {
            printf("Wrote past end of memory!\n");
            fclose(fp);
            return -1;
        }
        c8->mem[idx] = c;
        ++idx;
        c = fgetc(fp);
    }
    fclose(fp);

    printf("\nWrote a total of %d bytes into memory at offset 0x%x\n", idx - offset, offset);
    return 0;
}

int main(void) {
    init_device(&g_device);
    load_file(&g_device, "ibm_logo.ch8", 0x0200);
    return 0;
}