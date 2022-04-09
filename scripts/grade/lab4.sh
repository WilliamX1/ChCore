#!/bin/bash

# Common Config
RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
BOLD='\033[1m'
NONE='\033[0m'
make="${MAKE:-make}"

LAB4_PART_NUM=9
TIME=$[$LAB4_PART_NUM*10+100]

# Path
grade_dir=$(dirname $(readlink -f "$0"))
expect_dir=$grade_dir/expects
root_dir=$grade_dir/../..
scripts_dir=$root_dir/scripts
config_dir=$scripts_dir/build

echo -e "${BOLD}===============${NONE}"
echo -e "${BLUE}Grading lab 4...(may take ${TIME} seconds)${NONE}"

score=0
cd $root_dir
$make distclean > .build_log

for i in `seq 1 $LAB4_PART_NUM`
do
    cp $config_dir/lab4-$i.config $root_dir/.config
    $make build > .build_log 2>.build_stderr
    $expect_dir/lab4-$i.exp
    score=$[$score+$?]
done

echo -e "${BOLD}===============${NONE}"
echo -e "${GREEN}Score: $score/100${NONE}"

rm .build_log
rm .build_stderr
