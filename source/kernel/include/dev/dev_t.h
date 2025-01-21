/**
 * 
 * 设备管理层基本类型
 * 
 */
#ifndef DEV_T_H
#define DEV_T_H

#define DEV_NAME_SIZE       (32)
#define DEV_TABLE_SIZE      (128)

enum {
    DEV_UNKNOWN = 0,
    DEV_TTY,
    DEV_DISK,
    DEV_TIMER,
};

struct _dev_desc_t;

/**
 * @brief 描述特定的设备
 */
typedef struct _device_t {
    struct _dev_desc_t* desc;   // 指向该种设备的描述
    int mode;
    int minor;                  // 次设备号
    void* data;
    int open_cnt;               // 打开的引用计数
}device_t;

/**
 * @brief 描述某种类型的设备
 */
typedef struct _dev_desc_t {
    char name[DEV_NAME_SIZE];
    int major;                  // 主设备号, 描述哪一种类型的设备
    
    int  (*open)(device_t*);
    int  (*read)(device_t*, int, char*, int);
    int  (*write)(device_t*, int, char*, int);
    int  (*control)(device_t*, int, int, int);
    void (*close)(device_t*);

}dev_desc_t;

#endif // DEV_T_H