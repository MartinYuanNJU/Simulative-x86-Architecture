#include "cpu/cpu.h"

void set_CF_add(uint32_t result, uint32_t src, size_t data_size) 
{
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size); 	
	cpu.eflags.CF = result < src;
}

void set_CF_adc(uint32_t result, uint32_t src, size_t data_size) 
{
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	if(cpu.eflags.CF==1)
		cpu.eflags.CF = result <= src;
	else
		cpu.eflags.CF = result < src;
}

void set_CF_sub(uint32_t dest, uint32_t src, size_t data_size)
{
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.CF = dest < src;
}

void set_CF_sbb(uint32_t dest, uint32_t src, size_t data_size)
{
	dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	if(cpu.eflags.CF==1)
		cpu.eflags.CF = dest <= src;
	else
		cpu.eflags.CF = dest < src;
}

void set_ZF(uint32_t result, size_t data_size) 
{
	result = result & (0xFFFFFFFF >> (32 - data_size)); 
	cpu.eflags.ZF = (result == 0);
}

void set_SF(uint32_t result, size_t data_size) 
{
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size); 
	cpu.eflags.SF = sign(result);
}

void set_PF(uint32_t result)
{
	uint32_t temp = result & (0xFFFFFFFF >> 24);
	int count=0;
	for(int i=0; i<8; i++)
	{
		count += temp & 0x1;
		temp >>= 1;
	}
	if(count%2 == 0)
		cpu.eflags.PF = 1;
	else
		cpu.eflags.PF = 0;
}

void set_OF_add(uint32_t result, uint32_t src, uint32_t dest, size_t data_size) 
{ 
	switch(data_size) 
	{
		case 8:
			result = sign_ext(result & 0xFF, 8);
			src = sign_ext(src & 0xFF, 8); dest = sign_ext(dest & 0xFF, 8); 
			break;
		case 16:
			result = sign_ext(result & 0xFFFF, 16);
			src = sign_ext(src & 0xFFFF, 16); 
			dest = sign_ext(dest & 0xFFFF, 16); 
			break;
		default: break;// do nothing
	}
	if(sign(src) == sign(dest)) 
	{
		if(sign(src) != sign(result)) 
			cpu.eflags.OF = 1;
	else
		cpu.eflags.OF = 0;
	} 
	else
		cpu.eflags.OF = 0;
}

void set_OF_sub(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)
{
	switch(data_size)
	{
		case 8:
			result = sign_ext(result & 0xFF, 8);
			src = sign_ext(src & 0xFF, 8);
			dest = sign_ext(dest & 0xFF, 8);
			break;
											
		case 16:
			result = sign_ext(result & 0xFFFF, 16);
			src = sign_ext(src & 0xFFFF, 16); 
			dest = sign_ext(dest & 0xFFFF, 16);
			break;
			default: break;// do nothing
	}
	if(sign(src) != sign(dest))
	{
		if(sign(dest) != sign(result))
			cpu.eflags.OF = 1;
		else
			cpu.eflags.OF = 0;
	}
	else
		cpu.eflags.OF = 0;
}

void set_OF_mul(uint64_t result,size_t data_size)
{
	result=result & (0xFFFFFFFFFFFFFFFF>>(64-(data_size*2)));
	if(data_size==8)
		result=result & 0xFFFFFFFFFFFFFF00;
	else if(data_size==16)
		result=result & 0xFFFFFFFFFFFF0000;
	else
		result=result & 0xFFFFFFFF00000000;
	if(result!=0)
	{
		cpu.eflags.OF=1;
		cpu.eflags.CF=1;
	}	
	else
	{
		cpu.eflags.OF=0;
		cpu.eflags.CF=0;
	}
}

void set_OF_imul(int64_t result,size_t data_size)
{
	result=result & (0xFFFFFFFFFFFFFFFF>>(64-(data_size*2)));
	uint64_t aim=(uint64_t)result >> (data_size-1);
	uint64_t newres=(uint64_t)result >> data_size;
	bool equal=true;
	if(aim==0x0)
	{
		for(int i=0;i<data_size;i++)
	    {
			if((newres & 0x1)==0x1)
			{
				equal=false;
				break;
			}
			newres >>= 1;
		}
	}
	else
	{
		for(int i=0;i<data_size;i++)
	    {
			if((newres & 0x0)==0x1)
			{
				equal=false;
				break;
			}
			newres >>= 1;
		}
	}
	if(equal)
		cpu.eflags.OF=1;
	else
		cpu.eflags.OF=0;
}

