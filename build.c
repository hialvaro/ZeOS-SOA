/*
 *  $Id: build.c,v 1.1.1.1 2006-09-04 15:05:34 jcosta Exp $
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *  Copyright (C) 1997 Martin Mares
 *  2003 Modificat per Zeus Gómez per insertar el codi d'usuari
 *       també a la imatge del nucli en espai d'adreces separat.
 */

/*
 * This file builds a disk-image from three different files:
 *
 * - bootsect: exactly 512 bytes of 8086 machine code, loads the rest
 * - setup: 8086 machine code, sets up system parm
 * - system: 80386 code for actual system
 *
 * It does some checking that all files are of the correct type, and
 * just writes the result to stdout, removing headers and padding to
 * the right amount. It also writes some system data to stderr.
 */

/*
 * Changes by tytso to allow root device specification
 * High loaded stuff by Hans Lermen & Werner Almesberger, Feb. 1996
 * Cross compiling fixes by Gertjan van Wingerde, July 1996
 * Rewritten by Martin Mares, April 1997
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <fcntl.h>

/* Don't touch these, unless you really know what you're doing. */
#define DEF_INITSEG	0x9000
#define DEF_SYSSEG	0x1000
#define DEF_SETUPSEG	0x9020
#define DEF_SYSSIZE	0x7F00

typedef unsigned char byte;
typedef unsigned short word;
typedef  u_int32_t u32;

#define DEFAULT_MAJOR_ROOT 0
#define DEFAULT_MINOR_ROOT 0

/* Minimal number of setup sectors (see also bootsect.S) */

byte buf[1024];
int fd;

void die(const char * str, ...)
{
	va_list args;
	va_start(args, str);
	vfprintf(stderr, str, args);
	fputc('\n', stderr);
	exit(1);
}

/* Reading of ld86 output (Minix format) */

#define MINIX_HEADER_LEN 32

void minix_open(const char *name)
{
	static byte hdr[] = { 0x01, 0x03, 0x10, 0x04, 0x20, 0x00, 0x00, 0x00 };
	static u32 *lb = (u32 *) buf;

	if ((fd = open(name, O_RDONLY, 0)) < 0)
		die("Unable to open `%s': %m", name);
	if (read(fd, buf, MINIX_HEADER_LEN) != MINIX_HEADER_LEN)
		die("%s: Unable to read header", name);
	if (memcmp(buf, hdr, sizeof(hdr)) || lb[5])
		die("%s: Non-Minix header", name);
	if (lb[3])
		die("%s: Illegal data segment", name);
	if (lb[4])
		die("%s: Illegal bss segment", name);
	if (lb[7])
		die("%s: Illegal symbol table", name);
}

void usage(void)
{
	die("Usage: build bootsect system user [> image]");
}

int main(int argc, char ** argv)
{
	unsigned int i, sz, uz;
	u32 im_size, sys_size, usr_size;
	byte major_root, minor_root;
	struct stat sb;

	if (argc != 4)
		usage();
	else {
		major_root = DEFAULT_MAJOR_ROOT;
		minor_root = DEFAULT_MINOR_ROOT;
	}

	minix_open(argv[1]);				    /* Copy the boot sector */
	i = read(fd, buf, sizeof(buf));
	fprintf(stderr,"Boot sector %d bytes.\n",i);
	if (i != 512)
		die("Boot block must be exactly 512 bytes");
	if (buf[510] != 0x55 || buf[511] != 0xaa)
		die("Boot block hasn't got boot flag (0xAA55)");
	buf[508] = minor_root;
	buf[509] = major_root;
	if (write(1, buf, 512) != 512)
		die("Write call failed");
	close (fd);

	if ((fd = open(argv[2], O_RDONLY, 0)) < 0)	    /* Copy the system itself */
		die("Unable to open `%s': %m", argv[2]);
	if (fstat (fd, &sb))
		die("Unable to stat `%s': %m", argv[2]);
	sys_size = sz = sb.st_size;
	fprintf (stderr, "System is %d kB\n", sz/1024);
	while (sz > 0) {
		int l, n;

		l = (sz > sizeof(buf)) ? sizeof(buf) : sz;
		if ((n=read(fd, buf, l)) != l) {
			if (n < 0)
				die("Error reading %s: %m", argv[2]);
			else
				die("%s: Unexpected EOF", argv[2]);
		}
		if (write(1, buf, l) != l)
			die("Write failed");
		sz -= l;
	}
	close(fd);

	if ((fd = open(argv[3], O_RDONLY, 0)) < 0)    /* Copiamos el user */
		die("Unable to open `%s': %m", argv[3]);
	if (fstat (fd, &sb))
		die("Unable to stat `%s': %m", argv[3]);
	usr_size = uz = sb.st_size;
	fprintf (stderr, "User is %d kB\n", uz/1024);
	while (uz > 0) {
		int l, n;

		l = (uz > sizeof(buf)) ? sizeof(buf) : uz;
		if ((n=read(fd, buf, l)) != l) {
			if (n < 0)
				die("Error reading %s: %m", argv[3]);
			else
				die("%s: Unexpected EOF", argv[3]);
		}
		if (write(1, buf, l) != l)
			die("Write failed");
		uz -= l;
	}
	close(fd);

	im_size = (sys_size + usr_size + 15) / 16;

	fprintf (stderr, "Image is %d kB\n", (int)(sys_size + usr_size)/1024);
	buf[0] = (im_size & 0xff);
	buf[1] = ((im_size >> 8) & 0xff);
	if (lseek(1, 500, SEEK_SET) != 500)
		die("Output: seek failed");
	if (write(1, buf, 2) != 2)
		die("Write of image length failed");

	buf[0] = (sys_size & 0xff);
	buf[1] = ((sys_size >> 8) & 0xff);
	buf[2] = ((sys_size >> 16) & 0xff);
	buf[3] = 0;
	if (lseek(1, 512, SEEK_SET) != 512)
	  die("Output: seek failed");
	if (write(1, buf, 4) != 4)
	  die("Write of system length failed");

	buf[0] = (usr_size & 0xff);
	buf[1] = ((usr_size >> 8) & 0xff);
	buf[2] = ((usr_size >> 16) & 0xff);
	buf[3] = 0;
	if (lseek(1, 516, SEEK_SET) != 516)
	  die("Output: seek failed");
	if (write(1, buf, 4) != 4)
	  die("Write of user length failed");
	buf[0] = (0x7E5A & 0xff);
	buf[1] = ((0x7E5A >> 8) & 0xff);
	buf[2] = ((0x7E5A >> 16) & 0xff);
	buf[3] = 0;
	if (lseek(1, 520, SEEK_SET) != 520)
	  die("Output: seek failed");
	if (write(1, buf, 4) != 4)
	  die("Write of user length failed");

	return 0;					    /* Everything is OK */
}
