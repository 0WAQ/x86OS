/**
 * 
 * 字符串的处理函数
 *
 */ 

#include "tools/klib.h"

void kernel_vsprintf(char* buf, const char* fmt, va_list args) {
    
    // 定义两种状态
    enum {READ_FMT, READ_ARGS} state = NORMAL;
    
    char* cur = buf;
    char ch;

    while((ch = *fmt++) != '\0') {
        switch (state)
        {
        case READ_FMT:
            // 若当前字符是 % 就改变状态
            if(ch == '%') {
                state = READ_ARGS;
            }
            else {
                // 否则正常拷贝
                *cur++ = ch;
            }
            break;
        
        case READ_ARGS:
            // 字符串类型
            if(ch == 's') {
                const char* str = va_arg(args, char*);
                
                // 将这个字符串放入buf
                int len = kernel_strlen(str);
                while(len--) {
                    *cur++ = *str++;
                }
            }
            // 整型
            else if(ch == 'd') {

            }

            state = READ_FMT;
            break;
        }
    }
}

void kernel_strcpy(char* dest, const char* src) {
    if(!dest || !src) {
        return;
    }

    while(*dest && *src) {
        *dest++ = *src++;
    }

    *dest = '\0';
}

void kernel_strncpy(char* dest, const char* src, int size) {
    if(!dest || !src || !size) {
        return;
    }

    char* d = dest;
    const char* s = src;
    while(size-- > 0 && *s) {
        *d++ = *s++;
    }

    if(size == 0) {
        *(d - 1) = '\0';
    }
    else {
        *d = '\0';
    }
}

// TODO:
int kernel_strncmp(const char* s1, const char* s2, int size) {
    if(!s1 || !s2) {
        return -1;
    }

    while(*s1 && *s2 && (*s1 == *s2) && size) {
        ++s1, ++s2;
        --size;
    }

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}

int kernel_strlen(const char* str) {
    if(!str) {
        return 0;
    }

    const char* s = str;
    int len = 0;
    while(*s++) {
        ++len;
    }
    return len;
}

void kernel_memcpy(void* dest, void* src, int size) {
    if(!dest || !src || !size) {
        return;
    }

    uint8_t* s = (uint8_t*)src;
    uint8_t* d = (uint8_t*)dest;
    while(size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void* dest, uint8_t v, int size) {
    if(!dest || !size) {
        return;
    }

    uint8_t* d = (uint8_t*)dest;
    while(size--) {
        *d++ = v;
    }
}

int kernel_memcmp(void* d1, void* d2, int size) {
    if(!d1 || !d2 || !size) {
        return 1;
    }

    uint8_t* p1 = (uint8_t*)d1;
    uint8_t* p2 = (uint8_t*)d2;
    while(size--) {
        if(*p1++ != *p2++) {
            return 1;
        }
    }
    return 0;
}
