/**
 * 
 * 系统公用宏定义
 * 
 */
#ifndef MACROS_H
#define MACROS_H

#define offset_of(container_type, member_name) \
        ((uint32_t)&((container_type*)0)->member_name)

#define container_of(member, container_type, member_name) \
    ( (container_type*)( (uint32_t)member - offset_of(container_type, member_name) ) )

#define entry_of(member, container_type, member_name) \
    ((container_type*)(member ? container_of(member, container_type, member_name) : 0))


#endif // MACROS_H