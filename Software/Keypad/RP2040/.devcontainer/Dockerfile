FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# ------------------------------------------------------------
# Base packages (NO ninja)
# ------------------------------------------------------------
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    wget \
    curl \
    ca-certificates \
    gcc-arm-none-eabi \
    gdb-multiarch \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    pkg-config \
    usbutils \
    && rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------
# Pico SDK
# ------------------------------------------------------------
WORKDIR /opt
RUN git clone --depth 1 https://github.com/raspberrypi/pico-sdk.git

ENV PICO_SDK_PATH=/opt/pico-sdk

RUN git -C /opt/pico-sdk submodule update --init --recursive

# ------------------------------------------------------------
# FreeRTOS Kernel
# ------------------------------------------------------------
WORKDIR /opt
RUN git clone --depth 1 https://github.com/FreeRTOS/FreeRTOS-Kernel.git freertos

ENV FREERTOS_KERNEL_PATH=/opt/freertos

# ------------------------------------------------------------
# Picotool (new version, no "load")
# ------------------------------------------------------------
WORKDIR /opt
RUN git clone --depth 1 https://github.com/raspberrypi/picotool.git && \
    mkdir picotool/build && \
    cd picotool/build && \
    cmake .. -G "Unix Makefiles" && \
    make -j$(nproc) && \
    make install

# ------------------------------------------------------------
# Force Makefiles globally (important)
# ------------------------------------------------------------
ENV CMAKE_GENERATOR="Unix Makefiles"

# ------------------------------------------------------------
# Workspace
# ------------------------------------------------------------
WORKDIR /workspaces/lighting_controller
