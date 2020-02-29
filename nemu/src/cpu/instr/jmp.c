#include "cpu/instr.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/mmu/segment.h"

make_instr_func(jmp_near) {
    OPERAND rel;
    rel.type = OPR_IMM;
	rel.sreg = SREG_CS;
    rel.data_size = data_size;
    rel.addr = eip + 1;
    operand_read(&rel);
	int offset = sign_ext(rel.val, data_size);
	print_asm_1("jmp", "", 2, &rel);
	cpu.eip += offset;
    return 1 + data_size / 8;
}

make_instr_func(jmp_short) {
    OPERAND rel;
    rel.type = OPR_IMM;
	rel.sreg = SREG_CS;
    rel.data_size = 8;
    rel.addr = eip + 1;
    operand_read(&rel);
	int offset = sign_ext(rel.val, 8);
	print_asm_1("jmp", "", 2, &rel);
	cpu.eip += offset;
    return 2;
}

make_instr_func(jmp_near_indirect) {
	int len=1;
    decode_data_size_near
	decode_operand_rm
	opr_src.sreg=SREG_CS;
	operand_read(&opr_src);
	int offset = sign_ext(opr_src.val, opr_src.data_size);
	print_asm_1("jmp", "", 2, &opr_src);
	cpu.eip = offset;
    return 0;
}

make_instr_func(jmp_far_imm) {
	OPERAND address,sregcontents;
	address.type=OPR_IMM;
	address.data_size=32;
	address.addr=eip+1;
	operand_read(&address);
	sregcontents.type=OPR_IMM;
	sregcontents.data_size=16;
	sregcontents.addr=eip+5;
	operand_read(&sregcontents);
	cpu.segReg[1].val=sregcontents.val;
	print_asm_1("ljmp", "", 7, &address);
	cpu.eip=address.val;
	load_sreg(1);
    return 0;
}
