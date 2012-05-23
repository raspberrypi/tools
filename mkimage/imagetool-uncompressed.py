#!/usr/bin/env python2

import os
import os.path
import re
import shutil
import sys

app = sys.argv.pop(0)
appdir = os.path.dirname(app)

bootloader_fn = os.path.join(appdir, "boot-uncompressed.txt")
atags_fn = os.path.join(appdir, "args-uncompressed.txt")

if len(sys.argv) < 1:
    print >>sys.stderr, "usage: imagetool-uncompressed.py kernel [image]";
    sys.exit(1)

kernel_fn = sys.argv.pop(0)
if len(sys.argv):
    image_fn = sys.argv.pop(0)
else:
    image_fn = "kernel.img"

mem = [0 for i in range(32768)]

def load_to_mem(name, addr):
   f = open(name)

   line = 0
   for l in f.readlines():
      line = line + 1
      try:
         semi = l.index(";")
         l = l[:semi]
      except:
         pass

      l = l.strip()
      if not l:
         continue

      try:
         value = int(l, 16)
      except:
         print >>sys.stderr, name + ":" + str(line) + ": syntax error"
         sys.exit(1)

      for i in range(4):
         mem[addr] = int(value >> i * 8 & 0xff)
         addr += 1

   f.close()

load_to_mem(bootloader_fn, 0x00000000)
load_to_mem(atags_fn, 0x00000100)

image_f = open(image_fn, "wb")

for m in mem:
   image_f.write(chr(m))

kernel_f = open(kernel_fn, "rb")
shutil.copyfileobj(kernel_f, image_f)
kernel_f.close()

image_f.close()
