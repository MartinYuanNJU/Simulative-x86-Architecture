#include "cpu/instr.h"
#include "cpu/cpu.h"
#include "memory/memory.h"

make_instr_func(iret) {
	OPERAND regeip,sregcs,eflags;

	regeip.type=OPR_MEM;
	regeip.sreg=SREG_SS;
	regeip.data_size=32;
	regeip.addr=cpu.esp;
	operand_read(&regeip); //pop eip
	cpu.esp+=4;
	cpu.eip=regeip.val;
	
	sregcs.type=OPR_MEM;
	sregcs.sreg=SREG_SS;
	sregcs.data_size=32;
	sregcs.addr=cpu.esp;
	operand_read(&sregcs); //pop cs
	cpu.esp+=4;
	cpu.cs.val=(uint16_t)sregcs.val;

	eflags.type=OPR_MEM;
	eflags.sreg=SREG_SS;
	eflags.data_size=32;
	eflags.addr=cpu.esp;
	operand_read(&eflags); //pop eflags
	cpu.esp+=4;
	cpu.eflags.val=eflags.val;

	load_sreg(1); //reload the hidden part of CS

	print_asm_0("int", "", 1);
	return 0;
}
