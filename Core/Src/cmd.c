#include "cmd.h"
#include "lprintf.h"
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "lcd_nt35510.h"
#include "common.h"

#define uint uint32_t
#define lprint lprintf

#define BBN 5
#define MIN_YUV_FILES_NUM NPERBB*BBN
#define NPERBB 10000
#define GET_FILE_PATH_AND_NAME(buf, n) \
        slprintf(buf, "BB%d/V%d/YUV%d.BIN", n/NPERBB, (n%NPERBB)/100, n%NPERBB);

extern int g_fmounted;
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
uint8_t read_buf[512*16];

extern const uint8_t ziku12[];
extern const uint8_t ziku[];
uint32_t get_ziku12_size();
uint32_t get_ziku_size();
void write_mem16(char *p);
void read_mem16(char *p);
void lcd_clr_window(u16 color, u16 xs, u16 ys, u16 xe, u16 ye);

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
void show_sdinfo();
void sd_read(uint8_t*read_buf, uint p1, uint p2);
//extern char*duf;
extern char duf[];
extern SD_HandleTypeDef hsd;
extern I2C_HandleTypeDef hi2c2;
void fs_test(const char*text);
void test(char*p)
{
    (void)p;
    uint para = 0, np;

    np = get_howmany_para(p);
    prt_dec(np);
    if(np > 0){
        p=str_to_hex(p, &para);
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
        prt_hex(duf);
        lmemset(duf, 0, 20);
        mem_print(duf, (uint32_t)duf, 20);
        start_dma_uart_recv();
        while(1){
            if(dma_uart_recved()){
                lprintf("recved!\r\n");
                mem_print(duf, (uint32_t)duf, 20);
            }
        }
    }
    else if(para==2){
        lprintf("sd card info\r\n");
        show_sdinfo();
    }
    else if(para==3){
        uint p1=0, p2=1;
        lprintf("sd card read\r\n");
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }
        if(np >= 3){
            p=str_to_hex(p, &p2);
        }
        prt_dec(p1);
        prt_dec(p2);
        sd_read(read_buf, p1, p2);
        mem_print((char*)read_buf, (uint32_t)0, 512);
    }
    else if(para==4){
        char*ps=0;
        lprintf("fs test\r\n");
        if(np >= 2){
            p=str_to_str(p, &ps);
        }
        prt_hex(ps);
        if(ps){
            lprintf("ps=%s\r\n", ps);
        }
        fs_test(ps);
    }
    else if(para==5){
        lprintf("i2c test\r\n");
        uint p1=0x20, p2=0x5a;
        uint8_t d;
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }
        if(np >= 3){
            p=str_to_hex(p, &p2);
        }
        d=p2;
        HAL_StatusTypeDef ret=HAL_I2C_Master_Transmit(&hi2c2, p1, &d, 1, 1000000);
        prt_hex(ret);
    }
    else if(para==7){
        lprintf("lcd clear test\r\n");
        uint p1=0xFFFF;
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }
        LCD_Clear(p1);
    }
    else if(para==8){
        lprintf("lcd set bak\r\n");
        uint p1=1000;
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }
        extern TIM_HandleTypeDef htim12;
        __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, p1&0xffff);
    }
    else if(para==0x65){
        FRESULT res; /* FatFs function common result code */
        char fnm[32];
        FIL MyFile; /* File object for SD */
        if(!g_fmounted){
            logline;
            return;
        }
        uint p1=100;
        uint32_t bytesread; /* File write/read counts */
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }

        if(p1>MIN_YUV_FILES_NUM-1){
            p1-= MIN_YUV_FILES_NUM;
        }
        lprintf("one pic show %d with data dump\r\n", p1);
        GET_FILE_PATH_AND_NAME(fnm, p1);
        lprintf("fnm=%s\r\n", fnm);
        FILINFO myfno;
        if(f_stat(fnm, &myfno) == FR_OK){
            prt_dec(myfno.fsize);
        }
        if(f_open(&MyFile, fnm, FA_READ) == FR_OK){
            lprintf("open OK\r\n");
            LCD_SetWindows(0,0,639,479);   
            res = f_read(&MyFile, (u8*)0x20004000, 0x1b000, (UINT*)&bytesread);
            //prt_hex(bytesread);
            //prt_hex(res);

            if((bytesread == 0) || (res != FR_OK))
            {
                logline;
                return;
            }
            else
            {
                //mem_print((char*)read_buf, 0, sizeof(read_buf));
                rgb565_to_lcd((u8*)0x20004000, bytesread);
            }
            /*##-9- Close the open text file #############################*/
            f_close(&MyFile);
        }
        else{
            logline;
        }
    }
    else if(para==6){
        FRESULT res; /* FatFs function common result code */
        char fnm[32];
        FIL MyFile; /* File object for SD */
        if(!g_fmounted){
            logline;
            return;
        }
        uint p1=100;
        uint32_t bytesread; /* File write/read counts */
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }

        while(1){//-------------------------------------------------------------------------

        if(p1>MIN_YUV_FILES_NUM-1){
            p1-= MIN_YUV_FILES_NUM;
        }
        lprintf("pic show %d\r\n", p1);
        GET_FILE_PATH_AND_NAME(fnm, p1);
        lprintf("fnm=%s\r\n", fnm);
        prt_hex(sizeof(read_buf));
        FILINFO myfno;
        if(f_stat(fnm, &myfno) == FR_OK){
            prt_dec(myfno.fsize);
        }
        if(f_open(&MyFile, fnm, FA_READ) == FR_OK){
            lprintf("open OK\r\n");
            u32 pic_ms=HAL_GetTick();
            lcd_clr_window(WHITE, 0, 0, 5, 479);
            lcd_clr_window(BLACK, 6, 0, 10, 479);
            lcd_clr_window(WHITE, 635, 0, 639, 479);
            lcd_clr_window(BLACK, 631, 0, 634, 479);
            LCD_SetWindows(0,0,639,479);   
            while(!f_eof(&MyFile)){
              res = f_read(&MyFile, read_buf, sizeof(read_buf), (UINT*)&bytesread);
              //prt_hex(bytesread);
              //prt_hex(res);

              if((bytesread == 0) || (res != FR_OK))
              {
                logline;
                break;
              }
              else
              {
#if 0
                //mem_print((char*)read_buf, 0, sizeof(read_buf));
                for(int ti=0;ti<bytesread;ti+=2){
                    Lcd_WriteData_16Bit((read_buf[ti]<<8)|read_buf[ti+1]);
                }
#else
                rgb565_to_lcd(read_buf, bytesread);
#endif
              }
            }
            pic_ms=HAL_GetTick()-pic_ms;
            prt_dec(pic_ms);
            /*##-9- Close the open text file #############################*/
            f_close(&MyFile);
        }
        else{
            logline;
        }
        p1++;

        }//-------------------------------------------------------------------------

    }
    else if(para==9){
        lprintf("cam init test\r\n");
        cam_init();
    }
    else if(para==0xa){
        HAL_StatusTypeDef ret;
        uint p1=320;
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }
        lprintf("start cam receive %d\r\n", p1);
        //0x20004000 -> (320x240=76800)0x12c00 ->0x20016c00
        cam_dma_err=0;
        cam_linect=0;
        cam_vsct=0;
        cam_fmct=0;
        ret=HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)0x20004000, p1/4);
        prt_hex(ret);
        lprintf("start cam receive done\r\n");
    }
    else if(para==0xb){
        lprintf("check cam receive\r\n");
        prt_hex(HAL_DCMI_GetState(&hdcmi));
        prt_hex(hdcmi.ErrorCode);
        prt_dec(cam_dma_err);
        prt_dec(cam_linect);
        prt_dec(cam_vsct);
        prt_dec(cam_fmct);
        LCD_SetWindows(0,0,639,479);
        rgb565_to_lcd((u8*)0x20004000, 0x1c000);
    }
    else if(para==0xb2){
        int n=0x1c000/2;
        uint16_t*tp=(uint16_t*)0x20004000;
        lprintf("check cam receive (2)\r\n");
        prt_hex(HAL_DCMI_GetState(&hdcmi));
        prt_hex(hdcmi.ErrorCode);
        prt_dec(cam_dma_err);
        prt_dec(cam_linect);
        prt_dec(cam_vsct);
        prt_dec(cam_fmct);
        LCD_SetWindows(0,0,639,479);
        while(n--){
			Lcd_WriteData_16Bit(*tp++);
        }
    }
    else if(para==0xb3){//move forwad 1 byte
        int n=0x1c000/2-1;
        uint16_t t;
        uint8_t* t8p=(uint8_t*)0x20004001;
        lprintf("check cam receive (3)\r\n");
        prt_hex(HAL_DCMI_GetState(&hdcmi));
        prt_hex(hdcmi.ErrorCode);
        prt_dec(cam_dma_err);
        prt_dec(cam_linect);
        prt_dec(cam_vsct);
        prt_dec(cam_fmct);
        LCD_SetWindows(0,0,639,479);
        while(n--){
            t=(*t8p++)<<8;
            t+=*t8p++;
			Lcd_WriteData_16Bit(t);
        }
    }
    else if(para==0x171){
        lprintf("toggle 71 1\r\n");
        g_cam_r71p_e=1-g_cam_r71p_e;
        prt_dec(g_cam_r71p_e);
    }
    else if(para==0x170){
        lprintf("toggle 70 1\r\n");
        g_cam_r70p_e=1-g_cam_r70p_e;
        prt_dec(g_cam_r70p_e);
    }
    else if(para==0xdccf){
        uint p1=0x101;
        lprintf("dcmi config\r\n");
        if(np >= 2){
            p=str_to_hex(p, &p1);
        }
        g_dcmi_cfg=p1;
        prt_hex(g_dcmi_cfg);
        MX_DCMI_Init();
    }
}
static const struct command cmd_list[]=
{
    {"exit",cmd_exit},
    {"help",print_help},
    {"history",history},
    {"hr",read_mem16},
    {"hw",write_mem16},
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

void write_mem16(char *p)
{
    uint value, tmp;

    tmp = get_howmany_para(p);
    if(tmp == 0 || tmp > 2)
	goto error;
    p = str_to_hex(p, &value);
    if(tmp == 1)
        goto write;
    str_to_hex(p, (uint32_t*)&mrw_addr);
    mrw_addr = (uint32_t*)((uint32_t)mrw_addr & 0xfffffffe);
    value&=0xffff;
write:
    *(uint16_t*)mrw_addr = value;
    lprint("Write 0x%x@0x%x\r\n",value,mrw_addr);
    return;

error:
    lprint("Err!\r\nw v [addr]\r\n");

}


void read_mem16(char *p)
{
    uint value, tmp;

    tmp = get_howmany_para(p);
    if( tmp > 1)
	goto error;
    if(tmp == 0)
    	goto read;
    str_to_hex(p, (uint32_t*)&mrw_addr);
    mrw_addr = (uint32_t*)((uint32_t)mrw_addr & 0xfffffffe);
read:
    value = *(uint16_t*)mrw_addr;
    lprint("Read 0x%x at memory 0x%x\r\n",value,mrw_addr);

    return;

error:
    lprint("Err!\r\nr addr\r\n");

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
