#ifndef __COMMON_H
#define __COMMON_H

char* lstrncpy(char*d, const char*s, unsigned int n);
int lstrcmp(const char *a,const char *b);
void lmemset(char *d,unsigned char v,unsigned int n);
char *lstrchr(const char *str, int c);
void putchars(const char *pt);

#endif
