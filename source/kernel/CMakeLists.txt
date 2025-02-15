project(kernel LANGUAGES C)

# 使用自定义的链接器

set(CMAKE_EXE_LINKER_FLAGS "-m elf_i386 -T ${PROJECT_SOURCE_DIR}/kernel.lds")
set(CMAKE_C_LINK_EXECUTABLE "${LINKER_TOOL} <OBJECTS> ${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf")

# 注意保证init/kernel_start.S在最前头
# 以递归方式扫描，可以扫描子目录, 扫描到的是绝对路径
file(GLOB_RECURSE C_LIST "*.S" "*.c" "*.h")

# 生成可执行文件
add_executable(${PROJECT_NAME} init/kernel_start.S ${C_LIST} ../applib/lib_syscall.c)
target_compile_definitions(${PROJECT_NAME} PRIVATE FIRST_TASK)

# 不带调试信息的elf生成，何种更小，写入到image目录下
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD

                # 原本的参数是 -O binary，会将 kernel.elf 文件的格式全部丢掉
                # 有可能导致最后的二进制文件膨胀，并且不方便设置各个段的权限等信息
                # 
                # 若想读取elf，需添加elf相关结构
                   COMMAND ${OBJCOPY_TOOL} -S ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/image/${PROJECT_NAME}.elf
                   COMMAND ${OBJDUMP_TOOL} -x -d -S -m i386 ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_dis.txt
                   COMMAND ${READELF_TOOL} -a ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_elf.txt
)