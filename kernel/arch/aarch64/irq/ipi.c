#include <irq/ipi.h>

void arch_send_ipi(u32 cpu, u32 ipi)
{
        plat_send_ipi(cpu, ipi);
}

int handle_ipi(u32 ipi)
{
        switch (ipi) {
        default:
                kwarn("Unknow IPI %d\n", ipi);
                return -1;
        }
}

void handle_ipi_request_to_local_cpu(struct ipi_data *data_self)
{
        /* Do nothing on AArch64 now. */
}
