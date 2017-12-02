stm32-bluepill
===

ChibiOS demos on [stm32-bluepill](http://wiki.stm32duino.com/index.php?title=Blue_Pill), mainly on STM32F103C8T6(64k)

dependences:

* [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
* [ChibiOS](http://www.chibios.org)
* [stlink](https://github.com/texane/stlink)

howto:

1. configure env.sh to set the right path for toolchain and ChibiOS
2. `source env.sh`
3. `cd demo_folder`
4. `make` to build
5. `make flash` to write chips with st-flash
