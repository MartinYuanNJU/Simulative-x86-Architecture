#include "cpu/instr.h"

make_instr_func(lgdt) {
	OPERAND limit_addr,base_addr;
	limit_addr.data_size=16;
	int len=1;
	len+=modrm_rm(eip+1,&limit_addr);
	operand_read(&limit_addr);
	base_addr=limit_addr;
	base_addr.data_size=32;
	base_addr.addr+=2;
	operand_read(&base_addr);
	cpu.gdtr.limit=limit_addr.val;
	cpu.gdtr.base=base_addr.val;
	print_asm_1("lgdt", "", 7, &limit_addr);
	return len;
}

