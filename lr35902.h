#define CPU_FREQ 4194304

typedef struct {
	uint16_t PC, SP_val;
	uint16_t *SP;
	uint8_t r_data[8];
	uint16_t *AF , *BC, *DE, *HL;
	uint8_t *F, *A, *C, *B, *E, *D, *L, *H;

	uint8_t ime_flag;
} regs_t;

typedef enum {
	MEM_R_8,
	MEM_R_16,
	MEM_W_8,
	MEM_W_16,
	MEM_RW_16,
	NONE,
} opt_t;

typedef struct {
	char *desc;
	void (*handler)(void*, void*);
	void *a;
	void *b;
	opt_t opt;
	uint8_t length;
	uint8_t cycles[2];
} inst_t;

typedef enum {
	Z_FLAG = 1 << 7,
	N_FLAG = 1 << 6,
	H_FLAG = 1 << 5,
	C_FLAG = 1 << 4,
} flag_t;

typedef enum {
	COND_NZ = 0,
	COND_NC = 1,
	COND_Z = 2,
	COND_C = 3,
} cond_t;

void cpu_init();
void cpu_test();
int cpu_step();
void cpu_dump_reg();
regs_t *cpu_get_regs();
uint8_t disas_op(uint16_t off);
uint8_t get_flag(flag_t f);
