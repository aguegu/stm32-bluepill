stm32-bluepill
===

ChibiOS demos on stm32-bluepill, mainly on STM32F103C8T6(64k)

* http://wiki.stm32duino.com/index.php?title=Blue_Pill
* http://www.vcc-gnd.com/rtd/html/STM32/quickref.html

dependences:

* [stlink](https://github.com/texane/stlink)
* [docker](https://docs.docker.com/)
* ~~[GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)~~
* ~~[ChibiOS](http://www.chibios.org)~~

### How to build the source

1. `docker pull aguegu/chibios`
2. clone the repo and `cd` into it
3. `cd demo_folder`
4. `make` to build
5. `make flash` to write chips with st-flash

### aguegu/chibios

It is a slim docker image repo contains ChibiOS (now 19.1.3) and gcc-arm-none-eabi (now 8-2019-q3-update) based on Debian on AMD64 platform.

With this image, all the building/compiling work is done within the container.

You may also copy the demo project from the ChibiOS demos, like `NIL-STM32F100-DISCOVERY`, then `cd` into it

1. rename the origin makefile to `chibios.makefile`, like `mv makefile chibios.makefile`
2. copy `Makefile` from one of the demo project of this repo
3. then this folder can work like any other demos in this repo

In this way, the host system is clean and free of the mess of cross-compiling.

### Tricky:

* can not read only 1 byte from stm32f1 i2c (DMA).
* chThdSleep value should not be 0.
