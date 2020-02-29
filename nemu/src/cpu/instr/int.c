#include "cpu/instr.h"
#include "cpu/intr.h"

make_instr_func(int_) {
	int len=1;
	decode_data_size_b
	decode_operand_i
	operand_read(&opr_src);
	raise_sw_intr(opr_src.val);
	print_asm_1("int", "", 2, &opr_src);
	return 0;
}
