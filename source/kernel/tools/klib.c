/**
 * 
 * 内核通用函数
 *
 */ 

#include "tools/klib.h"
#include "tools/log.h"
#include "common/cpu_instr.h"

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

void kernel_itoa(char* dest, int num, u32_t base) {
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

    u32_t unum = (u32_t)num;
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

int kernel_strcmp(const char* s1, const char* s2) {
    ASSERT(s1 != NULL && s2 != NULL);

    while(*s1 && *s2 && (*s1 == *s2)) {
        *s1++;
        *s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int kernel_strncmp(const char* s1, const char* s2, int size) {
    ASSERT(s1 != NULL && s2 != NULL);

    if (size <= 0) {
        return 0;
    }

    while (size--) {
        if (*s1 == '\0' || *s2 == '\0' || *s1 != *s2) {
            return (unsigned char)*s1 - (unsigned char)*s2;
        }
        s1++;
        s2++;
    }

    return 0;
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

    u8_t* s = (u8_t*)src;
    u8_t* d = (u8_t*)dest;
    while(size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void* dest, u8_t v, int size) {
    if(!dest || !size) {
        return;
    }

    u8_t* d = (u8_t*)dest;
    while(size--) {
        *d++ = v;
    }
}

int kernel_memcmp(void* d1, void* d2, int size) {
    if(!d1 || !d2 || !size) {
        return 1;
    }

    u8_t* p1 = (u8_t*)d1;
    u8_t* p2 = (u8_t*)d2;
    while(size--) {
        if(*p1++ != *p2++) {
            return 1;
        }
    }
    return 0;
}

void panic(const char* file, int line, const char* func, const char* msg) {
    log_print("ERROR ASSERT(%s)", msg);
    log_print("file: %s\r\nlog: line: %d\r\nlog: func: %s\r\n", file, line, func);
    for(;;) { hlt(); }
}


int strings_count(char** addr) {
    int cnt = 0;
    if(addr) {
        while(*addr++) {
            ++cnt;
        }
    }
    return cnt;
}


char* get_filename_from_path(const char* path) {
    char* s = (char*)path;
    
    while(*s != '\0') {
        ++s;
    }

    while((*s != '/') && (s >= path)) {
        --s;
    }

    return s + 1;
}
