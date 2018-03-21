#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC, TK_HEX, TK_REGU, TK_REG, TK_VAR, TK_MIMI, TK_PLPL, TK_UEQ, TK_AND, TK_OR, TK_NOT

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
	{"!=", TK_UEQ},				// unequal
	{"&&", TK_AND},				// and
	{"\\|\\|", TK_OR},		// or
	{"![^=]", TK_NOT},		// not
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

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
								case TK_HEX: {tokens[i].type = TK_HEX; strncpy(tokens[i].str,substr_start,substr_len); break;}
								case TK_DEC: {tokens[i].type = TK_DEC; strncpy(tokens[i].str,substr_start,substr_len); break;}
								case TK_REGU: {tokens[i].type = TK_REGU; strncpy(tokens[i].str,substr_start,substr_len); break;}
								case TK_REG: {tokens[i].type = TK_REG; strncpy(tokens[i].str,substr_start,substr_len); break;}
								case TK_VAR: {tokens[i].type = TK_VAR; strncpy(tokens[i].str,substr_start,substr_len); break;}
								case '(': {tokens[i].type = '('; break;}
								case ')': {tokens[i].type = ')'; break;}
								case TK_MIMI: {tokens[i].type = TK_MIMI; break;}
								case TK_PLPL: {tokens[i].type = TK_PLPL; break;}
								case TK_EQ: {tokens[i].type = TK_EQ; break;}
								case TK_UEQ: {tokens[i].type = TK_UEQ; break;}
								case TK_AND: {tokens[i].type = TK_AND; break;}	 
								case TK_OR: {tokens[i].type = TK_OR; break;}
								case TK_NOT: {tokens[i].type = TK_NOT; break;}
								case '+': {tokens[i].type = '+'; break;}
								case '-': {tokens[i].type = '-'; break;}
								case '*': {tokens[i].type = '*'; break;}
								case '/': {tokens[i].type = '/'; break;}
								case '%': {tokens[i].type = '%'; break;}
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

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
//  TODO();

  return 0;
}
