# 如何开始
1. 安装工具链
   - 安装`gcc-i686-linux-gnu`, `gdb`, `cmake`
   ```shell
   sudo apt update
   sudo apt upgrade
   sudo apt install gcc-i686-linux-gnu
   sudo apt install gdb
   sudo apt install cmake
   ```
   - 安装qemu
   ```shell
   sudo apt install qemu-system-x86
   ```

2. 移植newlib

   - 安装`i686-elf-tools`工具链
   ```shell
   cd ~ && mkdir compiler && cd compiler
   wget https://github.com/lordmilko/i686-elf-tools/releases/download/7.1.0/i686-elf-tools-linux.zip
   mkdir i686-elf-tools && cd i686-elf-tools
   unzip ../i686-elf-tools-linux.zip
   mkdir newlib
   ```

   - 配置`环境变量`
   ```shell
   echo "export PATH=\"$HOME/compiler/i686-elf-tools/bin:$PATH\"" >> ~/.bashrc
   source ~/.bashrc
   ```

   - 下载`newlib-4.10.0`
   ```shell
   wget ftp://sourceware.org/pub/newlib/newlib-4.1.0.tar.gz
   ```
   
   - 编译`newlib`
   ```shell
   cd newlib-4.10.0
   mkdir build && build
   ../configure --target=i686-elf --prefix=${HOME}/compiler/i686-elf-tools/newlib \
                --disable-multilib --disable-newlib-io-float --disable-newlib-supplied-syscalls
   make all -j${nproc}
   make install
   ```
TODO:
3. 创建虚拟硬盘
   ```shell
   cd image
   ```

# 目录结构
``` shell
x86OS/
|
|-- .vscode/    # vscode相关配置文件
|
|-- image/      # 虚拟硬盘文件
|
|-- newlib/     # 轻量级标准C库
|
|-- script/     # 运行脚本
|
|-- source/
| |-- applib/
| |
| |-- boot/         # boot模块
| |
| |-- common/       # common模块, 封装了其余三个模块公用类型
| |
| |-- kernel/       # kernel模块, OS内核
| | |-- core/           # 内存管理等核心功能
| | |
| | |-- cpu/            # cpu抽象
| | |
| | |-- dev/            # 其余硬件抽象
| | |
| | |-- include/        # 头文件
| | |
| | |-- init/           # 内核初始化
| | |
| | |-- ipc/            # 进程间通信
| | |
| | |-- tools/          # kernel中的公用模块
| | |
| | |-- CMakeLists.txt
| | |
| | |-- kernel.lds      # 链接脚本
| |
| |-- loader/       # loader模块
| |
| |-- shell/
| | |-- cmd/
| | |
| | |-- include/
| |
| |-- snake/
|
|-- .gitignore
|
|-- CMakeList.txt
|
|-- README.md
```

# 