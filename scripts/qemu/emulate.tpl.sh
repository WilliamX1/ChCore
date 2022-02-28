#!/bin/bash

set -e

basedir=$(dirname "$0")
# basedir should be /build directory

$basedir/../scripts/qemu/qemu_wrapper.sh \
    @qemu@ -gdb tcp::1234 @qemu_options@
