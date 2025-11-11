# Makefile for MAX Extension Tests

CC = riscv64-unknown-elf-gcc
CFLAGS = -march=rv64gcv -mabi=lp64d -static -O2
OBJDUMP = riscv64-unknown-elf-objdump

TARGETS = test_vmatmul

all: $(TARGETS)

test_vmatmul: test_vmatmul.c
	$(CC) $(CFLAGS) -o $@ $<
	$(OBJDUMP) -d $@ > $@.dump

clean:
	rm -f $(TARGETS) *.dump

.PHONY: all clean
