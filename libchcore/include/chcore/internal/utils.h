#pragma once

#define ROUND_UP(x, n)   (((x) + (n)-1) & ~((n)-1))
#define ROUND_DOWN(x, n) ((x) & ~((n)-1))
