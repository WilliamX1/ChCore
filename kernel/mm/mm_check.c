#include <mm/mm_check.h>
#include <mm/buddy.h>
#include <mm/slab.h>

/*
 * Note that this function does not return the exact number of free memory in
 * the system because some other CPUs may allocate memory concurrently.
 *
 * Nevertheless, we use it for debugging now.
 */
u64 get_free_mem_size(void)
{
        u64 size;
        int i;

        size = get_free_mem_size_from_slab();
        for (i = 0; i < physmem_map_num; ++i) {
                size += get_free_mem_size_from_buddy(&global_mem[i]);
        }
        return size;
}
