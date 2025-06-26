
#define NUM_REGISTERS 16
#define MEM_SIZE 4096
#define FONT_OFFSET 0x0050
#define FONT_SIZE 80
#define STACK_SIZE 256
#define PROGRAM_OFFSET 0x0200

typedef struct {
    unsigned char v[NUM_REGISTERS];     // General registers
    unsigned short i;                   // Index register
    unsigned short sp;                  // Stack pointer
    unsigned short pc;                  // Program counter
} RegisterSet;

class Chip8 {
    private:
        RegisterSet regs;
        unsigned char mem[MEM_SIZE];
        unsigned short stack[STACK_SIZE];
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
        void run(void);
};
