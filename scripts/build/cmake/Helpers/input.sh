#!/bin/bash

# This script prints a prompt and read user input.
# Should only be used in `LoadConfigAsk.cmake`.

echo -n "$1 "
read -p "" input
echo $input >&2
