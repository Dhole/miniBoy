#include <stdint.h>
#include "memory.h"
#include "lr35902.h"
#include "io_regs.h"

static uint8_t state;
static uint32_t cyc_cnt;

void timer_emulate(uint32_t cycles) {
	uint8_t cnt;
	int div;

	if (!mem_bit_test(IO_TIMCONT, MASK_IO_TIMCONT_Start)) {
		return;
	} else if (state == 0) {
		state = 1;
		cyc_cnt = 0;
	}

	switch (mem_read_8(IO_TIMCONT) & MASK_IO_TIMCONT_clock) {
	case OPT_Timer_clock_4096:
		div = CPU_FREQ / 4096;
		break;
	case OPT_Timer_clock_262144:
		div = CPU_FREQ / 262144;
		break;
	case OPT_Timer_clock_65536:
		div = CPU_FREQ / 65536;
		break;
	case OPT_Timer_clock_16384:
		div = CPU_FREQ / 16384;
		break;
	}

	cyc_cnt += cycles;

	if (cyc_cnt / div > 0) {
		cyc_cnt -= div;

		cnt = mem_read_8(IO_TIMECNT) + 1;
		if (cnt == 0) {
			// Overflow: Interrupt Timer
			mem_bit_set(IO_IFLAGS, MASK_IO_INT_Timer_Overflow);
			mem_write_8(IO_TIMECNT, mem_read_8(IO_TIMEMOD));
		} else {
			mem_write_8(IO_TIMECNT, cnt);
		}
	}
}
