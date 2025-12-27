#ifndef __COMMON_H
#define __COMMON_H

#include "lprintf.h"
#include "cmd.h"
#include "stm32f4xx_hal.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

char* lstrncpy(char*d, const char*s, unsigned int n);
int lstrcmp(const char *a,const char *b);
void lmemset(char *d,unsigned char v,unsigned int n);
char *lstrchr(const char *str, int c);
void putchars(const char *pt);
int cam_init(int);
extern DCMI_HandleTypeDef hdcmi;
extern UART_HandleTypeDef huart2;

extern volatile uint32_t ghn;
extern volatile uint32_t gfn;
void MX_DCMI_Init(void);
void Lcd_WriteData_16Bit(u16 Data);
extern int g_dcmi_cfg;
extern int g_cam_r70p_e;
extern int g_cam_r71p_e;
extern int cam_linect;
extern int cam_vsct;
extern int cam_fmct;
extern int cam_dma_err;
void rgb565_to_lcd(u8*buf, u32 len);
void rgb565_to_lcd_noswap(u8*buf, u32 len);
int cam_w_reg(uint8_t addr, uint8_t data);
uint8_t cam_r_reg(uint8_t addr);
extern int camreaderr;
void wait_sdw_done();
void sd_write(uint8_t*w_buf, uint32_t p1, uint32_t p2);
#endif
