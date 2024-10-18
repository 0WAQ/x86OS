/**
 * 
 * 字符串的处理函数
 *
 */ 

#include "tools/klib.h"

void kernel_sprintf(char* buf, const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kernel_vsprintf(buf, fmt, args);
    va_end(args);    
}

void kernel_vsprintf(char* buf, const char* fmt, va_list args) {
    
    // 定义两种状态
    enum {READ_FMT, READ_ARGS} state = READ_FMT;
    
    char* cur = buf;
    char ch;

    while((ch = *fmt++) != '\0') {
        switch (state)
        {
        // 读取fmt
        case READ_FMT:
            // 若读到 % 就改变状态
            if(ch == '%') {
                state = READ_ARGS;
            }
            // 否则就正常拷贝
            else {
                *cur++ = ch;
            }
            break;
        
        // 读取args
        case READ_ARGS:
            switch (ch)
            {
            case 's':   // 字符串
            {
                const char* str = va_arg(args, char*);
                
                // 将这个字符串放入buf
                int len = kernel_strlen(str);
                while(len--) {
                    *cur++ = *str++;
                }
                break;
            }
            case 'd':   // 整型
            {
                int num = va_arg(args, int);
                kernel_itoa(cur, num, 10);
                cur += kernel_strlen(cur);
                break;
            }
            case 'x':   // 十六进制数
            {
                *cur++ = '0';
                *cur++ = 'x';
                int num = va_arg(args, int);
                kernel_itoa(cur, num, 16);
                cur += kernel_strlen(cur);
                break;
            }
            case 'c':   // 字符
            {
                char c = va_arg(args, int);
                *cur++ = c;
                break;
            }
            }

            state = READ_FMT;
            break;
        }
    }
}

void kernel_itoa(char* dest, int num, uint32_t base) {
    char* p = dest;
    char* start = dest;
    
    // 不支持其它进制
    if(base != 2 && base != 8 && base != 10 && base != 16) {
        *p = '\0';
        return;
    }

    // 只有十进制支持负数
    if(num < 0 && base == 10) {
        ++start;    // 在下面对调时跳过负号
        *p++ = '-';
    }

    // 转换表
    static const char* num2ch = {"0123456789ABCDEF"};

    uint32_t unum = num < 0 ? -num : num;
    do {
        *p++ = num2ch[unum % base];
        unum /= base;
    }while(unum);
    *p-- = '\0';

    // 对调
    while(start < p) {
        char ch = *start;
        *start = *p;
        *p = ch;
    
        ++start, --p;
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
