#include "env.h"
#include "sha256.h"
#include "cmd.h"
#include "mock_uart.h"
#include "common.h"
#include "lprintf.h"
#include <stdint.h>
#include <string.h>
#include <display.h>
#define uint uint32_t
#define lprint lprintf

static char cmd_buf[COM_MAX_LEN] = "";
static uint cmd_buf_p = COM_MAX_LEN;
static uint quit_cmd = 0;
extern uint32_t task_mask;
extern uint32_t logv;

uint get_howmany_para(char *s);
char * str_to_str(char *s, char**result);
/*
void con_send(char X)
{
    __io_putchar(X);
    if(X == '\n'){
        __io_putchar('\r');
    }
}
*/
uint32_t ci=0;

uint8_t cmd_caches[CMD_CACHES_SIZE][COM_MAX_LEN] = {0};
uint32_t cmdcache_index=0;
uint32_t review_cmd_his_index;
uint8_t read_buf[512];

extern const uint8_t ziku12[];
extern const uint8_t ziku[];
uint32_t get_ziku12_size();
uint32_t get_ziku_size();

void cmd_exit(char *p)
{
    (void)p;
    lprintf("Quit CMD!\n");
    quit_cmd = 1;

    return;

}
void reboot(char *p)
{
    (void)p;
    soft_reset_system();

    return;

}

void history(char *p)
{
    (void)p;
    uint32_t n = CMD_CACHES_SIZE, cix = cmdcache_index;
    con_send('\n');
    while(n--){
        if(cix==0){
            cix = CMD_CACHES_SIZE-1;
        }
        else{
            cix--;
        }
        if(cmd_caches[cix][0]!=0){
            lprintf("%s\n", cmd_caches[cix]);
        }
    }
    con_send('\n');

    return;
}
void poweroff(char *p)
{
    char*p1;
    uint32_t np = get_howmany_para(p);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    lprintf("number para=%d\n", np);
    if(np==0){
        lprintf("need para!\n");
        return;
    }
    else{
        p = str_to_str(p, &p1);
        lprintf("p1=%s\n", p1);
    }
    if(!lstrcmp(p1, "sleep")){
        lprintf("gosleep\r\n");
        __WFI();
        lprintf("wake\r\n");
    }
    else if(!lstrcmp(p1, "stop")){
        lprintf("gostop\r\n");
        PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
        lprintf("wake\r\n");
    }
    else if(!lstrcmp(p1, "standby")){
        GPIO_InitTypeDef GPIO_InitStructure;
        PWR_WakeUpPinCmd (DISABLE);
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        PWR->CR |= PWR_CR_CWUF;
        PWR_WakeUpPinCmd (ENABLE);
        PWR_EnterSTANDBYMode();
    }
    else{
        lprintf("error para.\r\n");
    }
    con_send('\n');

    return;

}
void muart(char *p)
{
    char*p1;
    //int p2=10000;
    char rv;
    uint32_t np = get_howmany_para(p);
    lprintf("number para=%d\n", np);
    if(np==0){
        lprintf("no para!\n");
        while(1){
            rv='U';
            mock_uart_sends(&rv, 1);
            delay_ms(1000);
            rv='W';
            mock_uart_sends(&rv, 1);
            delay_ms(1000);
        }
        return;
    }
    else{
        p = str_to_str(p, &p1);
        lprintf("p1=%s\n", p1);
    }
    if(!lstrcmp(p1, "tx")){
        lprintf("tx test!\n");
        while(1){
            rv = con_recv();
            mock_uart_sends(&rv, 1);
        }
    }
    else if(!lstrcmp(p1, "rx")){
        lprintf("rx test!\n");
        while(1){
            if(0!=mock_uart_rx(&rv, 1, 100)){
                prt_hex(rv);
                lprintf("%c\r\n", rv);
            }
        }
    }
    else{
        lprintf("error para.\r\n");
    }
    con_send('\n');

    return;

}

