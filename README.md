stm32-bluepill
===

ChibiOS demos on [stm32-bluepill](http://wiki.stm32duino.com/index.php?title=Blue_Pill), mainly on STM32F103C8T6(64k)

dependences:

* [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
* [ChibiOS](http://www.chibios.org)
* [stlink](https://github.com/texane/stlink)

howto:

0. clone the repo and `cd` in
1. `ln -s /path/to/ChiboOS ChibiOS` to create a symbol link to ChibiOS root
2. `ln -s /path/to/gcc-arm-none-eabi-X-Y gcc-arm-none-eabi` to create a symbol link to toolchain
3. `cd demo_folder`
4. `make` to build
5. `make flash` to write chips with st-flash
