FROM debian:buster-slim

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install bzip2 make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root

COPY gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 .
RUN tar xjf gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 && rm gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2

ENV PATH /root/gcc-arm-none-eabi-6-2017-q2-update/bin:$PATH

COPY ChibiOS_17.6.4.tar.gz .
RUN tar xzf ChibiOS_17.6.4.tar.gz \
    && mv ChibiOS_17.6.4 ChibiOS \
    && rm ChibiOS_17.6.4.tar.gz \
    && mkdir ChibiOS/demos/various/workspace

VOLUME /root/ChibiOS/demos/various/workspace
WORKDIR /root/ChibiOS/demos/various/workspace
