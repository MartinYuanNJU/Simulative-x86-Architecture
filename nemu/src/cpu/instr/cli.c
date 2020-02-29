#include "cpu/instr.h"
#include "cpu/reg.h"

make_instr_func(cli) {
	cpu.eflags.IF=0;
	print_asm_0("cli", "", 1);
	return 1;
}
