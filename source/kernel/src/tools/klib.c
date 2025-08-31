#include "tools/klib.h"
#include "tools/log.h"
#include "common/asm.h"

void ksprintf(char* buf, const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kvsprintf(buf, fmt, args);
    va_end(args);    
}

void kvsprintf(char* buf, const char* fmt, va_list args) {
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
                int len = kstrlen(str);
                while(len--) {
                    *cur++ = *str++;
                }
                break;
            }
            case 'd':   // 整型
            {
                int num = va_arg(args, int);
                kitoa(cur, num, 10);
                cur += kstrlen(cur);
                break;
            }
            case 'x':   // 十六进制数
            {
                int num = va_arg(args, int);
                kitoa(cur, num, 16);
                cur += kstrlen(cur);
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

void kitoa(char* dst, int num, u32_t base) {
    char* p = dst;
    char* start = dst;
    
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

void kstrcpy(char* dst, const char* src) {
    if(!dst || !src) {
        return;
    }
    while(*dst && *src) {
        *dst++ = *src++;
    }
    *dst = '\0';
}

void kstrncpy(char* dst, const char* src, int len) {
    if(!dst || !src || !len) {
        return;
    }

    char* d = dst;
    const char* s = src;
    while(len-- > 0 && *s) {
        *d++ = *s++;
    }

    if(len == 0) {
        *(d - 1) = '\0';
    }
    else {
        *d = '\0';
    }
}

int kstrcmp(const char* s1, const char* s2) {
    ASSERT(s1 != NULL && s2 != NULL);

    while(*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int kstrncmp(const char* s1, const char* s2, int len) {
    ASSERT(s1 != NULL && s2 != NULL);

    if (len <= 0) {
        return 0;
    }

    while (len--) {
        if (*s1 == '\0' || *s2 == '\0' || *s1 != *s2) {
            return (unsigned char)*s1 - (unsigned char)*s2;
        }
        s1++;
        s2++;
    }

    return 0;
}

int kstrlen(const char* str) {
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

void kmemcpy(void* dst, const void* src, int len) {
    if(!dst || !src) {
        return;
    }

    u8_t* s = (u8_t*)src;
    u8_t* d = (u8_t*)dst;
    while(len--) {
        *d++ = *s++;
    }
}

void kmemset(void* dst, u8_t v, int len) {
    if(!dst || !len) {
        return;
    }

    u8_t* d = (u8_t*)dst;
    while(len--) {
        *d++ = v;
    }
}

int kmemcmp(void* d1, void* d2, int len) {
    if(!d1 || !d2 || !len) {
        return 1;
    }

    u8_t* p1 = (u8_t*)d1;
    u8_t* p2 = (u8_t*)d2;
    while(len--) {
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


u32_t strings_count(char** addr) {
    u32_t cnt = 0;
    if(addr) {
        while(*addr++) {
            ++cnt;
        }
    }
    return cnt;
}


const char* get_filename_from_path(const char* path) {
    const char* s = path;
    while(*s != '\0') {
        ++s;
    }
    while((*s != '/') && (s >= path)) {
        --s;
    }
    return s + 1;
}
