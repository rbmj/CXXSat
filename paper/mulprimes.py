#!/usr/bin/env python
from sys import argv
print(hex(eval('0x%sL*0x%sL' % (argv[1], argv[2])))[2:-1])
