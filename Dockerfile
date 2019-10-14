FROM debian:buster-slim

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install bzip2 p7zip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root
# ADD https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2
COPY gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 .
RUN tar xjf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 && mv gcc-arm-none-eabi-8-2019-q3-update gcc-arm-none-eabi && rm gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2

# ADD https://osdn.net/dl/chibios/ChibiOS_19.1.3.7z
COPY ChibiOS_19.1.3.7z .
RUN 7zr x ChibiOS_19.1.3.7z && mv ChibiOS_19.1.3 ChibiOS && rm ChibiOS_19.1.3.7z

RUN apt-get remove -y bzip2 p7zip
