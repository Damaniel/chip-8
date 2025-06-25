
#define NUM_REGISTERS 16
#define MEM_SIZE 4096
#define FONT_OFFSET 0x0050
#define FONT_SIZE 80
#define STACK_SIZE 256

enum {
    V0, V1, V2, V3, V4, V5, V6, V7,
    V8, V9, VA, VB, VC, VD, VE, VF
} RegName;

typedef struct {
    unsigned char v[NUM_REGISTERS];     // General registers
    unsigned short i;                   // Index register
    unsigned short sp;                  // Stack pointer
    unsigned short pc;                  // Program counter
} RegisterSet;

typedef struct {
    RegisterSet reg;
    unsigned char mem[MEM_SIZE];
    unsigned short stack[STACK_SIZE];
} Chip8;

extern Chip8 g_device;
