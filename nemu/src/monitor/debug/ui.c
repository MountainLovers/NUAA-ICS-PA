#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  
	uint64_t N = 0;
  char *arg = strtok(NULL, " ");

	if (arg == NULL) N = 1;
  else 
	{
		int len = strlen(arg);
		if (arg[0] == '-')
		{
			if (len == 2 && arg[1] == '1')
			{
				cpu_exec(-1);
				return 0;
			}
			printf("Invalid Argument\n");	
			return 0;
		}
    for (int p=0;p<len;p++)
		{
			if (arg[p]<'0' || arg[p]>'9')
			{
				printf("Please enter 1~9\n");
				return 0;
			}
			N = N*10+(arg[p]-'0');
		}
  }
	if (!N) N = 1;

  cpu_exec(N);
  return 0;
}

static int cmd_info(char *args){
	char *arg = strtok(NULL, " ");

  if (arg == NULL) 
	{
		printf("Please enter the arguments('r' or 'w')\n");
		return 0;
	}
	if (arg[0] == 'r')
	{
		printf("Reg		HEX     		DEC\n");
		for (int i=R_EAX;i<=R_EDI;i++) printf("%s		0x%06x		%d\n",regsl[i],reg_l(i),reg_l(i));
//		for (int i=R_AX;i<=R_DI;i++) printf("%s		0x%04x\n",regsw[i],reg_w(i));
//		for (int i=R_AL;i<=R_BH;i++) printf("%s		0x%02x\n",regsb[i],reg_b(i));
	}
//	else
//		if (args[0] == 'w')
//  	{
//  	}
  return 0;
}


static int cmd_x(char *args){
	char *arg = strtok(NULL, " ");
 	if (arg == NULL) 
	{
		printf("There are no arguments!\n");
		return 0;
	}
	int N = 0;
	N = atoi(arg);
	if (N <= 0)
	{
		printf("Please check arguments\n");
		return 0;
	}
		
	arg = strtok(NULL, " ");
  uint32_t addr;
	uint8_t flag = true;
	/*	if (!sscanf(arg, "0x%x", &addr))
	{
		printf("Please check arguments\n");
		return 0;
	}
*/

	addr = expr(arg, &flag);
	if (flag == false) {printf("Expression is invalid.\n"); assert(0);}
  
	printf("Address     Little-Endian     Big-Endian\n");
	int i;
	for (i=1;i<=N;i++)
	{
		uint32_t value = vaddr_read(addr, 4);
		printf("0x%x    ", addr);
		printf("%02x  ", value & 0x000000FF);
		printf("%02x  ", (value >> 8) & 0x000000FF);
		printf("%02x  ", (value >> 16) & 0x000000FF);
		printf("%02x    ", (value >> 24)& 0x000000FF);
		printf("0x%08x\n", value);
		addr += 4;
	}
//	printf("%d %s\n",N,arg);
	return 0;
}

static int cmd_p(char *args) {
	uint8_t flag = true;
	uint32_t v = expr(args, &flag);
	if (!flag) printf("make_token failed!\n");
	else printf("result = %d\n", v);
	return 0;
}

static int cmd_w(char *args) {
	WP *newwp = new_wp();
//	newwp->oldvalue = expr(args, &flag);
//	if (!flag) printf("make_token failed!\n");
	int len = strlen(args);
	if (len >= 50) {printf("The length of args is too long to store!\n"); assert(0);}
	strncpy(newwp->expression, args, len);
	newwp->expression[len]='\0';
	printf("Watchpoint: NO.%d  expression:%s\n", newwp->NO, newwp->expression);
	return 0;
}

extern bool delete_wp(int);
static int cmd_d(char *args) {	
  char *arg = strtok(NULL, " ");
	int n = -1;
	if (sscanf(arg, "%d", &n) == -1) {printf("Check argument!\n"); assert(0);}
	if (delete_wp(n) == false) {printf("Delete WP failed!\n"); assert(0);}
	return 0;
}	

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "'si [N]': Execute the program for N steps and then stop. The default value of N is 1", cmd_si },
	{"info", "r: Print the infomation of states of registers. w: Print the information of states of watchpoints", cmd_info },
	{"x", "'x N EXPR' means print value of address from EXPR lasting N*4 Bytes", cmd_x },
	{"p", "'p EXPR' calculate the value of expression", cmd_p },
	{"w", "'w EXPR' set a watchpoint that program will stop when the value of expression change", cmd_w },
	{"d", "'d N' delete the wp whose NO is N", cmd_d },
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
