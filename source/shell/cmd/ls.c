#include "macros.h"
#include "fs/file.h"

#include <stdio.h>
#include <string.h>

int do_ls(int argc, char** argv) {
    // 打开目录
    DIR* p = opendir("temp");
    if(p == NULL) {
        printf("open dir failed.\n");
        return -1;
    }

    // 遍历目录项
    struct dirent* entry;
    while((entry = readdir(p)) != NULL) {
        strlwr(entry->name);
        printf("%c %s %d\n", (entry->type == FILE_DIR ? 'd': 'f'), entry->name, entry->size);
    }

    // 关闭目录
    closedir(p);
    return 0;
}