#include "cpu/instr.h"
#include "cpu/reg.h"
#include "device/port_io.h"

make_instr_func(in_b) {
	uint16_t port=cpu.gpr[2]._16;
	uint32_t readport=pio_read(port,1);
	readport=(readport << 24) >> 24;
	cpu.gpr[0].val=readport;
	print_asm_0("in", "", 1);
	return 1;
}

make_instr_func(in_v) {
	uint16_t port=cpu.gpr[2]._16;
	uint32_t readport=pio_read(port,data_size/8);
	readport=(readport << (32-data_size)) >> (32-data_size);
	cpu.gpr[0].val=readport;
	print_asm_0("in", "", 1);
	return 1;
}
