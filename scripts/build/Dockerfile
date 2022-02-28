# Dockerfile for ipads/chcore_builder.

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai
RUN apt-get update && \
        \
        apt-get install -y \
        cmake=3.16.* \
        cmake-curses-gui=3.16.* \
        make \
        ninja-build \
        cpio \
        binutils \
        binutils-aarch64-linux-gnu \
        binutils-riscv64-linux-gnu \
        gcc=4:9.3.* \
        gcc-aarch64-linux-gnu=4:9.3.* \
        gcc-riscv64-linux-gnu=4:9.3.* \
        g++=4:9.3.* \
        g++-aarch64-linux-gnu=4:9.3.* \
        g++-riscv64-linux-gnu=4:9.3.* \
        grub-common \
        grub-pc-bin \
        xorriso && \
        \
        apt-get clean && \
        rm -rf /var/lib/apt/lists/*
