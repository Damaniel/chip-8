#include <stdio.h>
#include "../include/chip-8.h"
#include <stdlib.h>

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
    int i, j;

    for (i = 0; i < MEM_SIZE; ++i) {
        mem[i] = 0;
    }
    for (i = 0; i < STACK_SIZE; ++i) {
        stack[i] = 0;
    }
    for (i = 0; i < FONT_SIZE; ++i) {
        mem[i + FONT_OFFSET] = font_data[i];
    }

    for (j = 0;j < CHIP_8_HEIGHT; ++j) {
        for (i = 0; i < CHIP_8_WIDTH; ++i) {
            vmem[i][j] = 0;
        }
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
    regs.delay_timer = 0;
    regs.sound_timer = 0;
}

void Chip8::init(void) {
    int idx;

    // Clear the memory
    init_mem();

    // Clear the registers
    init_registers();

    // Clear the key state
    for (idx = 0; idx < NUM_KEYS; ++idx) {
        key_state[idx] = 0;
    }
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

void Chip8::dump_vmem(void) {
    int i, j;
    for (j = 0; j < CHIP_8_HEIGHT; ++j) {
        for (i = 0; i < CHIP_8_WIDTH; ++i) {
            if (vmem[i][j] == 0) {
                printf(".");
            }
            else {
                printf("X");
            }
        }
        printf("\n");
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
    int idx, idx2, reg, reg2, x_reg, y_reg, num_rows;
    unsigned char draw_x, draw_y;
    unsigned char cval, cval2, rval, digit;
    unsigned short sval, sval2, sprite_off;

    switch (prefix) {
        // Two instructions: clear screen (0x00e0) and return subroutine (0x00ee)
        case 0x0:
            switch (instruction) {
                case 0x00e0:
                    printf("Clear screen\n");
                    for (idx2 = 0;idx2 < CHIP_8_HEIGHT; ++idx2) {
                        for (idx = 0; idx < CHIP_8_WIDTH; ++idx) {
                            vmem[idx][idx2] = 0;
                        }
                    }
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
        // 1NNN - Jump to the location specified by the lower 12 bits
        case 0x1:
            printf("Jump to %.3x\n", (instruction & 0x0fff));
            regs.pc = (instruction & 0x0fff);
            break;
        // 2NNN - Call the subroutine specified by the lower 12 bits
        case 0x2:
            printf("Call subroutine at %.3x\n", (instruction & 0x0fff));
            stack_push(regs.pc);
            regs.pc = (instruction & 0x0fff);
            break;
        // 3XNN - Skip next if Vx == NN
        case 0x3:
            reg = (instruction & 0x0f00) >> 8;
            cval = (instruction & 0x00ff);
            printf("Skip next if %.2x == %.2x\n", regs.v[reg], cval);
            if (regs.v[reg] == cval) {
                regs.pc += 2;
            }
            break;
        // 4XNN - Skip next if Vx != NN
        case 0x4:
            reg = (instruction & 0x0f00) >> 8;
            cval = (instruction & 0x00ff);
            if (regs.v[reg] != cval) {
                regs.pc += 2;
            }
            break;
        // 5XY0 - Skip next if Vx == Vy
        case 0x5:
            reg = (instruction & 0xf00) >> 8;
            reg2 = (instruction & 0x00f0) >> 4;
            if (regs.v[reg] == regs.v[reg2]) {
                regs.pc += 2;
            }
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
        // 8XYN - assorted bit and assignment operations
        case 0x8:
            reg = (instruction & 0x0f00) >> 8;
            reg2 = (instruction & 0x00f0) >> 4;
            cval = (instruction & 0x000f);
            switch (cval) {
                // Assignment (Vx = Vy)
                case 0x0:
                    regs.v[reg] = regs.v[reg2];
                    break;
                // OR (Vx |= Vy)
                case 0x1:
                    regs.v[reg] = regs.v[reg] | regs.v[reg2];
                    break;
                // AND (Vx &= Vy)
                case 0x2:
                    regs.v[reg] = regs.v[reg] & regs.v[reg2];
                    break;
                // XOR (Vx ^= Vy)
                case 0x3:
                    regs.v[reg] = regs.v[reg] ^ regs.v[reg2];
                    break;
                // Add with overflow (Vx += Vy)
                case 0x4:
                    if ((int)regs.v[reg] + (int)regs.v[reg2] > 255)
                        regs.v[VF] = 1;
                    else
                        regs.v[VF] = 0;
                    regs.v[reg] = regs.v[reg] + regs.v[reg2];
                    break;
                // Subtract with underflow (Vx -= Vy)
                case 0x5:
                    if (regs.v[reg] >= regs.v[reg2])
                        regs.v[VF] = 1;
                    else
                        regs.v[VF] = 0;
                    regs.v[reg] = regs.v[reg] - regs.v[reg2];
                    break;
                // Shift right, LSB goes into VF
                case 0x6:
                    regs.v[VF] = regs.v[reg] & 0x01;
                    regs.v[reg] = regs.v[reg] >> 1;
                    break;
                // Subtract Vx from Vy and assign to Vx, set VF if underflow
                case 0x7:
                    if (regs.v[reg2] >= regs.v[reg])
                        regs.v[VF] = 1;
                    else
                        regs.v[VF] = 0;
                    regs.v[reg] = regs.v[reg2] - regs.v[reg];
                    break;
                // Shift left, MSB goes into VF
                case 0xE:
                    regs.v[VF] = regs.v[reg] & 0x80;
                    regs.v[reg] = regs.v[reg] << 1;
                    break;
                default:
                    printf("Unknown operation: %.4x\n", instruction);
                    break;
            }
            break;
        // 9XY0 - skip if Vx != Vy
        case 0x9:
            reg = (instruction & 0x0f00) >> 8;
            reg2 = (instruction & 0x00f0) >> 4;
            if (regs.v[reg] != regs.v[reg2]) {
                regs.pc += 2;
            }
            break;
        // ANNN - set the value of I to NNN
        case 0xa:
            sval = (instruction & 0x0fff);
            printf("Set the value of I to %.3x\n", sval);
            regs.i = sval;
            break;
        // BNNN - Jump to V0 + NNN
        case 0xb:
            sval = (instruction & 0x0fff);
            regs.pc = regs.v[V0] + sval;
            break;
        // CXNN - set Vx to a random value AND NN
        case 0xc:
            reg = (instruction & 0x0f00) >> 8;
            cval = (instruction & 0x00ff);
            rval = rand() % 256;
            regs.v[reg] = rval & cval;
            break;
        // DXYN - draw a sprite using the registers VX and VY, N rows
        case 0xd:
            x_reg = (instruction & 0x0f00) >> 8;
            y_reg = (instruction & 0x00f0) >> 4;
            num_rows = (instruction & 0x000f);
            printf("Draw at %.4x, %.4x, %d rows from %.4x\n", regs.v[x_reg], regs.v[y_reg], num_rows, regs.i);
            // Start row and column are x and y mod width and height, respectively
            draw_x = regs.v[x_reg] % CHIP_8_WIDTH;
            draw_y = regs.v[y_reg] % CHIP_8_HEIGHT;
            // Get the offset of the first row of the sprite
            sprite_off = regs.i;
            for(idx2 = 0; idx2 < num_rows; ++idx2) {
                // If this row is on the screen...
                if (draw_y + idx2 < CHIP_8_HEIGHT) {
                    // Get the row of data
                    cval = mem[sprite_off + idx2];
                    // for each pixel
                    for (idx = 0; idx < 8; ++idx) {
                        // If this pixel is on the screen
                        if (draw_x + idx < CHIP_8_WIDTH) {
                            // extract the pixel value
                            cval2 = (cval & 0x80) >> 7;
                            // If set and the destination is set, clear the space and set VF to 1.
                            // If the destination is clear, set the space
                            // If the pixel is clear, do nothing.
                            if (cval2 == 0x1) {
                                if (vmem[draw_x + idx][draw_y + idx2] == 0x1) {
                                    vmem[draw_x + idx][draw_y + idx2] = 0x0;
                                    regs.v[VF] = 1;
                                }
                                else {
                                    vmem[draw_x + idx][draw_y + idx2] = 0x1;
                                }
                            }
                        }
                        // Shift left to put the next pixel in the row into the MSB
                        cval = cval << 1;
                    }
                }
            }
            break;
        // EXXX - key operations (key up, key down)
        case 0xe:
            sval = (instruction & 0x00ff);
            reg = (instruction & 0x0f00) >> 8;
            switch (sval) {
                // Skip next if key in Vx is pressed
                case 0x9E:
                    if (key_state[regs.v[reg] & 0x0f] != 0) {
                        regs.pc += 2;
                    }
                    break;
                // Skip next if key in Vx is not pressed
                case 0xA1:
                    if (key_state[regs.v[reg] & 0x0f] == 0 ) {
                        regs.pc += 2;
                    }
                    break;
                default:
                    printf("Unrecognized keyboard operation %.4x\n", instruction);
                    break;
            }
            break;
        // FXXX - miscellaneous functions (memory, timer, keys, bcd)
        case 0xf:
            reg = (instruction & 0x0f00) >> 8;
            sval = (instruction & 0x00ff);
            switch (sval) {
                // Set Vx to the value of the delay timer
                case 0x07:
                    reg = (instruction & 0x0f00) >> 8;
                    regs.v[reg] = regs.delay_timer;
                    break;
                // Block until a key is pressed and then set Vx to the key pressed
                // The 'block' is performed by pushing the PC back so that this
                // instruction is run again until a key press shows up
                case 0x0A:
                    if (is_key_pressed) {
                        regs.v[reg] = key_pressed;
                        is_key_pressed = 0;
                    }
                    else {
                        regs.pc -= 2;
                    }
                    break;
                // Set the delay timer to the value in Vx
                case 0x15:
                    reg = (instruction & 0x0f00) >> 8;
                    regs.delay_timer = regs.v[reg];
                    break;
                // Set the sound timer to the value in Vx
                case 0x18:
                    reg = (instruction & 0x0f00) >> 8;
                    regs.sound_timer = regs.v[reg];
                    break;
                // Add Vx to I (VF not affected)
                case 0x1E:
                    cval = (instruction & 0x0f00) >> 8;
                    regs.i += regs.v[cval];
                    break;
                // Set I to the location of the font sprite for the specified hex digit
                case 0x29:
                    cval = (instruction & 0x0f00) >> 8;
                    regs.i = FONT_OFFSET + (5 * cval);
                    break;
                // Store the value of Vx as 3 BCD digits at I, I+1 and I+2
                case 0x33:
                    reg = (instruction & 0x0f00) >> 8;
                    cval = regs.v[reg];
                    // Extract the 1s digit
                    digit = cval % 10;
                    mem[regs.i + 2] = digit;
                    cval = cval / 10;
                    // Extract the 10s digit
                    digit = cval % 10;
                    mem[regs.i + 1] = digit;
                    cval = cval / 10;
                    // Extract the 100s digit
                    digit = cval % 10;
                    mem[regs.i] = digit;
                    break;
                // reg dump (to offset I, but I remains unchanged)
                case 0x55:
                    sval2 = regs.i;
                    for (idx = 0; idx < reg; ++idx) {
                        mem[sval2 + idx] = regs.v[idx];
                    }
                    break;
                // reg load (from offset I, but I remains unchanged)
                case 0x65:
                    sval2 = regs.i;
                    for(idx = 0; idx < reg; ++idx) {
                        regs.v[idx] = mem[sval2 + idx];
                    }
                    break;
            }
            break;
        default:
            printf("Illegal opcode: %.4x\n", instruction);
    }
}

void Chip8::run(void) {
    int i;
    unsigned short instruction;

    for(i = 0; i < 100; ++i) {
        printf("- PC is at %.4x\n", regs.pc);
        instruction = fetch();
        printf("Instruction is %.4x\n", instruction);
        execute(instruction);
    }
}
