FROM debian:buster-slim

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install bzip2 make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root

COPY gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 .
RUN tar xjf gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 && rm gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2

ENV PATH /root/gcc-arm-none-eabi-6-2017-q2-update/bin:$PATH

# https://osdn.net/projects/chibios/scm/svn/tree/head/tags/ver17.6.4/
COPY chibios-tags_ver17.6.4-r13112.tar.gz .
RUN tar xzf chibios-tags_ver17.6.4-r13112.tar.gz \
    && mv chibios-tags_ver17.6.4-r13112/ver17.6.4 ChibiOS \
    && rm chibios-tags_ver17.6.4-r13112.tar.gz \
    && mkdir ChibiOS/demos/various/workspace

VOLUME /root/ChibiOS/demos/various/workspace
WORKDIR /root/ChibiOS/demos/various/workspace