//***************************************************************

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
	uint32_t res=0;
	res=dest+src;
	set_CF_add(res,src,data_size);
	set_PF(res);
	set_ZF(res,data_size);
	set_SF(res,data_size);
	set_OF_add(res,src,dest,data_size);
	return res&(0xFFFFFFFF>>(32-data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
	uint32_t res=src+dest+cpu.eflags.CF;
	set_CF_adc(res,src,data_size);
	set_PF(res);
	set_ZF(res,data_size);
	set_SF(res,data_size);
	set_OF_add(res,src,dest,data_size);
	return res&(0xFFFFFFFF>>(32-data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}


uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
	uint32_t res=dest-src;
	set_CF_sub(dest, src, data_size);
	set_PF(res);
	set_ZF(res,data_size);
	set_SF(res,data_size);
	set_OF_sub(res,src,dest,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
	uint32_t res=dest-src-cpu.eflags.CF;
	set_CF_sbb(dest,src,data_size);
	set_PF(res);
	set_ZF(res,data_size);
	set_SF(res,data_size);
	set_OF_sub(res,src,dest,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}


uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
	uint64_t a=((uint64_t)src << (64-data_size)) >> (64-data_size);
	uint64_t b=((uint64_t)dest << (64-data_size)) >> (64-data_size);
	uint64_t res=a*b;
	set_OF_mul(res,data_size);
	return res&(0xFFFFFFFFFFFFFFFF>>(64-(data_size*2)));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
	int64_t a=((int64_t)src << (64-data_size)) >> (64-data_size);
	int64_t b=((int64_t)dest << (64-data_size)) >> (64-data_size);
	int64_t res=a*b;
	set_OF_imul(res,data_size);
	return res;
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
	uint64_t a=src << (64-data_size) >> (64-data_size);
	uint64_t b=dest << (64-data_size) >> (64-data_size);
	assert(a!=0);
	uint64_t res=b/a;
	uint64_t overflow=res & 0xFFFFFFFF00000000;
	assert(overflow==0);
	return (uint32_t)res;
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
	assert(src!=0);
	int64_t res=dest/src;
	return (int32_t)res;
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest) {
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
	assert(src!=0);
	uint64_t res=dest%src;
	return (uint32_t)res;
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

int32_t alu_imod(int64_t src, int64_t dest) {
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
	assert(src!=0);
	int64_t res=dest%src;
	return (int32_t)res;
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
	uint32_t res = src & dest;
	cpu.eflags.CF=0;
	cpu.eflags.OF=0;
	set_PF(res);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
	uint32_t res = src ^ dest;
	cpu.eflags.CF=0;
	cpu.eflags.OF=0;
	set_PF(res);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
	uint32_t res = src | dest;
	cpu.eflags.CF=0;
	cpu.eflags.OF=0;
	set_PF(res);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
	src=src << (32-data_size) >> (32-data_size);
	dest=dest << (32-data_size) >> (32-data_size);
	uint32_t res=dest;
	uint32_t k=src;
	while(k!=0)
	{
		cpu.eflags.CF=res >> (data_size-1);
		res=res*2;
		k=k-1;
	}
	if(src==1)
	{
		if((res >> (data_size-1))!=cpu.eflags.CF)
			cpu.eflags.OF=1;
		else
			cpu.eflags.OF=0;
	}
	set_PF(res);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
	src=src << (32-data_size) >> (32-data_size);
	dest=dest << (32-data_size) >> (32-data_size);
	uint32_t res=dest;
	while(src!=0)
	{
		cpu.eflags.CF=res & 0x00000001;
		res=res/2;
		src=src-1;
	}
	if(src==1)
		cpu.eflags.OF=dest >> (32-data_size);
	set_PF(res);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);	
#else
	src=src << (32-data_size) >> (32-data_size);
	uint32_t res=(int32_t)(dest << (32-data_size)) >> (32-data_size);
	while(src!=0)
	{
		cpu.eflags.CF=res & 0x00000001;
		res=(int32_t)res >> 1;
		src=src-1;
	}
	if(src==1)
		cpu.eflags.OF=0;
	set_PF(res);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size) {
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
	src=src << (32-data_size) >> (32-data_size);
	dest=dest << (32-data_size) >> (32-data_size);
	uint32_t res=dest;
	uint32_t k=src;
	while(k!=0)
	{
		cpu.eflags.CF=res >> (data_size-1);
		res=res*2;
		k=k-1;
	}
	if(src==1)
	{
		if((res >> (data_size-1))!=cpu.eflags.CF)
			cpu.eflags.OF=1;
		else
			cpu.eflags.OF=0;
	}
	set_PF(res);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
	/*printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	assert(0);
	return 0;*/
#endif
}
