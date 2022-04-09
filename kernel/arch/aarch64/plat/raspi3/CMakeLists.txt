target_sources(${kernel_target} PRIVATE uart/uart.c irq/irq.c irq/timer.c
                                        mm/mmparse.c)
