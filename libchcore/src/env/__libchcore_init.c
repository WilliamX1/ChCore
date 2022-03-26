#include <chcore/types.h>
#include <chcore/assert.h>
#include <chcore/internal/mem_layout.h>

/*
 * This is intended to be called in crt, before jumping
 * to the C entry point of libc (_start_c, _dlstart_c, etc.).
 *
 * It will read out pmo map addresses and caps at the start
 * of stack env page, then return the pointer of a standard
 * env page that can understood by unmodified libc code.
 */
long *__libchcore_init(long *p, int is_dyn_loader)
{
        if (*p != ENV_MAGIC) {
                /*
                 * It's not a ChCore env page, which happens when libc.so
                 * jump to the app's _start_c it loads. In this case, the
                 * globals initialized by this function have been initialized.
                 */
                return p;
        }
        p++;
        return p;
}
