#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/mmu/segment.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg) {
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */
	uint32_t linear_addr=cpu.segReg[sreg].base+offset;
	return linear_addr;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg) {
	/* TODO: load the invisibile part of the segment register 'sreg' by reading the GDT.
	 * The visible part of 'sreg' should be assigned by mov or ljmp already.
	 */
	uint32_t addr=cpu.gdtr.base+(cpu.segReg[sreg].index << 3); //get the address
	SegDesc temp;
	temp.val[0]=laddr_read(addr,4); //read the first 4 bytes of the SegDesc
	temp.val[1]=laddr_read(addr+4,4); //read the second 4 bytes of the SegDesc
	assert(temp.present==1);
	assert(temp.privilege_level>=cpu.segReg[sreg].rpl);
	assert(temp.granularity==1);
	uint32_t Base=(temp.base_31_24 << 24)+(temp.base_23_16 << 16)+temp.base_15_0;
	uint32_t Limit=(temp.limit_19_16 << 16)+temp.limit_15_0;
	cpu.segReg[sreg].base=Base;
	cpu.segReg[sreg].limit=Limit;
	cpu.segReg[sreg].type=temp.type;
	cpu.segReg[sreg].privilege_level=temp.privilege_level;
	cpu.segReg[sreg].soft_use=temp.soft_use;
	assert(cpu.segReg[sreg].base==0);
	assert(cpu.segReg[sreg].limit==0xFFFFF);
}
