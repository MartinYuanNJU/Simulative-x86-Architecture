#include "cpu/instr.h"

make_instr_func(call_near) {
	OPERAND rel, pushnum;
    rel.type = OPR_IMM;
	rel.sreg = SREG_CS;
    rel.data_size = data_size;
    rel.addr = eip + 1;
	operand_read(&rel);
	int offset = sign_ext(rel.val, data_size);
	print_asm_1("call", "", 2, &rel);
	pushnum.type = OPR_MEM;
	cpu.esp-=4;
	pushnum.addr=cpu.esp;
	pushnum.sreg=SREG_SS;
	pushnum.data_size=32;
	pushnum.val=cpu.eip+1+data_size/8;
	operand_write(&pushnum);
	cpu.eip += offset;
    return 1 + data_size/8;
}

make_instr_func(call_near_indirect) {
    int len=1;
    decode_data_size_near
    decode_operand_rm
	opr_src.sreg=SREG_CS;
    operand_read(&opr_src);
    int offset = sign_ext(opr_src.val, opr_src.data_size);
    print_asm_1("call", "", 2, &opr_src);
    OPERAND pushnum;
    pushnum.type = OPR_MEM;
    cpu.esp-=4;
    pushnum.addr=cpu.esp;
    pushnum.data_size=32;
	pushnum.sreg=SREG_SS;
	pushnum.val=cpu.eip+len;
	operand_write(&pushnum);
	cpu.eip = offset;
    return 0;
}

