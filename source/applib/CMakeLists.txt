
project(app LANGUAGES C)

file(GLOB C_LIST "*.S" "*.c" "*.h")
add_library(${PROJECT_NAME} ${C_LIST})

add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${OBJDUMP_TOOL} -x -d -S -m i386 ${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.a > ${PROJECT_NAME}_dis.txt
                   COMMAND ${READELF_TOOL} -a ${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.a > ${PROJECT_NAME}_elf.txt
)