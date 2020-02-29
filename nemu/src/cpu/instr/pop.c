#include "cpu/instr.h"

static void instr_execute_1op() {
	OPERAND popnum;
	popnum.type=OPR_MEM;
	popnum.sreg=SREG_SS;
	popnum.addr=cpu.esp;
	popnum.data_size=opr_src.data_size;
	operand_read(&popnum);
	opr_src.sreg=SREG_SS;
	if(opr_src.data_size==16)
		cpu.esp+=2;
	else
		cpu.esp+=4;
	opr_src.val=popnum.val;
	operand_write(&opr_src);
}

make_instr_impl_1op(pop, r, v)

make_instr_func(popa)
{
	OPERAND reg;
	
	reg.type=OPR_MEM;
	reg.sreg=SREG_SS;
	reg.data_size=32;

	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;
	cpu.edi=reg.val;

	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;
	cpu.esi=reg.val;
	
	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;
	cpu.ebp=reg.val;

	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;

	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;
	cpu.ebx=reg.val;

	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;
	cpu.edx=reg.val;

	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;
	cpu.ecx=reg.val;

	reg.addr=cpu.esp;
	operand_read(&reg);
	cpu.esp+=4;
	cpu.eax=reg.val;

	print_asm_0("popa", "", 1);
	return 1;
}
