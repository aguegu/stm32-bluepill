FROM debian:buster-slim

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install bzip2 p7zip ca-certificates wget make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root

ADD https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 .
RUN tar xjf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 && mv gcc-arm-none-eabi-8-2019-q3-update gcc-arm-none-eabi && rm gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2

ENV PATH /root/gcc-arm-none-eabi/bin:$PATH

RUN wget https://osdn.net/dl/chibios/ChibiOS_19.1.3.7z \
    && 7zr x ChibiOS_19.1.3.7z \
    && mv ChibiOS_19.1.3 ChibiOS \
    && rm ChibiOS_19.1.3.7z \
    && mkdir ChibiOS/demos/various/workspace

RUN apt-get remove -y bzip2 p7zip ca-certificates wget && apt-get autoremove -y

VOLUME /root/ChibiOS/demos/various/workspace
WORKDIR /root/ChibiOS/demos/various/workspace

