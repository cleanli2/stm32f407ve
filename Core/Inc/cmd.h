#ifndef _CMD_H
#define _CMD_H

#define con_send __io_putchar
#define con_recv __io_getchar
#define con_is_recved() __io_char_received()

#define COM_MAX_LEN 81
#define CMD_CACHES_SIZE 20
#define ENTER_CHAR 0x0d
#ifndef NULL
#define NULL 0
#endif

struct command{
     char * cmd_name;
     void (*cmd_fun)(char *);
};

void go(char *p);
void print_mem(char *p);
void print_help(char *para);
void read_mem(char *para);
void write_mem(char *para);
void run_cmd_interface();

#endif
