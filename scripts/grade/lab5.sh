#!/bin/bash

make="${MAKE:-make}"

#$make distclean
#$make defconfig
#$make build

RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
BOLD='\033[1m'
NONE='\033[0m'

# Path
grade_dir=$(dirname $(readlink -f "$0"))
expect_dir=$grade_dir/expects
root_dir=$grade_dir/../..
scripts_dir=$root_dir/scripts
config_dir=$scripts_dir/build

echo -e "${BOLD}===============${NONE}"
echo -e "${BLUE}Grading lab 5...(may take 20 seconds)${NONE}"

cd $root_dir
$make distclean > .build_log
cp $config_dir/lab5.config $root_dir/.config
$make build > .build_log 2>.build_stderr

$grade_dir/expects/lab5.exp
score=$?

echo -e "${BOLD}===============${NONE}"
echo -e "${GREEN}Score: $score/100${NONE}"
