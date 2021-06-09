# [ExistOS-For-HP39GII](https://github.com/Repeerc/LibreCalc-For-HP39GII)

[English readme](./README_en.md)

一个开源的HP39GII固件项目

## 简介

[![GPL Licence](https://badges.frapsoft.com/os/gpl/gpl.png?v=103)](https://opensource.org/licenses/GPL-3.0/)

本固件项目由一群计算器爱好者始创，使用了[FreeRTOS kernel](https://github.com/FreeRTOS/FreeRTOS)、[TinyUSB](https://github.com/hathach/tinyusb)、[FatFs](http://elm-chan.org/fsw/ff/00index_e.html)、[dhara](https://github.com/dlbeer/dhara)等库。我们非常欢迎同好试用和改善本项目的代码，也非常乐意听取您的宝贵意见。期待您的参与！
## 目前工作进展
- [x] Boot
- [x] 调试串口
- [x] LCD 256级灰度 屏幕驱动
- [x] STMP3770中断控制器
- [x] 键盘驱动（GPIO轮询）
- [x] 定时器驱动
- [x] CPU频率设定
- [x] RTC时钟
- [x] USB大容量存储模式
- [x] USB串口控制台
- [x] FLASH驱动
- [x] FATFS文件系统
- [x] 多任务
- [x] 虚拟内存
- [x] 进程、ELF加载
- [x] 最小化MicroPython实现
- [ ] 进程切换、进程结束、ELF卸载
- [ ] 用户界面GUI
- [ ] 电源管理
- [ ] Linux 5.4 半虚拟化双内核实现
- [ ] 完全脱离官方刷机工具的固件升级

目前工作进展：已经完成了操作系统大部分内核和一些必要的硬件驱动。但由于未弄清STMP3770固件刷入机制，因此需要在Win7\WinXP环境下用计算器官方的固件升级软件刷入本固件。同时还没有用户界面，现在的工作重心是实现虚拟内存管理和进程加载。关于用户界面GUI等还在讨论中，如有宝贵意见可于Issues提出。


## 固件编译/安装

### CMake

1. 安装 arm-none-eabi-gcc. Linux系统可以用包管理器安装，Windows系统可以从[这里](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)下载. 对于Windows系统,请将安装目录下的./bin路径添加至PATH环境变量.

2. 对于Linux系统, 请将99-hp39gii.rules复制到/etc/udev/rules.d/  
    之后执行
    ```bash
    sudo service udev restart
    # 或者
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ```

3. 对于非Windows系统, 请先编译 /tools/sbtools (无需安装)

4. 对于Windows系统, 下载[Ninja](https://github.com/ninja-build/ninja/releases)并将exe文件添加至PATH变量. 也可以使用GNU Make, 但是编译速度会慢一点并且安装会麻烦一点。

5. 在此文件夹中, 执行
    ```bash
    mkdir ./build
    cd ./build
    cmake .. （或cmake .. -G Ninja）
    make (或ninja)   #编译固件
    make flash (或ninja flash)   #将固件刷入RAM (刷入前请将计算器完全断电(抠电池), 之后在按住ON/C按键的同时插入USB线，即可进入刷写模式)
    ```

注意：HP39GII的相关驱动程序请自行安装。

## 代码提交规范

**如果您想贡献代码，请遵守以下规范**

1. 变量

    * 变量采用小驼峰命名法命名。例如`windowHeight`。
    * 函数参数的命名与变量相同。
    * 不应使用单个字符命名。临时变量，循环变量除外，允许使用i，j，k等。
    * 可考虑在名称前加适当有意义的前缀，如p代表指针。
    * 不得在一行内同时定义变量和指针，如 `char *p, q;`。

2. 函数

    * 函数采用下划线命名法命名。例如`get_window_width`。
    * 函数的命名应遵循动宾关系。
    * 可考虑在名称前加适当有意义的前缀，如is代表返回值为bool类型。
    * 短小的函数可以定义为inline，函数的参数和返回值应尽量使用指针而非变量。
    * 应尽量避免使用递归，而应考虑重构为循环。

3. 常量，宏及硬件相关

    * 常量及宏采用下划线分隔大写字母的方式命名。例如`MAX_WIDTH`。

4. 自定义类型

    * 自定义类型采用下划线命名法命名（暂定）。
    * 用struct定义非单例对象时，必须使用typedef语句先定义自定义类型。

5. 运算符及其他符号

    * 一元运算符应紧贴变量，如`c++`，`*p`。
    * 二元运算符两侧都应空格，如`i == 1`，`a += 3`。例外：->运算符。
    * 三元运算符同二元运算符，如`isLeft ? 1 : 0`。
    * 逗号后应空格。
    * 在不易理解的地方应适当加注括号。

6. 伪类

    若确有面向对象的必要的，可以考虑用`typedef struct`实现伪类。

    * 伪类采用大驼峰（帕斯卡）命名法命名。

    * 伪类中保存属性，其命名与一般变量相同。

    * 伪类的方法不保存在伪类中，而采用全局函数。方法采用下划线法命名。

      * 一般方法命名为`ClassName_method_name`，其第一个参数始终应为一个指向该类型对象的指针并命名为`this`（即使不需要）。

      * 静态方法命名为`ClassName_static_method_name`。
      * 命名为`ClassName_initializer`的一般方法应在伪类的对象定义后立即调用。

7. 代码部分

    7.1

    ```c
    if (a == 1) {                          // 关键字与括号间应空格，括号与花括号间应空格
        // code here
    }else{                                 // 采用Java风格
        // code here
    }
    if (b == 1) return;                    // 块中只有一句代码时，可以不打花括号并不换行，空一格即可
    ```

    7.2

    ```c
    while (true)
        ;                                  // 使用空循环时，应将分号换行并缩进
    ```
    7.3

    ​	(a) 一般for循环的循环变量定义在for循环中：

    ```c
        for (int i = 0; i < l; i++) {
            // code here
        }
    ```

    ​	(b) 外部使用循环变量的情况，也应在此处赋初值：

    ```c
        int i;
        for (i = 0; i < l; i++) {
                // code here
        }
        return i;
    ```

    ​	(c) 不得将for循环此三处中任意一处空出：` for ( ; ; ) `，否则请使用while循环。

    7.4 禁止在需要判断语句的地方进行赋值操作，如 `if (a = 1)，(a = 1) ? a : 0 `等。

    7.5 应尽量避免使用goto语句。

    7.6 应多用switch，少用else if。switch语句中，每一个case中都最好有一个break/return语句，多个case共用完全相同的一段代码时除外。使用switch穿越时应分外小心并最好加注注释。



## 贡献者



## 协议

[GPL-3.0](https://github.com/Repeerc/LibreCalc-For-HP39GII/blob/master/LICENSE)
