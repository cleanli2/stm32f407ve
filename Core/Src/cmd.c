#include "cmd.h"
#include "lprintf.h"
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#define uint uint32_t
#define lprint lprintf

static char cmd_buf[COM_MAX_LEN] = "";
static uint cmd_buf_p = COM_MAX_LEN;
static uint quit_cmd = 0;
extern uint32_t task_mask;
extern uint32_t logv;

char * str_to_hex(char *s, uint32_t * result);
void delay(int a);
uint get_howmany_para(char *s);
char * str_to_str(char *s, char**result);
uint32_t ci=0;

uint8_t cmd_caches[CMD_CACHES_SIZE][COM_MAX_LEN] = {0};
uint32_t cmdcache_index=0;
uint32_t review_cmd_his_index;
uint8_t read_buf[512];

extern const uint8_t ziku12[];
extern const uint8_t ziku[];
uint32_t get_ziku12_size();
uint32_t get_ziku_size();

char* lstrncpy(char*d, const char*s, unsigned int n)
{
    char*ret=d;
    while(n--){
        *d++=*s;
        if(*s++==0)return ret;
    }
    return ret;
}

int lstrcmp(const char *a,const char *b)
{
    while(1){
        if(*a!=*b++)return 1;
        if(*a++==0)return 0;
    }
}
void lmemset(char *d,unsigned char v,unsigned int n)
{
	while(n--)*d++=v;
}
char *lstrchr(const char *str, int c)
{
    while(1){
        if(*str==(char)c)return (char*)str;
        if(*str++==0)return NULL;
    }
}

void cmd_exit(char *p)
{
    (void)p;
    lprintf("Quit CMD!\r\n");
    quit_cmd = 1;

    return;

}
void reboot(char *p)
{
    (void)p;
    NVIC_SystemReset();

    return;

}

void history(char *p)
{
    (void)p;
    uint32_t n = CMD_CACHES_SIZE, cix = cmdcache_index;
    lprintf("\r\n");
    while(n--){
        if(cix==0){
            cix = CMD_CACHES_SIZE-1;
        }
        else{
            cix--;
        }
        if(cmd_caches[cix][0]!=0){
            lprintf("%s\r\n", cmd_caches[cix]);
        }
    }
    lprintf("\r\n");

    return;
}

void start_dma_uart_recv();
int dma_uart_recved();
extern const char*duf;
void test(char*p)
{
    (void)p;
    uint para = 0, np;

    np = get_howmany_para(p);
    if(np > 0){
        str_to_hex(p, &para);
    }
    prt_hex(para);
    if(para==0){
        lprintf("gpio switch test\r\n");
        while(1){
            GPIOC->BSRR = GPIO_PIN_6;
            GPIOC->BSRR = (uint32_t)GPIO_PIN_6<< 16U;
        }
    }
    else if(para==1){
        lprintf("uart dma rcv test\r\n");
        start_dma_uart_recv();
        while(1){
            if(dma_uart_recved()){
                mem_print(duf, 0, 20);
            }
        }
    }
}
static const struct command cmd_list[]=
{
    {"exit",cmd_exit},
    {"help",print_help},
    {"history",history},
    {"pm", print_mem},
    {"r",read_mem},
    {"reboot",reboot},
    {"test",test},
    {"w",write_mem},
    {NULL, NULL},
};
static uint32_t * mrw_addr;
void go(char *para)
{
    (void)para;
	(*((void (*)())mrw_addr))();
}

void print_help(char *para)
{
    (void)para;
    uint i = 0;
    //lprintf("Version %s%s\r\n", VERSION, GIT_SHA1);
    lprint("Cmd:\r\n");
    while(1){
            if(cmd_list[i].cmd_name == NULL)
                    break;
	    lprint("--%s\r\n", cmd_list[i].cmd_name);
            i++;
    }
}

uint asc_to_hex(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';	
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return 0;
}

uint get_howmany_para(char *s)
{
	uint tmp = 0;
	while(1){
		while(*s == ' ')
			s++;
		if(*s)
			tmp++;
		while(*s != ' ' && *s)
			s++;
		if(!*s)
			return tmp;
	}
}

char * str_to_str(char *s, char**result)
{
    while(*s == ' ')s++;
    *result=s;
    while(*s != ' ' && *s != 0)s++;
    if(*s==0){
        return NULL;
    }
    *s++=0;
    while(*s == ' ')s++;
    return s;
}

char * str_to_hex(char *s, uint32_t * result)
{
	uint  i = 0;

	*result = 0;
	while(*s == ' ')s++;
	for(i=0;i<8;i++){
		if(*s == ' ' || *s == 0)
			break;
		*result = *result*16 + asc_to_hex(*s++);
	}
	while(*s == ' ')s++;
	return s;
}

void print_mem(char *p)
{
    uint length = 0x80, tmp;
    char *cp;

    tmp = get_howmany_para(p);
    if( tmp > 1)
        goto error;
    if(tmp == 0)
        goto print;
    str_to_hex(p, &length);
print:
    cp = (char *)mrw_addr;
    mem_print(cp, (uint32_t)mrw_addr, length);

    return;

error:
    lprint("Err!\r\npm [length]\r\n");

}

