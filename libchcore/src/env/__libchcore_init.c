#include <chcore/types.h>
#include <chcore/assert.h>
#include <chcore/internal/mem_layout.h>

int __chcore_procm_cap = -1;

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

        usize nr_caps = *p++;
        if (nr_caps == ENV_NO_CAPS) {
                nr_caps = 0;
        }

        if (nr_caps > 0) {
                /*
                 * NOTE: when this func is called before _dlstart_c, the GOT
                 * of libc.so is not filled, so that we cannot directly access
                 * global variables defined in libchcore, instead, we must add
                 * the load base of libc.so to the addresses of the globals.
                 */
                usize base = is_dyn_loader ? LIBC_SO_LOAD_BASE : 0;
                int *procm_cap_p = (int *)((char *)&__chcore_procm_cap + base);

                for (int i = 0; i < nr_caps; i++) {
                        switch (i) {
                        case 0:
                                *procm_cap_p = *p++;
                                break;
                        default:
                                chcore_bug("too many caps");
                        }
                }
        }

        return p;
}
