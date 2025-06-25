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
    unsigned char prefix = (instruction & 0xf000) >> 12;
    int reg, x_reg, y_reg, num_rows;
    unsigned char cval;
    unsigned short sval;

    switch (prefix) {
        // Two instructions: clear screen (0x00e0) and return subroutine (0x00ee)
        case 0x0:
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
        // Jump to the location specified by the lower 12 bits
        case 0x1:
            printf("Jump to %.3x\n", (instruction & 0x0fff));
            regs.pc = (instruction & 0x0fff);
            break;
        // Call the subroutine specified by the lower 12 bits
        case 0x2:
            printf("Call subroutine at %.3x\n", (instruction & 0x0fff));
            stack_push(regs.pc);
            regs.pc = (instruction & 0x0fff);
            break;
        case 0x3:
            break;
        case 0x4:
            break;
        case 0x5:
            break;
        // 6XNN - set the value of register X to the value NN
        case 0x6:
            reg = (instruction & 0x0f00) >> 8;
            cval = (instruction & 0x00ff);
            printf("Set register V%x to %.2x\n", reg, cval);
            regs.v[reg] = cval;
            break;
        // 7XNN - add the value of NN to register X (with no flags adjusted)
        case 0x7:
            reg = (instruction & 0x0f00) >> 8;
            cval = (instruction & 0x00ff);
            printf("Add %.2x to register v%x\n", reg, cval);
            regs.v[reg] += cval;
            break;
        case 0x8:
            break;
        case 0x9:
            break;
        // ANNN - set the value of I to NNN
        case 0xa:
            sval = (instruction & 0x0fff);
            printf("Set the value of I to %.3x\n", sval);
            regs.i = sval;
            break;
        case 0xb:
            break;
        case 0xc:
            break;
        // DXYN - draw a sprite using the registers VX and VY, N rows
        case 0xd:
            x_reg = (instruction & 0x0f00) >> 8;
            y_reg = (instruction & 0x00f0) >> 4;
            num_rows = (instruction & 0x000f);
            printf("Draw at %.4x, %.4x, %d rows from %.4x\n", regs.v[x_reg], regs.v[y_reg], num_rows, regs.i);
            break;
        case 0xe:
            break;
        case 0xf:
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
