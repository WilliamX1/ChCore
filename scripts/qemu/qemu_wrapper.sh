#!/bin/bash

set -e

verlte() {
    [ "$1" = "$(echo -e "$1\n$2" | sort -V | head -n 1)" ]
}

verlt() {
    [ "$1" = "$2" ] && return 1 || verlte $1 $2
}

qemu=$1
shift
qemu_options=$@
qemu_version=$($qemu --version | head -n 1 | awk '{print $NF}')

if [[ "$qemu" == *"qemu-system-aarch64"* ]]; then
    if verlt $qemu_version 6.2.0; then
        # in qemu < 6.2.0, machine type = raspi3
        # in qemu >= 6.2.0, machine type = raspi3b
        qemu_options=$(echo $qemu_options | sed 's/-machine[ \t]\{1,\}raspi3b/-machine raspi3/g')
    fi
fi

$qemu $qemu_options
