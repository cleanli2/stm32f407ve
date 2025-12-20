#ifndef __COMMON_H
#define __COMMON_H

#include "lprintf.h"
#include "stm32f4xx_hal.h"

typedef uint8_t u8;
typedef uint32_t u32;

char* lstrncpy(char*d, const char*s, unsigned int n);
int lstrcmp(const char *a,const char *b);
void lmemset(char *d,unsigned char v,unsigned int n);
char *lstrchr(const char *str, int c);
void putchars(const char *pt);
int cam_init(void);
extern DCMI_HandleTypeDef hdcmi;

extern int cam_dma_err;
void rgb565_to_lcd(u8*buf, u32 len);
#endif
