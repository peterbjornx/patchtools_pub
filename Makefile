SRCS_C =\
	patchfile.c \
	crypto.c \
	fprom.c \
	cpukeys.c \
	dump_patch.c \
	file_io.c \
	filefmt.c
CFLAGS +=-g

patchtools: $(SRCS_C) opt_cipher.o

opt_cipher.o: opt_cipher.s
	nasm -felf64 opt_cipher.s

clean:
	rm *.o patchtools
