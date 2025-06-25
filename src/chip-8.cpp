#include <stdio.h>
#include "../include/chip-8.h"

enum {
    V0, V1, V2, V3, V4, V5, V6, V7,
    V8, V9, VA, VB, VC, VD, VE, VF
} RegName;

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
void Chip8::init_mem(void) {
    int i;

    for (i = 0; i < MEM_SIZE; ++i) {
        mem[i] = 0;
    }
    for (i = 0; i < STACK_SIZE; ++i) {
        stack[i] = 0;
    }
    for (i = 0; i < FONT_SIZE; ++i) {
        mem[i + FONT_OFFSET] = font_data[i];
    }
}

void Chip8::init_registers(void) {
    int i;
    for (i = 0; i < NUM_REGISTERS; ++i) {
        regs.v[i] = 0;
    }
    regs.i = 0;
    regs.sp = 0;
    regs.pc = PROGRAM_OFFSET;
}

void Chip8::init(void) {
    // Clear the memory
    init_mem();
    // Clear the registers
    init_registers();
}

int Chip8::load_file(char *filename, short offset) {
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
        mem[idx] = c;
        ++idx;
        c = fgetc(fp);
    }
    fclose(fp);

    printf("\nWrote a total of %d bytes into memory at offset 0x%x\n", idx - offset, offset);
    return 0;
}

void Chip8::dump_memory(void) {
    int i;
    char b1, b2;
    int row_count = 0;

    for (i = 0; i < MEM_SIZE; i += 2) {
        b1 = mem[i];
        b2 = mem[i+1];
        printf("%.2x%.2x ", b1, b2);
        row_count +=1;
        if (row_count > 8) {
            printf("\n");
            row_count = 0;
        }
    }
}

void Chip8::dump_stack(void) {
    int i;
    int row_count = 0;
    for (i = regs.sp ; i >= 0 ; --i) {
        if (regs.sp == i) {
            printf("-> ");
        }
        else {
            printf("   ");
        }
        printf("%3d: %.4x\n", i, stack[i]);
    }
}

unsigned short Chip8::fetch(void) {
    unsigned char b1, b2;
    unsigned short instruction;

    // Get the two bytes that make up the instruction
    // We'll assemble them into a big endian 16-bit value
    // directly
    b1 = mem[regs.pc];
    b2 = mem[regs.pc + 1];

    // Increment the program counter
    regs.pc += 2;

    // Assemble and return the instruction
    instruction = (b1 << 8) | b2;
    return instruction;
}

void Chip8::stack_push(unsigned short val) {
    stack[regs.sp] = val;
    regs.sp += 1;
}

unsigned short Chip8::stack_peek(void) {
    return stack[regs.sp];
}

unsigned short Chip8::stack_pop(void) {
    unsigned short val = stack_peek();
    regs.sp -= 1;
    return val;
}

void Chip8::execute(unsigned short instruction) {
    unsigned char prefix = (instruction & 0xff00) >> 8;

    switch (prefix) {
        case 0x00:
            switch (instruction) {
                case 0x00e0:
                    printf("Clear screen\n");
                    break;
                case 0x00ee:
                    printf("Return to code at %.4x\n", stack_peek());
                    regs.pc = stack_pop();
                    break;
                default:
                    printf("Unknown instruction: %.4x\n", instruction);
                    break;
            }
            break;
        case 0x01:
            break;
        case 0x02:
            break;
        case 0x03:
            break;
        case 0x04:
            break;
        case 0x05:
            break;
        case 0x06:
            break;
        case 0x07:
            break;
        case 0x08:
            break;
        case 0x09:
            break;
        case 0x0a:
            break;
        case 0x0b:
            break;
        case 0x0c:
            break;
        case 0x0d:
            break;
        case 0x0e:
            break;
        case 0x0f:
            break;
        default:
            printf("Illegal opcode: %.4x\n", instruction);
    }
}

void Chip8::run(void) {
    int i;
    unsigned short instruction;
    for(i = 0; i < 100; ++i) {
        instruction = fetch();
        printf("Instruction is %.4x\n", instruction);
        execute(instruction);
    }
}
