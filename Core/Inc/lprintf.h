#ifndef __LPRINTF_H
#define __LPRINTF_H
#include<stdint.h>

#define DEBUG_LOG_BUF_SIZE 2048
int __io_putchar(int ch);
uint16_t __io_getchar();
int __io_char_received();

extern uint16_t LCD_PRINT_BACK_COLOR;
extern uint16_t LCD_PRINT_FRONT_COLOR;
void lprintf(const char *fmt, ...);
void lprintf_time(const char *fmt, ...);
void lprintf_to(const char *fmt, ...);
void lprintf_time_buf(uint32_t, const char *fmt, ...);
void slprintf(char*buf, const char *fmt, ...);
void lcd_lprintf(uint32_t cs, uint32_t x, uint32_t y, const char *fmt, ...);
void lcd_lprintf_win(uint32_t cs, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char *fmt, ...);
void mem_print(const char*buf, uint32_t ct_start, uint32_t len);
void puthexch(char c);
char halfbyte2char(char c);

#define logline lprintf("line:%d %s\r\n", __LINE__, __func__)
#define prtline lprintf("line:%d %s\r\n", __LINE__, __func__)
#define prt_hex(x) lprintf(#x"=%x\r\n", x)
#define prt_dec(x) lprintf(#x"=%d\r\n", x)

#endif
