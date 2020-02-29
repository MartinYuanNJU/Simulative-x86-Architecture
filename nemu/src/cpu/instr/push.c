#include "cpu/instr.h"

static void instr_execute_1op() {
	opr_src.sreg=SREG_SS;
	operand_read(&opr_src);
	OPERAND dest;
	dest.type=OPR_MEM;
	dest.sreg=SREG_SS;
	dest.data_size=opr_src.data_size;
	if(opr_src.data_size==8 && opr_src.type==OPR_IMM)
	{
		cpu.esp-=4;
		dest.addr=cpu.esp;
		dest.val=((int32_t)opr_src.val << 24) >> 24;
		dest.data_size=32;
	}
	else if(opr_src.data_size==16)
	{
		cpu.esp-=2;
		dest.addr=cpu.esp;
		dest.val=opr_src.val;
	}
	else
	{
		cpu.esp-=4;
		dest.addr=cpu.esp;
		dest.val=opr_src.val;
	}
	operand_write(&dest);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, rm, v)

make_instr_func(pusha)
{
	OPERAND reg;
	uint32_t tempesp=cpu.esp;
	reg.type=OPR_MEM;
	reg.sreg=SREG_SS;
	reg.data_size=32;

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=cpu.eax;
	operand_write(&reg);

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=cpu.ecx;
	operand_write(&reg);

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=cpu.edx;
	operand_write(&reg);

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=cpu.ebx;
	operand_write(&reg);

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=tempesp;
	operand_write(&reg);

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=cpu.ebp;
	operand_write(&reg);

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=cpu.esi;
	operand_write(&reg);

	cpu.esp-=4;
	reg.addr=cpu.esp;
	reg.val=cpu.edi;
	operand_write(&reg);

	print_asm_0("pusha", "", 1);
	return 1;
}

