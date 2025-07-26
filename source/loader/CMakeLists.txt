project(loader LANGUAGES C)

set(CMAKE_EXE_LINKER_FLAGS "-m elf_i386 -Ttext=0x8000")
set(CMAKE_C_LINK_EXECUTABLE "${LINKER_TOOL} <OBJECTS> ${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf")

add_executable(${PROJECT_NAME} loader_start.S loader16.c loader32.c)

# 生成 bin 文件, 写入到 image 目录下
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${OBJCOPY_TOOL} -O binary ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/image/${PROJECT_NAME}.bin
    COMMAND ${OBJDUMP_TOOL} -x -d -S -m i8086 ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_dis.txt
    COMMAND ${READELF_TOOL} -a ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_elf.txt
)