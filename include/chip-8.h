
#define NUM_REGISTERS 16
#define MEM_SIZE 4096
#define FONT_OFFSET 0x0050
#define FONT_SIZE 80
#define STACK_SIZE 256
#define PROGRAM_OFFSET 0x0200
#define CHIP_8_WIDTH    64
#define CHIP_8_HEIGHT   32
#define NUM_KEYS 16

typedef struct {
    unsigned char v[NUM_REGISTERS];     // General registers
    unsigned short i;                   // Index register
    unsigned short sp;                  // Stack pointer
    unsigned short pc;                  // Program counter
    unsigned char delay_timer;          // Delay timer
    unsigned char sound_timer;          // Sound timer
} RegisterSet;

class Chip8 {
    private:
        RegisterSet regs;
        unsigned char mem[MEM_SIZE];
        unsigned short stack[STACK_SIZE];
        unsigned char vmem[CHIP_8_WIDTH][CHIP_8_HEIGHT];
        // The current down/up state of each of the 16 keys.  Each is set to non-zero on the cycle
        // the key is first held down, and set back to zero on the first frame it is released
        unsigned char key_state[NUM_KEYS];
        // Has a key been pressed since the previous execution loop?
        // If so, is_key_pressed is non-zero and key_pressed holds the key
        unsigned char is_key_pressed;
        unsigned char key_pressed;
        void init_mem(void);
        void init_registers(void);
        unsigned short fetch(void);
        void execute(unsigned short instruction);
        void stack_push(unsigned short val);
        unsigned short stack_peek(void);
        unsigned short stack_pop(void);

    public:
        void init(void);
        int load_file(char *filename, short offset);
        void dump_memory(void);
        void dump_stack(void);
        void dump_vmem(void);
        void run(void);
};
