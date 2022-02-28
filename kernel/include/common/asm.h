#pragma once

#define BEGIN_FUNC(_name)        \
        .global _name;           \
        .type _name, % function; \
        _name:

#define END_FUNC(_name) .size _name, .- _name

#define EXPORT(symbol) \
        .globl symbol; \
        symbol:

#define LOCAL_DATA(x) \
        .type x, 1;   \
        x:

#define DATA(x)    \
        .global x; \
        .hidden x; \
        LOCAL_DATA(x)

#define END_DATA(x) .size x, .- x
