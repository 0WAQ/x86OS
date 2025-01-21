/**
 * 
 * 硬件定时器配置相关参数
 * 
 */
#ifndef TIME_T_H
#define TIME_T_H

// 定时器时钟
#define PIT_OSC_FREQ        1193182

// 定时器的寄存器和各项位配置
#define PIT_COMMAND_MODE_PORT   0x43
#define PIT_CHANNEL0_DATA_PORT  0x40

#define PIT_CHANNEL0            (0 << 6)
#define PIT_LOAD_LOHI           (3 << 4)
#define PIT_MODE3               (3 << 1)

#endif // TIME_T_H
