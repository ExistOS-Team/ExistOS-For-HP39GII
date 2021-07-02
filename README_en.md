# [ExistOS-For-HP39GII](https://github.com/Repeerc/LibreCalc-For-HP39GII)

An open source HP39GII firmware project

## Project brief

[![GPL Licence](https://badges.frapsoft.com/os/gpl/gpl.png?v=103)](https://opensource.org/licenses/GPL-3.0/)

This firmware project is created by a group of calculator enthusiasts, used some libraries such as [FreeRTOS kernel](https://github.com/FreeRTOS/FreeRTOS), [TinyUSB](https://github.com/hathach/tinyusb), [FatFs](http://elm-chan.org/fsw/ff/00index_e.html), [dhara](https://github.com/dlbeer/dhara)and etc. We very welcome some like-minded friends to try and improve the code for this project, and very want to get your valueable advices. Hoping for your participation.

## Current development status
- [x] Boot
- [x] Debugging serial
- [x] LCD screen driver
- [x] STMP3770 interrupt controller
- [x] Keyboard driver (GPIO polling)
- [x] Timer driver
- [x] CPU frequency setting
- [x] RTC
- [x] USB MSC mode
- [x] USB CDC(Virtual serial port) mode
- [x] USB HID keyboard mouse mode
- [x] USB dynamic configuration
- [x] FLASH Driver
- [x] FATFS
- [x] Multitasking
- [x] Virtual memory
- [x] Process, ELF loading
- [x] Minimal MicroPython implement
- [ ] Process switch, terminate, ELF unload
- [ ] GUI
- [ ] Power management
- [ ] Linux 5.4 half-virtualization dual-kernel implementation
- [ ] Firmware upgrade completely without offical upgrade tools

Current development status: Most of the OS kernel and some necessary hardware drivers have been implemented. But due to the lacking of knowledge about the upgrading method, we still need to use the offical upgrade tool in Windows 7/XP to flash the firmware. And no GUI provided, currently we are focusing on implementing virtual memory management and process loading. The GUI and so on are under discussion. If you have advice, please tell us at Issues.

## Compile/Flash

### CMake

1. Install arm-none-eabi-gcc. For linux, use the package manager; for Windows, download from [here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). For Windows,please add the ./bin in the install path to the PATH environment variable.

2. For Linux, please copy 99-hp39gii.rules to /etc/udev/rules.d/  
    and type
    ```bash
    sudo service udev restart
    # or
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ```

3. For non-Windows system, please compile /tools/sbtools first (no need to install)

4. For Windows, download [Ninja](https://github.com/ninja-build/ninja/releases) and add the .exe file to PATH. GNU Make is also supported but it is slower and harder to install.

5.  execute these commands in the terminal in this directory.
    ```bash
    mkdir ./build
    cd ./build
    cmake ..
    make (or ninja)   # Compile
    make flash (or ninja flash)   # Flash into RAM (Please completely poweroff the calculator(remove battery), then plug the usb cable while holding the ON/C button to enter flashing mode, before using this command)
    ```

Notice: Please install drivers of HP39GII by yourself.

## Code submit standard

**If you want to contibute code, please follow these standards**

1. Variable

    * Variables are named using lower camel case. For example, `windowHeight`
    * Function parameter naming is the same as variable.
    * Don't name using a single letter except for temporary or loop variable. 
    * It is prefered to add some meaningful prefix. For example, `p` means pointer.
    * Don't define variable and pointer at the same time on one line, such as `char *p, q;`

2. Function

    * Functions are named using underline. For example, `get_window_width`。
    * Function names should follow the Verb - object relationship.
    * It is prefered to add some meaningful prefix. For example, `is` means the type of return value is bool.
    * Short function can be defined as inline, function parameters and return values should be pointer instead of variable.
    * Avoid using recursion, consider refactoring to cycling.

3. Constant, macro and hardware-related

    * Constants and macros are named using underline to divide upper-case letters. For example, `MAX_WIDTH`。

4. Custom type

    * Custom type names are named using underline (tentative).
    * Must use typedef to define a custom type before using struct to define non-single-instance object.

5. Operator and other symbol

    * Unary operators should appress the variable, such as `c++`, `*p`.
    * Binary operators shold have spaces on both sides, such as `i == 1`, `a += 3`, except for `->`
    * Ternary are the same as binary, such as `isLeft ? 1 : 0`.
    * commas should be followed by a space.
    * Parentheses should be added appropriately where it is not easy to understand.

6. Pseudo-class

    If object-oriented is necessary, you can consider using `typedef struct` as pseudo-class.

    * Pseudo classes should be named by the upper camel case (Pascal) name.

    * For properties inside the class, its name is the same as the general variable.

    * Pseudo-class methods are not saved in pseudo-classes, but are global functions. Methods should be named using underline.

      * Normal method should be named as `ClassName_method_name`, the first parameter should always be a pointer to the instance and be named `this` (even unnecessary).

      * Static method should be named as`ClassName_static_method_name`。
      * General method named `ClassName_initializer` should be called right after an instance is defined.

7. Coding

    7.1

    ```c
    if (a == 1) {                          // There should be spaces between keywords and brackets, and spaces between brackets and curly brackets
        // code here
    }else{                                 // Use Java style
        // code here
    }
    if (b == 1) return;                    // When there is only one sentence of code in the block, you can leave a blank space without curly braces
    ```

    7.2

    ```c
    while (true)
        ;                                  // When using empty loops, you should wrap the semicolon and indent it
    ```
    7.3

    ​	(a) The loop variable of a general for loop is defined in the for loop:

    ```c
        for (int i = 0; i < l; i++) {
            // code here
        }
    ```

    ​	(b) In cases where circular variables are used externally, an initial value should also be given here:

    ```c
        int i;
        for (i = 0; i < l; i++) {
                // code here
        }
        return i;
    ```

    ​	(c) Do not leave any of the three of for loops empty: ` for ( ; ; ) `, otherwise use the while loop.

    7.4 Don't use assignment where a sentence needs to be judged, such as `if (a = 1), (a = 1) ? a : 0 `.

    7.5 The goto statement should be avoided as much as possible.

    7.6 Switch should be used more than else if.In a switch statement, it is best to have a break/return statement in each case, except when multiple cases share exactly the same piece of code.Care should be taken when using switch traversal and it is best to comment on it.

## Contributers



## License

[GPL-3.0](https://github.com/Repeerc/LibreCalc-For-HP39GII/blob/master/LICENSE)