void write_mem(char *p)
{
    uint value, tmp;

    tmp = get_howmany_para(p);
    if(tmp == 0 || tmp > 2)
	goto error;
    p = str_to_hex(p, &value);
    if(tmp == 1)
        goto write;
    str_to_hex(p, (uint32_t*)&mrw_addr);
    mrw_addr = (uint32_t*)((uint32_t)mrw_addr & 0xfffffffc);
write:
    *(uint32_t*)mrw_addr = value;
    lprint("Write 0x%x@0x%x\r\n",value,mrw_addr);
    return;

error:
    lprint("Err!\r\nw v [addr]\r\n");

}


void read_mem(char *p)
{
    uint value, tmp;

    tmp = get_howmany_para(p);
    if( tmp > 1)
	goto error;
    if(tmp == 0)
    	goto read;
    str_to_hex(p, (uint32_t*)&mrw_addr);
    mrw_addr = (uint32_t*)((uint32_t)mrw_addr & 0xfffffffc);
read:
    value = *(uint32_t*)mrw_addr;
    lprint("Read 0x%x at memory 0x%x\r\n",value,mrw_addr);

    return;

error:
    lprint("Err!\r\nr addr\r\n");

}

void handle_cmd()
{
    unsigned char i = 0;
    char *p_cmd, *p_buf;

    lprint("\r\n");
    if(!cmd_buf[0])
	return;
    //record the history cmd
    review_cmd_his_index = cmdcache_index;
    lstrncpy((char*)cmd_caches[cmdcache_index++], cmd_buf, COM_MAX_LEN);
    if(cmdcache_index==CMD_CACHES_SIZE){
        cmdcache_index = 0;
    }
    while(1){
	    if(cmd_list[i].cmd_name == NULL)
		    break;
	    p_cmd=cmd_list[i].cmd_name;
	    p_buf=cmd_buf;
	    while(*p_cmd){
		    if(*p_buf != *p_cmd)
			    break;
		    p_buf++;
		    p_cmd++;
	    }
	    if(!(*p_cmd) && (*p_buf == ' ' || !(*p_buf))){
            	    cmd_list[i].cmd_fun(p_buf);
                    lprint("'%s' done.\r\n", cmd_list[i].cmd_name);
            	    return;
       	    }
	    i++;
    }
    lprint("Unknow cmd:%s\r\n",cmd_buf);
}
#define POWER_TIMEOUT_S 10
extern uint32_t shot_msct;
extern uint32_t shot_systick_val;
extern unsigned long debug_enable;
uint time_limit_recv_byte(uint limit, char * c);
void run_cmd_interface()
{
    char c = 0, last_c = 0;

    logline;
    mrw_addr = (uint32_t*)0x20000000;
#ifdef GIT_SHA1
    lprintf("GIT %s\r\n", GIT_SHA1);
#else
    lprintf("NO GIT info\r\n");
#endif
    lprint("\r\n\r\nclean_cmd. \r\n'c' key go cmd...\r\n");
    lmemset(cmd_buf, 0, COM_MAX_LEN);
    lmemset((char*)&cmd_caches[0][0], 0, CMD_CACHES_SIZE*COM_MAX_LEN);;
    cmd_buf_p = 0;
    lprintf_time("Enter CMD\r\n");
    lprint("\r\nCleanCMD>");

    while(!quit_cmd){
        last_c = c;
        //wait_input();
        c = con_recv();
        if(c == ENTER_CHAR || c == 0x1b || c== 0x03){
            if(c == ENTER_CHAR){
                handle_cmd();
                lprintf("\r\n");
            }
            if(c == 0x03){
                lprintf("^C\r\n");
            }
            if(c == 0x1b){
                lprintf("\r\n");
            }
            lmemset(cmd_buf, 0, COM_MAX_LEN);
            cmd_buf_p = 0;
            lprint("CleanCMD>");
        }
        else if(c == 0x08){
            if(!cmd_buf_p)
                continue;
            cmd_buf[--cmd_buf_p] = 0;
            lprintf("\b \b");
        }
        else if(c == 0x7f){
            if(!cmd_buf_p)
                continue;
            cmd_buf[--cmd_buf_p] = 0;
            con_send(c);
        }
        else if(c == 0x5B && last_c == 0x1b){//history cmd
            lstrncpy(cmd_buf, (char*)cmd_caches[review_cmd_his_index],COM_MAX_LEN);
            cmd_buf_p = strlen(cmd_buf);
            lprintf("%s", cmd_caches[review_cmd_his_index]);
            if(review_cmd_his_index==0){
                review_cmd_his_index=CMD_CACHES_SIZE;
            }
            review_cmd_his_index--;
        }
        else{
            if(cmd_buf_p < (COM_MAX_LEN - 1)){
                cmd_buf[cmd_buf_p++] = c;
                con_send(c);
            }
        }
    }
    quit_cmd = 0;
    lprintf_time("Quit CMD\r\n");
}
