#include "cpu/instr.h"
#include "cpu/alu.h"

static void instr_execute_1op() {
	operand_read(&opr_src);
	uint32_t tempflags = cpu.eflags.CF;
	opr_src.val = alu_add(1, opr_src.val, opr_src.data_size);
	cpu.eflags.CF = tempflags;
	operand_write(&opr_src);
}

make_instr_impl_1op(inc, r, v)
make_instr_impl_1op(inc, rm, v)
