#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

// implement headfile
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "monitor/elf.h"
// end

// struct type op to find the dominate oprator
// implement as follows
typedef struct
{
	int place;
	int priority;
}dominateOperand;
// end

enum {
	NOTYPE = 256, EQ, HEXNUM, NUM, REG, SYMB

	/* TODO: Add more token types */
	, NEQ, AND, OR, POT, NEG, NOT, ZERO
};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// white space
	{"\\+", '+'},
	{"\\-", '-'},
	{"==", EQ},
	{"0[xX][0-9a-fA-F]+", HEXNUM},  // hex number
	{"[1-9][0-9]*", NUM},
	{"0", ZERO},
	{"\\$e[a,c,d,b]x", REG},
	{"\\$e[s,b]p", REG},
	{"\\$e[s,d]i", REG},
	{"[a-zA-Z_]+[a-zA-Z0-9_]*", SYMB},
	{"\\*", '*'},
	{"\\/", '/'},
	{"\\(", '('},
	{"\\)", ')'},
	{"!=", NEQ},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", NOT},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */

				if(substr_len>32)
				{
					printf("the data size has exceeded 32 bits to %d bits.", substr_len);
					return false;
				}
				for(int i=0;i<substr_len;i++)
					tokens[nr_token].str[i]=*(substr_start+i);
				tokens[nr_token].str[substr_len]='\0';

				//************************************************************************

				switch(rules[i].token_type) {
					default: tokens[nr_token].type = rules[i].token_type;
							 nr_token ++;
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

// func:check_parentheses
bool check_parentheses(int p, int q)
{
	if(tokens[p].type!='('||tokens[q].type!=')')
		return false;
	int left=0,right=0;
	for(int i=p+1;i<q;i++)
	{
		if(tokens[i].type=='(')
			left++;
		else if(tokens[i].type==')')
			right++;
		if(left<right)
			return false;
	}
	if(left!=right)
		return false;
	return true;
}
// end

// func:eval
// implement as follows
uint32_t eval(int p, int q, bool *success)
{
	if(*success==false)
		return -1;
	if(p>q)
	{
		printf("there is a syntax error around the point: %d\n", p);
		*success=false;
		return -1;
	}
	else if(p==q)
	{
		uint32_t num = 0;
		if(tokens[p].type==ZERO){}
		else if(tokens[p].type==NUM)
			num=atoi(tokens[p].str);
		else if(tokens[p].type==HEXNUM)
		{
			int i=strlen(tokens[p].str);
			if(i<3)
			{
				printf("syntax error: the hexnumber at point: %d lacks contents\n", p);
				*success=false;
				return -1;
			}
			for(int k=2;k<i;k++)
			{
				if(tokens[p].str[k]>='0'&&tokens[p].str[k]<='9')
				{
					uint32_t tempnum=1;
					for(int t=0;t<i-1-k;t++)
						tempnum=tempnum*16;
					tempnum*=tokens[p].str[k]-'0';
					num+=tempnum;
				}
				else if(tokens[p].str[k]>='a'&&tokens[p].str[k]<='f')
				{
					uint32_t tempnum=1;
					for(int t=0;t<i-1-k;t++)
						tempnum=tempnum*16;
					tempnum*=tokens[p].str[k]-'a'+10;
					num+=tempnum;
				}
				else if(tokens[p].str[k]>='A'&&tokens[p].str[k]<='F')
				{
					uint32_t tempnum=1;
					for(int t=0;t<i-1-k;t++)
						tempnum=tempnum*16;
					tempnum*=tokens[p].str[k]-'A'+10;
					num+=tempnum;
				}
			}
		}
		else if(tokens[p].type==REG)
		{
			if(tokens[p].str[2]=='a')
				num=cpu.eax;
			else if(tokens[p].str[2]=='b'&&tokens[p].str[3]=='x')
				num=cpu.ebx;
			else if(tokens[p].str[2]=='b'&&tokens[p].str[3]=='p')
				num=cpu.ebp;
			else if(tokens[p].str[2]=='c')
				num=cpu.ecx;
			else if(tokens[p].str[2]=='d'&&tokens[p].str[3]=='x')
				num=cpu.edx;
			else if(tokens[p].str[2]=='s'&&tokens[p].str[3]=='p')
				num=cpu.esp;
			else if(tokens[p].str[2]=='s'&&tokens[p].str[3]=='i')
				num=cpu.esi;
			else if(tokens[p].str[2]=='d'&&tokens[p].str[3]=='i')
				num=cpu.edi;
		}
		else if(tokens[p].type==SYMB)
		{
			num = look_up_symtab(&tokens[p].str[0], success);
			if(*success==false)
			{
				printf("there is a syntax error at point: %d, cannot match the symbol name\n", p);
				num=-1;
			}
		}
		else
		{
			printf("there is a syntax error at point: %d\n", p);
			*success=false;
			num=-1;
		}
		return num;
	}
	else if(check_parentheses(p,q)==true)
	{
		return eval(p+1, q-1, success);
	}
	else
	{
		int left=0,right=0;
		dominateOperand dominate_op[32];
		for(int i=0;i<32;i++)
		{
			dominate_op[i].place=-1;
			dominate_op[i].priority=7;
		}
		int t=0;
		for(int i=p;i<=q;i++)
		{
			if(tokens[i].type=='(')
				left++;
			else if(tokens[i].type==')')
				right++;
			else
			{
				if(left==right)
				{
					if(tokens[i].type==OR)
					{
						dominate_op[t].place=i;
						dominate_op[t].priority=1;
						t++;
					}
					else if(tokens[i].type==AND)
					{
						dominate_op[t].place=i;
						dominate_op[t].priority=2;
						t++;
					}
					if(tokens[i].type==EQ||tokens[i].type==NEQ)
					{
						dominate_op[t].place=i;
						dominate_op[t].priority=3;
						t++;
					}
					else if(tokens[i].type=='+'||tokens[i].type=='-')
					{
						dominate_op[t].place=i;
						dominate_op[t].priority=4;
						t++;
					}
					else if(tokens[i].type=='*'||tokens[i].type=='/')
					{
						dominate_op[t].place=i;
						dominate_op[t].priority=5;
						t++;
					}
					else if(tokens[i].type==NEG||tokens[i].type==POT||tokens[i].type==NOT)
					{
						dominate_op[t].place=i;
						dominate_op[t].priority=6;
						t++;
					}
				}
			}
		}
		if(left!=right)
		{
			*success=false;
			printf("syntax error: parentheses not matched\n");
			return -1;
		}
		if(dominate_op[0].place==-1)
		{
			*success=false;
			printf("syntax error: no operands between %d and %d\n", p, q);
			return -1;
		}
		int min=dominate_op[0].priority;
		int op=dominate_op[0].place;
		for(int i=1;i<t;i++)
		{
			if(dominate_op[i].priority<=min)
			{
				min=dominate_op[i].priority;
				op=dominate_op[i].place;
			}
		}
		if(tokens[op].type==NEG)
		{
			uint32_t val=eval(op+1, q, success);
			return -val;
		}
		else if(tokens[op].type==POT)
		{
			uint32_t val=eval(op+1, q, success);
			if(val>=128*1024*1024||val<0)
			{
				printf("the address must be a valid number between 0x0(0) and 0x%x(%d)\n", 128*1024*1024, 128*1024*1024);
				*success=false;
				return -1;
			}
			return vaddr_read((vaddr_t)val, 0, 4);
		}
		else if(tokens[op].type==NOT)
		{
			uint32_t val=eval(op+1, q, success);
			return !val;
		}
		uint32_t val1=eval(p, op-1, success);
		uint32_t val2=eval(op+1, q, success);
		switch(tokens[op].type)
		{
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': {
				if(val2==0)
				{
					printf("the division opration has a math error\n");
					*success = false;
					return -1;
				}
				else
					return val1 / val2;
			}
			case EQ: return val1 == val2;
			case NEQ: return val1 != val2;
			case AND: return val1 && val2;
			case OR: return val1 || val2;
			default: {
				*success = false;
				return -1;
			}
		}
	}
}
//end

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	//printf("\nPlease implement expr at expr.c\n");
	//assert(0);
	// implement as follows

	for(int i = 0; i < nr_token-1; i++) 
	{
		if(tokens[i].type == '*') 
		{
			bool is = false;
			if(i==0)
				is = true;
			else if(tokens[i+1].type=='('||tokens[i+1].type==NUM||tokens[i+1].type==HEXNUM||tokens[i+1].type==SYMB)
			{
				if(tokens[i-1].type=='+')
					is = true;
				else if(tokens[i-1].type=='-')
					is = true;
				else if(tokens[i-1].type=='*')
					is = true;
				else if(tokens[i-1].type=='/')
					is = true;
				else if(tokens[i-1].type=='(')
					is = true;
				else if(tokens[i-1].type==NEG)
					is = true;
				else if(tokens[i-1].type==AND)
					is = true;
				else if(tokens[i-1].type==OR)
					is = true;
				else if(tokens[i-1].type==NOT)
					is = true;
				else if(tokens[i-1].type==EQ)
					is = true;
				else if(tokens[i-1].type==NEQ)
					is = true;
			}
			if(is)
				tokens[i].type = POT; // this type is a pointer
		}
		else if(tokens[i].type == '-')
		{
			bool is = false;
			if(i==0)
				is = true;
			else if(tokens[i+1].type=='('||tokens[i+1].type==NUM||tokens[i+1].type==HEXNUM||tokens[i+1].type==SYMB)
			{
				if(tokens[i-1].type=='+')
					is = true;
				else if(tokens[i-1].type=='-')
					is = true;
				else if(tokens[i-1].type=='*')
					is = true;
				else if(tokens[i-1].type=='/')
					is = true;
				else if(tokens[i-1].type=='(')
					is = true;
				else if(tokens[i-1].type==AND)
					is = true;
				else if(tokens[i-1].type==OR)
					is = true;
				else if(tokens[i-1].type==NOT)
					is = true;
				else if(tokens[i-1].type==EQ)
					is = true;
				else if(tokens[i-1].type==NEQ)
					is = true;
			}
			if(is)
				tokens[i].type = NEG; // this type is a negative
		}
	}
	*success=true;
	return eval(0, nr_token-1, success);
}

