FROM debian:buster-slim

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install bzip2 p7zip ca-certificates wget make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root

ADD https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2 .
RUN tar xjf gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2 && rm gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2

ENV PATH /root/gcc-arm-none-eabi-7-2018-q2-update/bin:$PATH

RUN wget https://osdn.net/dl/chibios/ChibiOS_18.2.2.7z \
    && 7zr x ChibiOS_18.2.2.7z \
    && mv ChibiOS_18.2.2 ChibiOS \
    && rm ChibiOS_18.2.2.7z \
    && mkdir ChibiOS/demos/various/workspace

RUN apt-get remove -y bzip2 p7zip ca-certificates wget && apt-get autoremove -y

VOLUME /root/ChibiOS/demos/various/workspace
WORKDIR /root/ChibiOS/demos/various/workspace
