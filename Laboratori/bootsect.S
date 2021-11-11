!
!	bootsect.s    
!
! bootsect.s � la versi�simplificada del bootsect.s original de Linux.
! L'arxiu compilat hauria d'ocupar, sense les capceleres, exactament 512
! bytes, que � la longitud del sector 0 d'un disquet.

#define SEC_PISTA            18	! Modificar en cas d'utilitzar disquets
				! diferents de 1.44Mb:
				! 38: 2.88Mb; 18: 1.44Mb; 15: 1.2Mb; 9: 720kb
.text

SETUPSECS = 0			! default nr of setup-sectors
BOOTSEG   = 0x07C0		! original address of boot-sector
INITSEG   = 0x9000		! final address of boot-sector
SYSSEG    = 0x1000		! kernel (protected-mode) address
SYSSIZE   = 0x7F00		! kernel size
KERNEL_CS = 0x10

! ROOT_DEV & SWAP_DEV are now written by "build".
SWAP_DEV = 0

! ld86 requires an entry symbol. This may as well be the usual one.
.globl	_main
_main:
!
! ZEUS: 1. Movemos el sector 0 a la direccion 0x90000
!
	mov	ax,#BOOTSEG
	mov	ds,ax
	mov	ax,#INITSEG
	mov	es,ax
	mov	cx,#256
	sub	si,si
	sub	di,di
	cld
	rep
	movsw			! Moving boot sector from 0x07C00 to 0x90000
	jmpi	go,INITSEG

! ZEUS: Ejecutamos esto ya en la direccion 0x90000+go
! ax and es already contain INITSEG

go:	mov	di,#0x4000-12	! 0x4000 is arbitrary value >= length of
				! bootsect + length of setup + room for stack
				! 12 is disk parm size

!
! ZEUS: 2. Configuramos la pila de usuario en la direcci� 0x3ff40
!
	mov	ds,ax
	mov	ss,ax		! put stack at INITSEG:0x4000-12.
	mov	sp,di

!
! ZEUS: 3. Imprimimos el mensaje "Carregant"
!

	mov	ah,#0x03	! read cursor pos
	xor	bh,bh
	int	0x10

	mov	cx,#0xB
	mov	bx,#0x0007	! page 0, attribute 7 (normal)
	mov	bp,#msg1
	mov	ax,#0x1301	! write string, move cursor
	int	0x10
	mov	ah,#0x01	! Set cursor lines
	mov	ch,#0x30	! Invalid upper value
	int	0x10		! Delete cursor

! ok, we've written the message, now
! we want to load the system (at 0x10000)
!
! ZEUS: 4. Cargamos el sistema a partir de la 0x10000 utilizando una
!          rutina de la BIOS.
!
	mov	ax,#SYSSEG
	mov	es,ax		! segment of 0x010000
	call	read_it
	call	kill_motor

! Now we want to move to protected mode ...
!   This is the default real mode switch routine.
!   to be called just before protected mode transition
	cli			! no interrupts allowed !
	mov	al,#0x80	! disable NMI for the bootup sequence
	out	#0x70,al

! Now we enable A20
	call	empty_8042
	mov	al,#0xD1	! command write
	out	#0x64,al
	call	empty_8042
	mov	al,#0xDF	! A20 on
	out	#0x60,al
	call	empty_8042

! wait until a20 really *is* enabled; it can take a fair amount of
! time on certain systems; Toshiba Tecras are known to have this
! problem.  The memory location used here is the int 0x1f vector,
! which should be safe to use; any *unused* memory location < 0xfff0
! should work here.

#define	TEST_ADDR 0x7c

	push	ds
	xor	ax,ax		! segment 0x0000
	mov	ds,ax
	dec	ax		! segment 0xffff (HMA)
	mov	gs,ax
	mov	bx,[TEST_ADDR]	! we want to restore the value later
a20_wait:
	inc	ax
	mov	[TEST_ADDR],ax
	seg	gs
	cmp	ax,[TEST_ADDR+0x10]
	je	a20_wait	! loop until no longer aliased
	mov	[TEST_ADDR],bx	! restore original value
	pop	ds

