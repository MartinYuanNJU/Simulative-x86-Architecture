#include "cpu/cpu.h"
#include "memory/memory.h"

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr) {
#ifndef TLB_ENABLED
	/*printf("\nPlease implement page_translate()\n");
	assert(0);*/
	uint32_t dir=(laddr & 0xFFC00000) >> 22;
	uint32_t page=(laddr & 0x3FF000) >> 12;
	uint32_t offset=laddr & 0xFFF;
	PDE directory;
	uint32_t dir_entry=(cpu.cr3.pdbr << 12)+(dir << 2);
	directory.val=paddr_read(dir_entry,4);
	assert(directory.present==1);
	PTE table;
	uint32_t tbl_entry=(directory.page_frame << 12)+(page << 2);
	table.val=paddr_read(tbl_entry,4);
	assert(table.present==1);
	uint32_t physical_addr=(table.page_frame << 12)+offset;
	return physical_addr;
#else	
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
