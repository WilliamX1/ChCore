add_executable(helloworld.bin helloworld.c)
add_executable(lab5.bin lab5_main.c lab5_stdio.c)

chcore_copy_all_targets_to_ramdisk()
chcore_copy_files_to_ramdisk(test.txt)
