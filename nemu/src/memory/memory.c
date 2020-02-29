#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "device/mm_io.h"
#include "memory/cache.h"
#include <memory.h>
#include <stdio.h>


uint8_t hw_mem[MEM_SIZE_B];

uint32_t hw_mem_read(paddr_t paddr, size_t len) {
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data) {
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len) {
	uint32_t ret = 0;
#ifdef CACHE_ENABLED
	ret = cache_read(paddr, len);
#else
	ret = hw_mem_read(paddr, len);
#endif
	if(is_mmio(paddr)!=-1)
		return mmio_read(paddr,len,is_mmio(paddr));
	else
		return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data) {
#ifdef CACHE_ENABLED
	cache_write(paddr, len, data);
#else
	if(is_mmio(paddr)!=-1)
		mmio_write(paddr,len,data,is_mmio(paddr));
	else
		hw_mem_write(paddr, len, data);
#endif
}


uint32_t laddr_read(laddr_t laddr, size_t len) {
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_PAGE
	if(cpu.cr0.pg==1&&cpu.cr0.pe==1)
	{
		uint32_t new_laddr=(uint32_t)laddr+len-0x1;
		uint32_t before=(((uint32_t)laddr) & 0x1000) >> 12;
		uint32_t after=(((uint32_t)new_laddr) & 0x1000) >> 12;
		if(before!=after) //data cross the page boundary
		{
			uint32_t line_end=(((uint32_t)laddr) & 0xFFFFF000) + 0xFFF;
			uint32_t lenL=line_end-laddr+0x1;
			uint32_t lenH=(uint32_t)len-lenL;
			uint32_t hwaddrL = page_translate(laddr);
			uint32_t hwaddrH = page_translate(laddr+(laddr_t)lenL);
			uint32_t dataL=hw_mem_read(hwaddrL, lenL); //fetch low-order-bit data
			uint32_t dataH=hw_mem_read(hwaddrH, lenH); //fetch high-order-bit data
			return (dataL+(dataH << (lenL*8))) << (32-len*8) >> (32-len*8); //combine low-order-bit data with high-order-bit data as the result
		}
		else
		{
			uint32_t hwaddr = page_translate(laddr);
			return paddr_read(hwaddr, len);
		}
	}
	else
		return paddr_read(laddr, len);
#else
	return paddr_read(laddr, len);
#endif
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data) {
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_PAGE
	if(cpu.cr0.pg==1&&cpu.cr0.pe==1)
	{
		uint32_t new_laddr=(uint32_t)laddr+len-0x1;
		uint32_t before=(((uint32_t)laddr) & 0x1000) >> 12;
		uint32_t after=(((uint32_t)new_laddr) & 0x1000) >> 12;
		if(before!=after) //data cross the page boundary
		{
			uint32_t line_end=(((uint32_t)laddr) & 0xFFFFF000) + 0xFFF;
			uint32_t lenL=line_end-laddr+0x1;
			uint32_t lenH=(uint32_t)len-lenL;
			uint32_t hwaddrL = page_translate(laddr);
			uint32_t hwaddrH = page_translate(laddr+(laddr_t)lenL);
			uint32_t dataL=(data << (lenH*8)) >> (lenH*8); //fetch low-order-bit data
			uint32_t dataH=data >> (lenL*8); //fetch high-order-bit data
			hw_mem_write(hwaddrL, lenL, dataL); //write low-order-bit data
			hw_mem_write(hwaddrH, lenH, dataH); //write high-order-bit data
		}
		else
		{
			uint32_t hwaddr = page_translate(laddr);
			paddr_write(hwaddr, len, data);
		}
	}
	else
		paddr_write(laddr, len, data);
#else
	paddr_write(laddr, len, data);
#endif
}


uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len) {
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
	return laddr_read(vaddr,len);
#else
	uint32_t laddr=vaddr;
	if(cpu.cr0.pe==1)
		laddr=segment_translate(vaddr,sreg);
	return laddr_read(laddr,len);
#endif
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data) {
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
	return laddr_write(vaddr,len,data);
#else
	uint32_t laddr=vaddr;
	if(cpu.cr0.pe==1)
		laddr=segment_translate(vaddr,sreg);
	return laddr_write(laddr,len,data);
#endif
}

void init_mem() {
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);

#ifdef CACHE_ENABLED
	init_cache();
#endif

#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len) {
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t * get_mem_addr() {
	return hw_mem;
}
