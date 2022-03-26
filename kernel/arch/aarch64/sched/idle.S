#include <common/asm.h>
#include <common/vars.h>

BEGIN_FUNC(idle_thread_routine)
idle:   wfi
        b idle
END_FUNC(idle_thread_routine)
