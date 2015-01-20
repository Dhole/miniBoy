#include <stdint.h>
#include <stdio.h>
#include "memory.h"
#include "lr35902.h"
#include "io_regs.h"

const uint32_t div_cte = CPU_FREQ / 16384;

static uint32_t cyc_cnt, cyc_div_cnt, div;
static uint8_t div_reg, tim_reg, timemod_reg, timcont_reg;

void timer_write_8(uint16_t addr, uint8_t v) {
	switch (addr) {
	case IO_DIVIDER:
		div_reg = 0;
		break;
	case IO_TIMECNT:
		tim_reg = v;
		break;
	case IO_TIMEMOD:
		timemod_reg = v;
		break;
	case IO_TIMCONT:
		timcont_reg = v;
		switch (timcont_reg & MASK_IO_TIMCONT_clock) {
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
		break;
	default:
		break;
	}
}
uint8_t timer_read_8(uint16_t addr) {
	switch (addr) {
	case IO_DIVIDER:
		return div_reg;
		break;
	case IO_TIMECNT:
		return tim_reg;
		break;
	case IO_TIMEMOD:
		return timemod_reg;
		break;
	case IO_TIMCONT:
		return timcont_reg | 0xF8;
		break;
	default:
		break;
	}
	return 0;
}

void timer_init() {
	cyc_cnt = 0;
	cyc_div_cnt = 0;
}

void timer_emulate(uint32_t cycles) {

	cyc_div_cnt += cycles;
	while (cyc_div_cnt / div_cte > 0) {
		cyc_div_cnt -= div_cte;
		div_reg +=1;
	}

	if (!mem_bit_test(timcont_reg, MASK_IO_TIMCONT_Start)) {
		return;
	} 

	cyc_cnt += cycles;
	while (cyc_cnt / div > 0) {
		cyc_cnt -= div;

		tim_reg += 1;
		if (tim_reg == 0) {
			// Overflow: Interrupt Timer
			//printf("Timer Overflow\n");
			mem_bit_set(IO_IFLAGS, MASK_IO_INT_Timer_Overflow);
			tim_reg = timemod_reg;
		} 
	}
}
