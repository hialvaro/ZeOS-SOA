################################
##############ZeOS #############
################################
########## Makefile ############
################################

# package dev86 is required
AS86 = as86 -0 -a
LD86 = ld86 -0

HOSTCFLAGS = -Wall -Wstrict-prototypes -g
HOSTCC = gcc
CC = gcc
AS = as --32
LD = ld
OBJCOPY = objcopy -O binary -R .note -R .comment -S

INCLUDEDIR = include


CFLAGS = -m32 -O2  -g -fno-omit-frame-pointer -ffreestanding -Wall -I$(INCLUDEDIR) -fno-PIC
ASMFLAGS = -I$(INCLUDEDIR)
LDFLAGS = -g -melf_i386

SYSOBJ = \
	interrupt.o \
	entry.o \
	sys_call_table.o \
	io.o \
	sched.o \
	sys.o \
	mm.o \
	devices.o \
	utils.o \
	hardware.o \
	list.o \

LIBZEOS = -L . -l zeos

#add to USROBJ any object files required to complete the user program
USROBJ = \
	libc.o \
	# libjp.a \

all:zeos.bin

zeos.bin: bootsect system build user
	$(OBJCOPY) system system.out
	$(OBJCOPY) user user.out
	./build bootsect system.out user.out > zeos.bin

build: build.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

bootsect: bootsect.o
	$(LD86) -s -o $@ $<

bootsect.o: bootsect.s
	$(AS86) -o $@ $<

bootsect.s: bootsect.S
	$(CPP) $(ASMFLAGS) -traditional $< -o $@

entry.s: entry.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

sys_call_table.s: sys_call_table.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

user.o:user.c $(INCLUDEDIR)/libc.h

interrupt.o:interrupt.c $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h

io.o:io.c $(INCLUDEDIR)/io.h

sched.o:sched.c $(INCLUDEDIR)/sched.h

libc.o:libc.c $(INCLUDEDIR)/libc.h

mm.o:mm.c $(INCLUDEDIR)/types.h $(INCLUDEDIR)/mm.h

sys.o:sys.c $(INCLUDEDIR)/devices.h

utils.o:utils.c $(INCLUDEDIR)/utils.h


system.o:system.c $(INCLUDEDIR)/hardware.h system.lds $(SYSOBJ) $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/system.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/mm_address.h 


system: system.o system.lds $(SYSOBJ)
	$(LD) $(LDFLAGS) -T system.lds -o $@ $< $(SYSOBJ) $(LIBZEOS)

user: user.o user.lds $(USROBJ) 
	$(LD) $(LDFLAGS) -T user.lds -o $@ $< $(USROBJ)


clean:
	rm -f *.o *.s bochsout.txt parport.out system.out system bootsect zeos.bin user user.out *~ build 

disk: zeos.bin
	dd if=zeos.bin of=/dev/fd0

emul: zeos.bin
	bochs -q -f .bochsrc

gdb: zeos.bin
	bochs -q -f .bochsrc_gdb &
	gdb -x .gdbcmd system

emuldbg: zeos.bin
	bochs_nogdb -q -f .bochsrc