void fmenv(char *p)
{
    uint32_t para1 = 0, para2=0, tmp, cmdindex;

    lprintf("p=%s\n", p);
    tmp = get_howmany_para(p);
    lprintf("tmp=%d\n", tmp);
    if(tmp>=1){
	    p = str_to_hex(p, &cmdindex);
    }
    if(tmp == 0 || cmdindex == 0){
    }
    else if(cmdindex == 4){//
    }
    else if(cmdindex == 5){//fake w w25f
    }
    else if(cmdindex == 6){//cmd
    }
    else if(cmdindex == 7){//w w25f
    }
    else if(cmdindex == 8){//
        lprintf("erase one sector.\n");
        if(tmp<2){
            goto err;
        }
        p = str_to_hex(p, &para1);
        p = str_to_hex(p, &para2);
        if(para2 != 0xe4a5e07e){
            lprintf("erase %X fail", para1);
        }
        else{
            lprintf("not implement\n");
            //SPI_Flash_Erase_Sector(GET_SECTOR_ADDR(para1));
        }
    }
    else if(cmdindex == 0x9){//
        lprintf("switch env area\n");
        switch_env_area();
    }
    else if(cmdindex == 0xa){//
        lprintf("switch env area with data\n");
        p = str_to_hex(p, &para1);
        if(para1 != 0x5e14c6e7){
            lprintf("switch env fail", para1);
            goto err;
        }
        switch_env_area_with_data();
    }
    else if(cmdindex == 0xb){//
        lprintf("erase cur env area\n");
        p = str_to_hex(p, &para1);
        if(para1 != 0xe4a5ee7c){
            lprintf("erase env fail", para1);
        }
        else{
            erase_env_area();
        }
    }
    else if(cmdindex == 0xc){//
    }
    else if(cmdindex == 0xd){
    }
    con_send('\n');

    return;
err:
    lprintf("ERROR para\n");
}
void envprint(char *p)
{
    char*para;
    int tmp = get_howmany_para(p);
    lprintf("number of para %d\n", tmp);
    if(tmp>0){
        str_to_str(p, &para);
        if(0==lstrcmp(para, "r")){
            lprintf("raw env:\n");
            printrawenv(0);
        }
        else if(0==lstrcmp(para, "i")){
            lprintf("idle area raw env:\n");
            printrawenv(1);
        }
    }
    else{
        printenv();
    }
    con_send('\n');

    return;

}
void envget(char *p)
{
    uint32_t tmp;
    char* name, value[ENV_MAX_VALUE_LEN];
    tmp = get_howmany_para(p);
    lprintf("tmp=%d\n", tmp);
    if(tmp<1){
        lprintf("err\n");
        return;
    }
    str_to_str(p, &name);
    if(ENV_FAIL == get_env(name, value)){
        lprintf("get_env fail\n");
        return;
    }
    else{
        lprintf("get_env OK\n");
    }
    lprintf("%s=%s\n", name, value);
    con_send('\n');

    return;

}
void envset(char *p)
{
    uint32_t tmp;
    char* name, *value=0;
    tmp = get_howmany_para(p);
    lprintf("tmp=%d\n", tmp);
    if(tmp<1){
        lprintf("err\n");
        return;
    }
    if(tmp>=1){
        p = str_to_str(p, &name);
    }
    value = p;
    if(value){
        lprintf("todo: %s=%s\n", name, value);
    }
    else{
        lprintf("todo: del %s\n", name);
    }
    if(ENV_FAIL == set_env(name, value)){
        lprintf("set_env fail\n");
        return;
    }
    else{
        lprintf("set_env OK\n");
    }
    con_send('\n');

    return;

}
void adc(char *p)
{
    uint32_t np, rct=1000;
    np = get_howmany_para(p);
    if(np==0){
        lprintf("no para!\n");
        while(rct--){
            prt_hex(get_adc_value(0));
            delay_ms(1000);
        }
        return;
    }
    else{
        char*p1;
        p = str_to_str(p, &p1);
        lprintf("p1=%s\n", p1);

        if(!lstrcmp(p1, "r")){
            lprintf("adc random!\n");
            while(rct--){
                prt_hex(adc_random(0));
                lprintf("%c\n", asc_random());
                delay_ms(1000);
            }
        }
        else{
            lprintf("error para.\r\n");
        }
    }
    con_send('\n');

    return;
}
void csha(char *p)
{
    uint32_t np;
    np = get_howmany_para(p);
    if(np==0){
        lprintf("input str!\n");
        return;
    }
    else{
        char*p1;
        p = str_to_str(p, &p1);
        lprintf("p1=%s\n", p1);
        compute_sha256((uint8_t*)p1, strlen(p1), (char*)read_buf);
        lprintf("sha256=%s\r\n", read_buf);
    }
    con_send('\n');

    return;
}
void rtc(char *p)
{
    uint8_t tmp;
    uint32_t tp;

    tmp = get_howmany_para(p);
    lprintf("tmp=%d\n", tmp);
    if(tmp==6){
        p = str_to_hex(p, &tp);
        dt->y=tp+2000;
        p = str_to_hex(p, &tp);
        dt->m=tp;
        p = str_to_hex(p, &tp);
        dt->d=tp;
        p = str_to_hex(p, &tp);
        dt->h=tp;
        p = str_to_hex(p, &tp);
        dt->min=tp;
        p = str_to_hex(p, &tp);
        dt->s=tp;
        RTC_Set();
    }
    else if(tmp==0){
        while(!con_is_recved()){
            lprintf("%s\n", RTC_Get());
            delay_ms(1000);
        }
    }
    con_send('\n');

    return;

}
void test(char*para)
{
    (void)para;
    while(1){
        delay_ms(200);
#ifdef SVR
        prt_hex(LOCKPOSI_LOCKED());
#endif
    }
}
static const struct command cmd_list[]=
{
    {"adc",adc},
    {"csha",csha},
    {"exit",cmd_exit},
    {"envset",envset},
    {"envget",envget},
    {"envprint",envprint},
    {"fmenv",fmenv},
    {"help",print_help},
    {"history",history},
    {"muart",muart},
    {"pm",print_mem},
    {"poff",poweroff},
    {"r",read_mem},
    {"reboot",reboot},
    {"rtc",rtc},
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
    lprintf("Version %s%s\n", VERSION, GIT_SHA1);
    lprint("Cmd:\n");
    while(1){
            if(cmd_list[i].cmd_name == NULL)
                    break;
	    lprint("--%s\n", cmd_list[i].cmd_name);
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
    lprint("Err!\npm [length]\n");

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
    lprint("Write 0x%x@0x%x\n",value,mrw_addr);
    return;

error:
    lprint("Err!\nw v [addr]\n");

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
    lprint("Read 0x%x at memory 0x%x\n",value,mrw_addr);

    return;

error:
    lprint("Err!\nr addr\n");

}

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

void handle_cmd()
{
    unsigned char i = 0;
    char *p_cmd, *p_buf;

    lprint("\n");
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
                    lprint("'%s' done.\n", cmd_list[i].cmd_name);
            	    return;
       	    }
	    i++;
    }
    lprint("Unknow cmd:%s\n",cmd_buf);
}
#define POWER_TIMEOUT_S 10
extern uint32_t shot_msct;
extern uint32_t shot_systick_val;
void wait_input()
{
    uint32_t ms1, ms2, val1, val2;
    uint64_t nt;
    uint64_t st=get_system_us();
    ms1=shot_msct;
    val1=shot_systick_val;
    while(!con_is_recved()){
        nt=get_system_us();
        ms2=shot_msct;
        val2=shot_systick_val;
        //prt_dec((uint32_t)get_system_us()/1000000);
        if((nt-st)>((uint64_t)POWER_TIMEOUT_S*1000000u)){
            lprintf("\r\nPower timeout(10s)! Go standby\r\n");
            prt_dec((uint32_t)st);
            prt_dec((uint32_t)nt);
            prt_dec((uint32_t)(nt-st));
            prt_dec(ms1);
            prt_dec(ms2);
            prt_dec(val1);
            prt_dec(val2);
            poweroff("standby");
        }
    }
}
extern unsigned long debug_enable;
uint time_limit_recv_byte(uint limit, char * c);
void run_cmd_interface()
{
    char c = 0, last_c = 0;

    mrw_addr = (uint32_t*)0x20000000;
    lprintf("Version %s%s\n", VERSION, GIT_SHA1);
    lprint("\n\nclean_cmd. \n'c' key go cmd...\n");
    lmemset(cmd_buf, 0, COM_MAX_LEN);
    lmemset((char*)&cmd_caches[0][0], 0, CMD_CACHES_SIZE*COM_MAX_LEN);;
    cmd_buf_p = 0;
    lprintf_time("Enter CMD\n");
    lprint("\nCleanCMD>");

    while(!quit_cmd){
        last_c = c;
        //wait_input();
        c = con_recv();
        if(c == ENTER_CHAR || c == 0x1b || c== 0x03){
            if(c == ENTER_CHAR){
                handle_cmd();
                lprintf("\n");
            }
            if(c == 0x03){
                lprintf("^C\n");
            }
            if(c == 0x1b){
                lprintf("\n");
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
    lprintf_time("Quit CMD\n");
}
