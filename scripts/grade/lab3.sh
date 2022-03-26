#!/bin/bash

make="${MAKE:-make}"

RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
BOLD='\033[1m'
NONE='\033[0m'
LAB3_PART_NUM=5

grade_dir=$(dirname $(readlink -f "$0"))
expect_dir=$grade_dir/expects
root_dir=$grade_dir/../..
scripts_dir=$root_dir/scripts
config_dir=$scripts_dir/build
timeout=10
grade_dir=$(dirname $0)

echo -e "${BOLD}===============${NONE}"
echo -e "${BLUE}Grading lab 3...(may take 90 seconds)${NONE}"

score=0
cd $root_dir
# $make distclean > .build_log

$make build > .build_log 2>.build_stderr
$make qemu > $grade_dir/.lab3.grade.1 &
lastpid=$!
sleep $timeout
killall -15 qemu-system-aarch64 1>/dev/null 2>/dev/null

python3 $grade_dir/generator_lab3.py $config_dir 0
cp $config_dir/lab3.config $root_dir/.config
$make build > .build_log 2>.build_stderr
$make qemu > $grade_dir/.lab3.grade.2 &
lastpid=$!
sleep $timeout
killall -15 qemu-system-aarch64 1>/dev/null 2>/dev/null

python3 $grade_dir/generator_lab3.py $config_dir 1
cp $config_dir/lab3.config $root_dir/.config
$make build > .build_log 2>.build_stderr
$make qemu > $grade_dir/.lab3.grade.3 &
lastpid=$!
sleep $timeout
killall -15 qemu-system-aarch64 1>/dev/null 2>/dev/null

python3 $grade_dir/generator_lab3.py $config_dir 2
cp $config_dir/lab3.config $root_dir/.config
$make build > .build_log 2>.build_stderr
$make qemu > $grade_dir/.lab3.grade.4 &
lastpid=$!
sleep $timeout
killall -15 qemu-system-aarch64 1>/dev/null 2>/dev/null

python3 $grade_dir/generator_lab3.py $config_dir 3
cp $config_dir/lab3.config $root_dir/.config
$make build > .build_log 2>.build_stderr
$make qemu > $grade_dir/.lab3.grade.5 &
lastpid=$!
sleep $timeout
killall -15 qemu-system-aarch64 1>/dev/null 2>/dev/null

python3 $grade_dir/generator_lab3.py $config_dir 4
cp $config_dir/lab3.config $root_dir/.config
$make build > .build_log 2>.build_stderr
echo b > $grade_dir/.lab3.input
$make qemu < $grade_dir/.lab3.input > $grade_dir/.lab3.grade.6 &
lastpid=$!
sleep $timeout
killall -15 qemu-system-aarch64 1>/dev/null 2>/dev/null

python3 $grade_dir/judger_lab3.py $grade_dir/.lab3.grade

rm $grade_dir/.lab3.grade.*
rm .build_log
rm .build_stderr
