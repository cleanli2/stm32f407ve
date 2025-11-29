#include "lprintf.h"
#include "common.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>


char buf_printf_buf[64];
char lprintf_buf[256];
char lcdprintf_buf[256];
char halfbyte2char(char c)
{
    c&=0xf;
    return ((c & 0x0f) < 0x0a)?(0x30 + c):('A' + c - 0x0a);
}

void puthexch(char c)
{
    __io_putchar(halfbyte2char(c>>4));
    __io_putchar(halfbyte2char(c&0xf));
}

void put_hex_uint(uint32_t i)
{
    int c = 8;
    while(c--){
        __io_putchar(halfbyte2char((char)((i&0xf0000000)>>28)));
        i<<=4;
    }
}

int sput_hex_uint8(char*s, uint32_t i)
{
    int c = 2;
    char* p = s;
    while(c--){
        *p++ = (halfbyte2char((char)((i&0xf0)>>4)));
        i<<=4;
    }
    return 2;
}

int sput_hex_uint(char*s, uint32_t i)
{
    int c = 8;
    char* p = s;
    while(c--){
        *p++ = (halfbyte2char((char)((i&0xf0000000)>>28)));
        i<<=4;
    }
    return 8;
}

int sput_hex_uint64(char*s, uint64_t i)
{
    int c = 16;
    char* p = s;
    while(c--){
        *p++ = (halfbyte2char((char)((i&0xf000000000000000)>>60)));
        i<<=4;
    }
    return 16;
}

void puthexchars(char *pt)
{
    while(*pt){
        puthexch(*pt++);
        __io_putchar(' ');
    }
}


char * num2str(uint64_t jt, char * s, char n)
{
        char * st, k = 1, j;
        uint64_t tmp;

        st = s;
        if(n > 16 || n < 2){
                *st++ = 0x30;
                *st = 0;
                return s;
        }
        tmp = 1;
        while(jt/tmp >= n){
                k++;
                tmp *= n;
        }

        while(k--){
                j = jt/tmp;
                *st++ = halfbyte2char(j);
                jt -= tmp * j;
                tmp = tmp/n;
        }
        *st = 0;
        return s;
}

void print_uint(uint32_t num)
{
        char nc[11];
        num2str(num, nc, 10);
        putchars(nc);
}

int sprint_uint64(char*s, uint64_t num)
{
    char nc[21];
    lmemset(nc, 0, 21);
    num2str(num, nc, 10);
    lstrncpy(s, nc, 21);
    return strlen(nc);
}

int sprint_uint(char*s, uint32_t num)
{
    char nc[11];
    num2str(num, nc, 10);
    lstrncpy(s, nc,11);
    return strlen(nc);
}

int sprint_uint_0n(char*s, uint32_t num, uint32_t num_len)
{
    char nc[22];
    uint32_t raw_len;
    lmemset(nc,'0', 22);
    raw_len = sprint_uint(nc+11, num);
    lstrncpy(s, nc+11+raw_len-num_len, 22);
    return strlen(nc+11+raw_len-num_len);
}

void print_hex(uint32_t num)
{
        char nc[9];
        num2str(num, nc, 16);
        putchars(nc);
}

int sprint_hex(char*s, uint32_t num)
{
    char nc[9];
    num2str(num, nc, 16);
    lstrncpy(s, nc, 9);
    return strlen(nc);
}

void print_binary(uint32_t num)
{
        char nc[33];
        num2str(num, nc, 2);
        putchars(nc);
}
char sys_hour[14];

char*vslprintf(int print_with_time, char*s_buf, const char *fmt, va_list args)
{
    (void)print_with_time;
    const char *s;
    uint32_t d;
    int32_t di;
    uint64_t u;
    va_list ap;
    char*sp = s_buf;

    va_copy(ap, args);
    while (*fmt) {
        if (*fmt != '%') {
            *sp++ = *fmt++;
            continue;
        }
        switch (*++fmt) {
	    case '%':
	        *sp++ = (*fmt);
		break;
            case 's':
                s = va_arg(ap, const char *);
                lstrncpy(sp, s, 256);
                sp += strlen(s);
                break;
            case 'u':
                d = va_arg(ap, uint32_t);
                sp += sprint_uint(sp, d);
                break;
            case 'd':
                di = va_arg(ap, int32_t);
                if(di<0){
                    d=-di;
                    *sp++='-';
                }
                else{
                    d=di;
                }
                sp += sprint_uint(sp, d);
                break;
            case 'U':
                u = va_arg(ap, uint64_t);
                sp += sprint_uint64(sp, u);
                break;
	    case 'c':
                d = va_arg(ap, uint32_t);
                *sp++ = d&0xff;
                break;
	    case 'x':
                d = va_arg(ap, uint32_t);
                sp += sprint_hex(sp, d);
                break;
	    case 'W':
                u = va_arg(ap, uint64_t);
                sp += sput_hex_uint64(sp, u);
                break;
	    case 'X':
                d = va_arg(ap, uint32_t);
                sp += sput_hex_uint(sp, d);
                break;
	    case 'b':
                d = va_arg(ap, uint32_t);
                sp += sput_hex_uint8(sp, d);
                break;
            /* Add other specifiers here... */             
            default: 
                *sp++ = (*(fmt-1));
                *sp++ = (*fmt);
                break;
        }
        fmt++;
    }
    *sp = 0;
    va_end(ap);
    return sp;
}

#if 1
void lprintf_time(const char *fmt, ...)
{
    va_list ap;

    va_start(ap,fmt);
#ifdef NO_PRINT_WITH_TIME
    vslprintf(0, lprintf_buf,fmt,ap);
#else
    vslprintf(1, lprintf_buf,fmt,ap);
#endif
    putchars(lprintf_buf);
    va_end(ap);
}
#endif

void lprintf(const char *fmt, ...)
{
#if 1
    va_list ap;

    va_start(ap,fmt);
    vslprintf(0, lprintf_buf,fmt,ap);
    putchars(lprintf_buf);
    va_end(ap);
#else
    putchars(fmt);
#endif
}


void slprintf(char*buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    vslprintf(0, buf,fmt,ap);
    va_end(ap);
}

void mem_print(const char*buf, uint32_t ct_start, uint32_t len)
{
    const char*line_stt = buf;
    uint32_t left=len, line_len;

    putchars("\r\nMemShow Start:");
    while(left){
        int j, li;
        line_len = left>16?16:left;
        li=line_len;
        __io_putchar('\r');
        __io_putchar('\n');
        print_hex(ct_start);
        __io_putchar(':');
        __io_putchar(' ');
        j=0;
        while(li--){
            puthexch(line_stt[j]);
            __io_putchar(j == 7 ? '-':' ');
            j++;
        }
        li=line_len;
        j=0;
        __io_putchar(' ');
        while(li--){
            if(line_stt[j]>=0x20 && line_stt[j]<0x7f){
                __io_putchar(line_stt[j]);
            }
            else{
                __io_putchar('_');
            }
            j++;
        }
        left-=line_len;
        line_stt+=line_len;
        ct_start+=line_len;
    }
    lprintf("\r\nMemShow End:\r\n");
}
