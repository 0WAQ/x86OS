# 如何开始
1. 项目依赖
   - `qemu`
   - `gcc` 工具链
   - `vscode`, 可选
2. 创建虚拟硬盘
3. 使用 `vscode` 打开项目根目录
   - ...
4. 开始调试
   - ...

# 目录结构
``` shell
x86OS/
|
|-- .vscode/    # vscode相关配置文件
|
|-- image/      # 虚拟硬盘文件
|
|-- release/    # 生成的二进制或elf文件
|
|-- script/     # 运行脚本
|
|-- source/
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
| |
| |-- loader/       # loader模块
|
|
|-- .gitignore
|
|-- CMakeList.txt
|
|-- README.md
```

# 