#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"


void raise_intr(uint8_t intr_no) {
#ifdef IA32_INTR
	/*printf("Please implement raise_intr()");
	assert(0);*/
	cpu.esp-=4;
	vaddr_write(cpu.esp, 2, 4, cpu.eflags.val); //push EFLAGS
	cpu.esp-=4;
	uint32_t val=(cpu.cs.val << 16) >> 16;
	vaddr_write(cpu.esp, 2, 4, val); //push CS
	cpu.esp-=4;
	vaddr_write(cpu.esp, 2, 4, cpu.eip); //push eip
	uint32_t addr=cpu.idtr.base+(intr_no << 3); //get the address
	GateDesc idt;
	idt.val[0]=laddr_read(addr,4); //read the first 4 bytes of the GateDesc
	idt.val[1]=laddr_read(addr+4,4); //read the second 4 bytes of the GateDesc
	assert(idt.present==1);
	if(idt.type==0xE) //this type is interrupt, clear IF in EFLAGS
		cpu.eflags.IF=0;
	cpu.cs.val=idt.selector;
	uint32_t Offset=(idt.offset_31_16 << 16)+idt.offset_15_0;
	cpu.eip=Offset;
#endif
}

void raise_sw_intr(uint8_t intr_no) {
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
