#include "cpu/instr.h"

make_instr_func(leave) {
	OPERAND popnum;
	cpu.esp=cpu.ebp;
	popnum.type=OPR_MEM;
	popnum.addr=cpu.esp;
	popnum.data_size=32;
	popnum.sreg=SREG_SS;
	operand_read(&popnum);
	cpu.ebp=popnum.val;
	cpu.esp+=4;
	print_asm_0("leave", "", 1);
	return 1;
}

