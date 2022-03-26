#pragma once

#define MAP_SHARED  0x01
#define MAP_PRIVATE 0x02

#define MAP_ANONYMOUS 0x20

#define PROT_NONE  0
#define PROT_READ  1
#define PROT_WRITE 2
#define PROT_EXEC  4

#define PROT_CHECK_MASK (~(PROT_NONE | PROT_READ | PROT_WRITE | PROT_EXEC))
