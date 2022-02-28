#!/bin/sh

main() {
    for mod in "$@"
    do
        echo "Formatting \"$mod\" module"
        echo "[Round 1]"
        format $mod
        # run twice in case of clang-format bug
        echo "[Round 2]"
        format $mod
    done
}

format() {
    src="$1"
    find $src -type f -iname '*.h' -o -iname '*.c' -o -iname '*.cc' 2>/dev/null \
        | xargs clang-format -i -style=file --verbose
    # find $src -type f -iname '*.h' -o -iname '*.c' 2>/dev/null \
    #     | xargs echo
}

print_usage() {
    cat << EOF
Usage: ./scripts/format/format.sh [module1 [module2 ...]]

A "module" can be a directory or file.

Example:
    ./scripts/format/format.sh kernel/sched/sched.c
    ./scripts/format/format.sh userland/servers/procm userland/servers/tmpfs/main.c
EOF
}

if [ $# -eq 0 ]; then
    print_usage
    exit
fi

if [ -f /.dockerenv ]; then
    # we are in docker container
    main $@
else
    echo "Starting docker container to do formatting"
    docker run -it --rm \
        -u $(id -u ${USER}):$(id -g ${USER}) \
        -v $(pwd):/chos -w /chos \
        ipads/chcore_formatter:v1.0 \
        ./scripts/format/format.sh $@
fi
