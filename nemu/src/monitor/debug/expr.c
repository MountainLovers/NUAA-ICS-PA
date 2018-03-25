#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC, TK_HEX, TK_REG, TK_VAR, TK_MIMI, TK_PLPL, TK_UEQ, TK_AND, TK_OR, TK_NOT, TK_TANHAO, TK_JYY, TK_FS

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
	{"0x[a-fA-f0-9]{1,8}", TK_HEX},		// HEX number
	{"[0-9]{1,10}", TK_DEC},			// DEC number
	{"\\$((e?(ax|bx|cx|dx|bp|si|di|sp))|([a-d][l,h])|eip)", TK_REG},		// Reg Regex
	//TODO:variable such as "len"
	{"[a-zA-Z_][a-zA-Z0-9_]*", TK_VAR},							// variable such as "len"
	{"\\(", '('},				// left parenthese
	{"\\)", ')'},				// right parenthese
	{"--", TK_MIMI},		// unary operator --
	{"\\+\\+", TK_PLPL},		// unary operator ++
  {"==", TK_EQ},        // equal
	{"!=", TK_UEQ},				// unequal
//	{"!", TK_TANHAO},				// tanhao
	{"&&", TK_AND},				// and
	{"\\|\\|", TK_OR},		// or
	{"!", TK_NOT},		// not
	{"\\+", '+'},         // plus
	{"-", '-'},					// minus
	{"\\*", '*'},					// multiply
	{"/", '/'},					// divided
	{"%", '%'},					// mod
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

bool str_overflow(char *e, int *ss, int *len, int type) {
	int s = *ss, t = (*ss)+(*len)-1;
	if (type == TK_REG || type == TK_VAR) {
		return false;
	}
	if (type == TK_DEC) {
		while (s < t-1 && e[s] == '0') s++; 
		if (t-s+1 <= 31) {
			*ss=s; *len=t-s+1;
			return true;
		}
		else {
			return false;
		}
	}
	if (type == TK_HEX) {
		s+=2;
		while (s < t-1 && e[s] == '0') s++;
		if (t-s+1 <=29) {
			*ss=s; *len=t-s+1;
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}	

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s----%d",
            i, rules[i].regex, position, substr_len, substr_len, substr_start,rules[i].token_type);
        position += substr_len;
			
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
								case TK_HEX: {
									tokens[nr_token].type = TK_HEX;
								 	if (substr_len > 31) {
										int ss = position-substr_len, len=substr_len;
										if (str_overflow(e,&ss,&len,TK_HEX) == true) {
											substr_start = e+ss;
											tokens[nr_token].str[0]='0'; tokens[nr_token].str[1]='x';
											strncpy(tokens[nr_token++].str+2,substr_start,len);
											tokens[nr_token-1].str[len+2]='\0';
										}
										else {
											assert(0);
										}
									}
									else {
										strncpy(tokens[nr_token++].str,substr_start,substr_len);
										tokens[nr_token-1].str[substr_len]='\0'; 
									}
									break;
								}
								case TK_DEC: {
									tokens[nr_token].type = TK_DEC; 
									if (substr_len > 31) {
										int ss = position-substr_len, len=substr_len;
										if (str_overflow(e,&ss,&len,TK_DEC) == true) {
											substr_start = e+ss;
											strncpy(tokens[nr_token++].str,substr_start,len);
											tokens[nr_token-1].str[len]='\0';
										}
										else {
											assert(0);
										}
									}
									else {
										strncpy(tokens[nr_token++].str,substr_start,substr_len);
										tokens[nr_token-1].str[substr_len]='\0';
									}	 
									break;
								}
								case TK_REG: {tokens[nr_token].type = TK_REG; if (substr_len > 31) assert(0); strncpy(tokens[nr_token++].str,substr_start,substr_len); tokens[nr_token-1].str[substr_len]='\0'; break;}
								case TK_VAR: {tokens[nr_token].type = TK_VAR; if (substr_len > 31) assert(0); strncpy(tokens[nr_token++].str,substr_start,substr_len); tokens[nr_token-1].str[substr_len]='\0'; break;}
								case '(': {tokens[nr_token++].type = '('; break;}
								case ')': {tokens[nr_token++].type = ')'; break;}
								case TK_MIMI: {tokens[nr_token++].type = TK_MIMI; break;}
								case TK_PLPL: {tokens[nr_token++].type = TK_PLPL; break;}
								case TK_EQ: {tokens[nr_token++].type = TK_EQ; break;}
								case TK_UEQ: {tokens[nr_token++].type = TK_UEQ; break;}
								case TK_AND: {tokens[nr_token++].type = TK_AND; break;}	 
								case TK_OR: {tokens[nr_token++].type = TK_OR; break;}
								case TK_NOT: {tokens[nr_token++].type = TK_NOT; break;}
								case '+': {tokens[nr_token++].type = '+'; break;}
								case '-': {tokens[nr_token++].type = '-'; break;}
								case '*': {tokens[nr_token++].type = '*'; break;}
								case '/': {tokens[nr_token++].type = '/'; break;}
								case '%': {tokens[nr_token++].type = '%'; break;}
//          default: TODO();
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

//	for (i=0;i<nr_token;i++) printf("tokens[%d].type=%d\n",i,tokens[i].type);

  return true;
}

bool check_parentheses(int p, int q) {
	int i;
	if ((tokens[p].type != '(') || (tokens[q].type != ')')) return false;
	int now_level = 0;
	for (i = p+1; i <= q-1; i++){
		if (tokens[i].type == '(') now_level++;
		if (tokens[i].type == ')') now_level--;
		if (now_level < 0) return false;
	}
	if (now_level != 0) return false;
	return true;
}

