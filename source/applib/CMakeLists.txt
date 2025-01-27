
project(app LANGUAGES C)

option(FIRST_TASK "Enable FIRST_TASK compilation" OFF)

if(FIRST_TASK)
    message(STATUS "app: Compling applib with FIRST_TASK")
    add_definitions(-DFIRST_TASK)
else()
    message(STATUS "app: Compling applib whitout FIRST_TASK")
endif()

file(GLOB C_LIST "*.S" "*.c" "*.h")
add_library(${PROJECT_NAME} ${C_LIST})

add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${OBJDUMP_TOOL} -x -d -S -m i386 ${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.a > ${PROJECT_NAME}_dis.txt
                   COMMAND ${READELF_TOOL} -a ${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.a > ${PROJECT_NAME}_elf.txt
)