#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC, TK_HEX, TK_REGU, TK_REG, TK_VAR, TK_MIMI, TK_PLPL, TK_UEQ, TK_AND, TK_OR, TK_NOT, TK_TANHAO

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
	{"\\*\\$((e?(ax|bx|cx|dx|bp|si|di|sp))|([a-d][l,h]))", TK_REGU},		// Reg Regex use
	{"\\$((e?(ax|bx|cx|dx|bp|si|di|sp))|([a-d][l,h]))", TK_REG},		// Reg Regex
	//TODO:variable such as "len"
	{"[a-zA-Z_][a-zA-Z0-9_]*", TK_VAR},							// variable such as "len"
	{"\\(", '('},				// left parenthese
	{"\\)", ')'},				// right parenthese
	{"--", TK_MIMI},		// unary operator --
	{"\\+\\+", TK_PLPL},		// unary operator ++
  {"==", TK_EQ},        // equal
//	{"!=", TK_UEQ},				// unequal
	{"!", TK_TANHAO},				// tanhao
	{"&&", TK_AND},				// and
	{"\\|\\|", TK_OR},		// or
//	{"![^=]", TK_NOT},		// not
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
								case TK_HEX: {tokens[nr_token].type = TK_HEX; strncpy(tokens[nr_token++].str,substr_start,substr_len); break;}
								case TK_DEC: {tokens[nr_token].type = TK_DEC; strncpy(tokens[nr_token++].str,substr_start,substr_len); break;}
								case TK_REGU: {tokens[nr_token].type = TK_REGU; strncpy(tokens[nr_token++].str,substr_start,substr_len); break;}
								case TK_REG: {tokens[nr_token].type = TK_REG; strncpy(tokens[nr_token++].str,substr_start,substr_len); break;}
								case TK_VAR: {tokens[nr_token].type = TK_VAR; strncpy(tokens[nr_token++].str,substr_start,substr_len); break;}
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

uint32_t eval(int p, int q) {
	if (p > q) {
		printf("Bad expression!\n");
		assert(0);
	}
	else if (p == q) {
		// DEC or HEX
		uint32_t v;
		if (tokens[p].type == TK_DEC) {sscanf(tokens[p].str, "%d", &v); return v;}
		if (tokens[p].type == TK_HEX) {sscanf(tokens[p].str, "%x", &v); return v;}
		assert(0);
	}
	else if (check_parentheses(p, q) == true) {
		return eval(p+1, q-1);
	}
	else {
		int opt_level[300];
//		opt_level[TK_NOT] = 120;
		opt_level['+'] = 100;
		opt_level['-'] = 100;
		opt_level['*'] = 110;
		opt_level['/'] = 110;
		opt_level['%'] = 110;
		opt_level[TK_EQ] = 90;
		opt_level[TK_DEC] = 10000;
		opt_level[TK_HEX] = 10000;
//		opt_level[TK_UEQ] = 90;
		opt_level[TK_AND] = 80;
		opt_level[TK_OR] = 70;
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

		uint32_t val1 = eval(p, lowest_pos-1);
		uint32_t val2 = eval(lowest_pos+1, q);
		switch (tokens[lowest_pos].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case '%': return val1 % val2;
			case TK_EQ: return val1 == val2;
//			case TK_UEQ: return val1 != val2;
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
	printf("%d\n",eval(0, nr_token-1));
  /* TODO: Insert codes to evaluate the expression. */

  return 0;
}