uint32_t value(int p, int q);

uint32_t eval(int p, int q) {
	if (p > q) {
		printf("Bad expression!\n");
		assert(0);
	}
	else {
		switch (tokens[p].type) {
			case TK_JYY: {int v=value(p+1, q); return vaddr_read(v, 4);}
			case TK_FS: {int v=value(p+1, q); return -v;}
			case TK_MIMI: {int v=value(p+1, q); return --v;}
			case TK_PLPL: {int v=value(p+1, q); return ++v;}	
			case TK_NOT: {int v=value(p+1, q); return !v;}
			default: assert(0);	
		}
	}	
}

// normal
uint32_t value(int p, int q) {
	if (p > q) {
		printf("Bad expression!\n");
		assert(0);
	}
	else if (p == q) {
		// DEC or HEX
		uint32_t v;
		if (tokens[p].type == TK_DEC) {sscanf(tokens[p].str, "%d", &v); return v;}
		if (tokens[p].type == TK_HEX) {sscanf(tokens[p].str, "%x", &v); return v;}
	  if (tokens[p].type == TK_REG) {	
			char *reg32[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
		  char *reg16[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
	 		char *reg8[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};	 
			int l=strlen(tokens[p].str)-1;
			char str[32];
			strncpy(str,tokens[p].str+1,l);
			str[l]='\0';
			char *streip = "eip";
			if (strcmp(str, streip) == 0) return cpu.eip;
			int i,j=0;
			for (i=0;i<l;i++) if (str[i] >= 'A' && str[i] <= 'Z') str[i]+=32;
			if (l == 3 && !j) {
				for (i=0;i<8;i++) {
					if (strcmp(str,reg32[i]) == 0) {j=32; break;}
				}
			}
			if (l == 2 && !j) {
				for (i=0;i<8;i++) 
					if (strcmp(str,reg16[i]) == 0) {j=16; break;}
			}
			if (l == 2 && !j) {
				for (i=0;i<8;i++)
					if (strcmp(str,reg8[i]) == 0) {j=8; break;}
			}
			if (j == 32) return reg_l(i);
			if (j == 16) return reg_w(i);
			if (j == 8) return reg_b(i);
			assert(0);
		}
				
		assert(0);
	}
	else if (check_parentheses(p, q) == true) {
		return value(p+1, q-1);
	}
	else {
		int opt_level[300];
		opt_level[TK_NOT] = 120;
		opt_level['+'] = 100;
		opt_level['-'] = 100;
		opt_level['*'] = 110;
		opt_level['/'] = 110;
		opt_level['%'] = 110;
		opt_level[TK_EQ] = 90;
		opt_level[TK_DEC] = 10000;
		opt_level[TK_HEX] = 10000;
		opt_level[TK_UEQ] = 90;
		opt_level[TK_AND] = 80;
		opt_level[TK_OR] = 70;
		opt_level[TK_JYY] = 500;
		opt_level[TK_FS] = 510;
		int pp;
		int lowest_level = 1000, lowest_pos = -1, parenthese_flag = 0;
		for (pp=p;pp<=q;pp++) {
			if (tokens[pp].type == ')') {parenthese_flag--; continue;}
			if (tokens[pp].type == '(') {parenthese_flag++; continue;}
			if (parenthese_flag) continue;
			if (opt_level[tokens[pp].type] <= lowest_level) {
							lowest_level = opt_level[tokens[pp].type];
							lowest_pos = pp;
			}
		}
//		printf("lowest_pos=%d\n",lowest_pos);
		if (tokens[lowest_pos].type == TK_JYY || tokens[lowest_pos].type == TK_NOT || tokens[lowest_pos].type == TK_FS) {
			return eval(p, q);
		}

		uint32_t val1 = value(p, lowest_pos-1);
		uint32_t val2 = value(lowest_pos+1, q);
		switch (tokens[lowest_pos].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case '%': return val1 % val2;
			case TK_EQ: return val1 == val2;
			case TK_UEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
			case TK_OR: return val1 || val2;
			default: assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
	int i;
	for (i = 0; i < nr_token; i ++) {
		/*pointer explain usei *$eax*/
		if (tokens[i].type == '*' && (i == 0 || tokens[i-1].type == '+' || tokens[i-1].type == '-' || tokens[i-1].type == '*' || tokens[i-1].type == '/' || tokens[i-1].type == '%' || tokens[i-1].type == TK_MIMI || tokens[i-1].type == TK_PLPL || tokens[i-1].type == TK_EQ || tokens[i-1].type == TK_UEQ || tokens[i-1].type == TK_AND || tokens[i-1].type == TK_OR || tokens[i-1].type == TK_NOT || tokens[i-1].type == '('))
			tokens[i].type = TK_JYY;
		
		/*-1*/
		if (tokens[i].type == '-' && (i == 0 || tokens[i-1].type == '+' || tokens[i-1].type == '-' || tokens[i-1].type == '*' || tokens[i-1].type == '/' || tokens[i-1].type == '%' || tokens[i-1].type == TK_EQ || tokens[i-1].type == TK_UEQ || tokens[i-1].type == TK_AND || tokens[i-1].type == TK_OR || tokens[i-1].type == TK_NOT || tokens[i-1].type == '('))
			tokens[i].type = TK_FS;
//		printf("%d %d\n",i,tokens[i].type);
	}
//	printf("%d\n",value(0, nr_token-1));
  return value(0,nr_token-1);
}
