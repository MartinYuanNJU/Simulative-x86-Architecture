#include "cpu/instr.h"
#include "cpu/reg.h"
#include "device/port_io.h"

make_instr_func(out_b) {
	uint16_t port=cpu.gpr[2]._16;
	uint32_t data=(cpu.gpr[0].val << 24) >> 24;
	pio_write(port,1,data);
	print_asm_0("out", "", 1);
	return 1;
}

make_instr_func(out_v) {
	uint16_t port=cpu.gpr[2]._16;
	size_t len=data_size/8;
	uint32_t data=(cpu.gpr[0].val << (32-data_size)) >> (32-data_size);
	pio_write(port,len,data);
	print_asm_0("out", "", 1);
	return 1;
}