! well, that went ok, I hope. Now we have to reprogram the interrupts :-(
! we put them right after the intel-reserved hardware interrupts, at
! int 0x20-0x2F. There they won't mess up anything. Sadly IBM really
! messed this up with the original PC, and they haven't been able to
! rectify it afterwards. Thus the bios puts interrupts at 0x08-0x0f,
! which is used for the internal hardware interrupts as well. We just
! have to reprogram the 8259's, and it isn't fun.

	mov	al,#0x11	! initialization sequence
	out	#0x20,al	! send it to 8259A-1
	call	delay
	out	#0xA0,al	! and to 8259A-2
	call	delay
	mov	al,#0x20	! start of hardware int's (0x20)
	out	#0x21,al
	call	delay
	mov	al,#0x28	! start of hardware int's 2 (0x28)
	out	#0xA1,al
	call	delay
	mov	al,#0x04	! 8259-1 is master
	out	#0x21,al
	call	delay
	mov	al,#0x02	! 8259-2 is slave
	out	#0xA1,al
	call	delay
	mov	al,#0x01	! 8086 mode for both
	out	#0x21,al
	call	delay
	out	#0xA1,al
	call	delay
	mov	al,#0xFF	! mask off all interrupts for now
	out	#0xA1,al
	call	delay
	mov	al,#0xFB	! mask all irq's but irq2 which
	out	#0x21,al	! is cascaded

! set DS=CS, we know that SETUPSEG == CS at this point
	mov	ax,cs
	mov	ds,ax
!
! ZEUS: Carreguem els valors de les taules IDT i GDT
!
	lidt	idt_48		! load idt with 0,0
	lgdt	gdt_48		! load gdt with whatever appropriate

! Well, now's the time to actually move into protected mode. To make
! things as simple as possible, we do no register set-up or anything,
! we let the GNU-compiled 32-bit programs do that. We just jump to
! absolute address 0x1000 (or the loader supplied one),
! in 32-bit protected mode.
!
! Note that the short jump isn't strictly needed, although there are
! reasons why it might be a good idea. It won't hurt in any case.

	mov	eax,#1		! protected mode (PE) bit
	mov	cr0,eax		! a partir del i386 aix�ja � possible fer-ho

! Well, that certainly wasn't fun :-(. Hopefully it works, and we don't
! need no steenking BIOS anyway (except for the initial loading :-).
! The BIOS routine wants lots of unnecessary data, and it's less
! "interesting" anyway. This is how REAL programmers do it.

! NOTE: For high loaded big kernels we need a
!	jmpi    0x100000, KERNEL_CS
!
!	but we yet haven't reloaded the CS register, so the default size
!	of the target offset still is 16 bit.
!       However, using an operant prefix (0x66), the CPU will properly
!	take our 48 bit far pointer. (INTeL 80386 Programmer's Reference
!	Manual, Mixing 16-bit and 32-bit code, page 16-6)
!
! ZEUS: Saltamos al main()!!
!
	db	0x66,0xea	! prefix + jmpi-opcode
code32:	dd	0x10010		! will be set to 0x100000 for big kernels
	dw	KERNEL_CS

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! ZeOS Loading finishes here!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

! This routine loads the system at address 0x10000, making sure
! no 64kB boundaries are crossed. We try to load it as fast as
! possible, loading whole tracks whenever we can.
!
! in:	es - starting address segment (normally 0x1000)
!
sread:	.word 0			! sectors read of current track
head:	.word 0			! current head
track:	.word 0			! current track

read_it:
	mov	al,setup_sects	! 4 sectors
	inc	al
	mov	sread,al
	mov ax,es
	test ax,#0x0fff
die:	jne die			! es must be at 64kB boundary
	xor bx,bx		! bx is starting address within segment
rp_read:
	mov ax,es
	sub ax,#SYSSEG
	cmp ax,syssize		! have we loaded all yet?
	jbe ok1_read
	ret
ok1_read:
	mov ax,sectors
	sub ax,sread
	mov cx,ax
	shl cx,#9
	add cx,bx
	jnc ok2_read
	je ok2_read
	xor ax,ax
	sub ax,bx
	shr ax,#9
ok2_read:
	call read_track
	mov cx,ax
	add ax,sread
	cmp ax,sectors
	jne ok3_read
	mov ax,#1
	sub ax,head
	jne ok4_read
	inc track
ok4_read:
	mov head,ax
	xor ax,ax
ok3_read:
	mov sread,ax
	shl cx,#9
	add bx,cx
	jnc rp_read
	mov ax,es
	add ah,#0x10
	mov es,ax
	xor bx,bx
	jmp rp_read

read_track:
	pusha
	pusha
	mov	ax, #0xe2e 	! loading... message 2e = .
	mov	bx, #7
 	int	0x10
	popa

	mov	dx,track
	mov	cx,sread
	inc	cx
	mov	ch,dl
	mov	dx,head
	mov	dh,dl
	and	dx,#0x0100
	mov	ah,#2

	push	dx		! save for error dump
	push	cx
	push	bx
	push	ax

	int	0x13
	add	sp, #8
	popa
	ret

/*
 * This procedure turns off the floppy drive motor, so
 * that we enter the kernel in a known state, and
 * don't have to worry about it later.
 */
kill_motor:
	push dx
	mov dx,#0x3f2
	xor al, al
	outb
	pop dx
	ret

empty_8042:
       push    ecx
       mov     ecx,#0xFFFFFF

empty_8042_loop:
       dec     ecx
       jz      empty_8042_end_loop

	call	delay
	in	al,#0x64	! 8042 status port
	test	al,#1		! output buffer?
	jz	no_output
	call	delay
	in	al,#0x60	! read it
	jmp	empty_8042_loop
no_output:
	test	al,#2		! is input buffer full?
	jnz	empty_8042_loop	! yes - loop
empty_8042_end_loop:
        pop     ecx
	ret

!
! Delay is needed after doing I/O
!
  delay:
  .word 0x00eb			! jmp $+2
ret

sectors:
	.word SEC_PISTA  ! 18 sectores por pista: t�ico disquet de 1.44Mb

msg1:
	.byte 13,10
	.ascii "Loading.."

.org 435
gdt:		/* GLOBAL DESCRIPTOR TABLE */
  .word  0,0,0,0          ! dummy
                                 /*    Table Indicator RPL */
  .word  0,0,0,0          ! sin usar
      /* pointed by __KERNEL_CS = 0x10 = 0000 0000 0001 0 0 00 */
  .word  0xFFFF           ! 4Gb - (0x100000*0x1000 = 4Gb)
  .word  0x0000           ! base address=0
  .byte  0x00,0x9a        ! code read/exec
  .byte  0xcf,0x00        ! granularity = 4096, 386 (+5th nibble of limit)
    /* pointed by __KERNEL_DS = 0x18 = 0000 0000 0001 1 0 00 */
  .word  0xFFFF           ! 4Gb - (0x100000*0x1000 = 4Gb)
  .word  0x0000           ! base address = 00 03 2000
  .byte  0x00,0x92        ! data read/write
  .byte  0xcf,0x00        ! granularity = 4096, 386 (+5th nibble of limit)
      /* pointed by __USER_CS = 0x23 = 0000 0000 0001 0 0 11 */
  .word  0xFFFF           ! 4Gb - (0x100000*0x1000 = 4Gb)
  .word  0x0000           ! base address=0
  .byte  0x00,0xfa        ! code read/exec
  .byte  0xcf,0x00        ! granularity = 4096, 386 (+5th nibble of limit)
    /* pointed by __USER_DS = 0x2B = 0000 0000 0001 1 0 11 */
  .word  0xFFFF           ! 4Gb - (0x100000*0x1000 = 4Gb)
  .word  0x0000           ! base address = 00 03 2000
  .byte  0x00,0xf2        ! data read/write
  .byte  0xcf,0x00        ! granularity = 4096, 386 (+5th nibble of limit)
  /*TSS*/
  .word  104,0
  .byte  0,0x89,0,0

idt_48:
  .word  0,0,0

/*.org 497*/
setup_sects:
	.byte SETUPSECS
	.word	0	!dummy
syssize:
	.word SYSSIZE
gdt_48:
  .word  0x8000		! gdt limit = 2048, 256 GDT entries
gdt_addr:
  .word  gdt,0x9	! gdt base  = 0X9xxxx
	.word	0	!dummy
boot_flag:
	.word 0xAA55
