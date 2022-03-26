__asm__(".text \n"
        ".global _start\n"
        ".type _start,%function\n"
        "_start:\n"
        "       mov x29, #0\n"
        "       mov x30, #0\n"
        "       mov x0, sp\n"
        "       bl __libchcore_init\n"
        "       b _start_c\n");
