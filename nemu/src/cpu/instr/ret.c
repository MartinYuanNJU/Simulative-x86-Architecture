#include "cpu/instr.h"

make_instr_func(ret_near) {
	OPERAND popnum;
	popnum.type=OPR_MEM;
	popnum.sreg=SREG_CS;
	popnum.addr=cpu.esp;
	popnum.data_size=32;
	operand_read(&popnum);
	cpu.esp+=4;
	cpu.eip=popnum.val;
	print_asm_0("ret", "", 1);
	return 0;
}

make_instr_func(ret_near_imm16) {
	OPERAND popnum;
	int len=1;
	popnum.type=OPR_MEM;
	popnum.sreg=SREG_CS;
	popnum.addr=cpu.esp;
	popnum.data_size=32;
	operand_read(&popnum);
	cpu.esp+=4;
	cpu.eip=popnum.val;
	print_asm_0("ret", "", 3);
	decode_data_size_w
	decode_operand_i
	operand_read(&opr_src);
	cpu.esp+=opr_src.val;
	return 0;
}


