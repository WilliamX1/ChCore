import sys
import os

test_list = ['badinst1.bin', 'badinst2.bin', 'fault.bin', 'hello.bin', 'putget.bin']
template_path = sys.argv[1]
number_count = int(sys.argv[2])
open(os.path.join(template_path, 'lab3.config'), 'w').write(open(os.path.join(template_path, 'lab3.config.template'), 'r').read().replace('$${{ROOT_PROGRAM}}$$', test_list[number_count]))