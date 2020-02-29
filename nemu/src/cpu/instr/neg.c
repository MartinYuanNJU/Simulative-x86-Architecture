#include "cpu/instr.h"
#include "cpu/reg.h"

make_instr_func(neg_rm_v) {
	int len=1;
	decode_data_size_v
	decode_operand_rm
	operand_read(&opr_src);
	if(opr_src.val==0)
		cpu.eflags.CF=0;
	else
		cpu.eflags.CF=1;
	opr_src.val=-opr_src.val;
	operand_write(&opr_src);
	print_asm_1("neg", "", 2, &opr_src);
	return len;
}
