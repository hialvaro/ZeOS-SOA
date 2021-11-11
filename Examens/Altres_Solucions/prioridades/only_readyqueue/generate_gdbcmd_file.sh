#!/bin/bash
# GDB uses .text section as the starting point for the code, calculating all the
# symbol addresses from there. The problem is that our user file has
# 2 sections: .text and .text.main, where the latter should be the starting point, but
# as GDB takes the former, the symbol resolution is broken.
# This script calculates the starting address of .text.main section and creates
# a gdb command file to load the user file object using the .text.main address as
# the .text section as it should be.
# Updates XXXXXX value in .gdbcmd.template file with this final address.
GDBFILE=.gdbcmd

# ========== GET .text.main address =========
TEXT=$(readelf -R .text user|grep 0x|head -1|cut -d' ' -f3)
echo ".text ADDRESS == $TEXT"

# ========== GENERATE output file =========
sed s/XXXXXX/$TEXT/ $GDBFILE.template > $GDBFILE
