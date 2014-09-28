#define MEM_SIZE 0x10000

uint8_t *mem_get_mem();
void mem_bit_set(uint16_t addr, uint8_t bit);
void mem_bit_unset(uint16_t addr, uint8_t bit);
uint8_t mem_bit_test(uint16_t addr, uint8_t bit);
uint8_t mem_read_8(uint16_t addr);
uint16_t mem_read_16(uint16_t addr);
void mem_write_8(uint16_t addr, uint8_t v);
void mem_write_16(uint16_t addr, uint16_t v);
void mem_load(uint16_t addr, uint8_t *buf, uint16_t size);
void mem_dump(uint16_t start, uint16_t end);
void mem_dump_io_regs();
