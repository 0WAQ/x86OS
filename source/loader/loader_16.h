/**
 * 
 * 16位loader的头文件
 * 
 */
#ifndef LOADER_16_H
#define LOADER_16_H

/**
 * @brief 引导器入口函数，16位
 */
void loader_entry();

/**
 * @brief 内存检测函数，16位
 */
void detect_memory();

/**
 * @brief 进入保护模式，16位
 */
void enter_protect_mode();

/**
 * @brief 进入保护模式后的入口点，在start.S中定义，32位
 */
void protect_mode_entry();

/**
 * @brief 显示字符串，16位
 */
void show_msg(const char* msg);

#endif // LOADER_16_H