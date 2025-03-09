# x86OS: 一个基于Linux 0.11的x86操作系统
## 如何开始
1. 安装工具链
   - 安装`gcc-i686-linux-gnu`, `gdb`, `cmake`
   ```shell
   sudo dpkg --add-architecture i386

   sudo apt update

   sudo apt upgrade

   sudo apt install libc6-i386

   sudo apt install gcc-i686-linux-gnu

   sudo apt install gdb

   sudo apt install cmake
   
   sudo apt install texinfo
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

   - 下载`newlib-4.5.0`
   ```shell
   wget ftp://sourceware.org/pub/newlib/newlib-4.5.0.20241231.tar.gz

   tar xvf newlib-4.5.0.20241231.tar.gz
   ```
   
   - 编译`newlib`
   ```shell
   cd newlib-4.5.0.20241231

   mkdir build && build

   ../configure --target=i686-elf --prefix=${HOME}/compiler/i686-elf-tools/newlib \
   --disable-multilib --disable-newlib-supplied-syscalls --enable-newlib-long-time_t \
   --disable-newlib-io-float


   make all -j${nproc}

   make install
   ```
TODO:
3. 创建虚拟硬盘
   ```shell
   cd image
   ```