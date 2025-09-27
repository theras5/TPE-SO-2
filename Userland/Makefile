include Makefile.inc

MODULE    := ./0000-sampleCodeModule.bin
C_SRCS    := $(wildcard [^_]*.c) $(wildcard pongis/*.c)
ASM_SRCS  := $(wildcard asm/*.asm)
OBJS      := $(C_SRCS:%.c=%.o) $(ASM_SRCS:asm/%.asm=asm/%.o)

CFLAGS    := $(GCCFLAGS) -std=gnu11 -ffreestanding -O2 -msse -Iinclude -Ipongis/include
ASFLAGS   := $(ASMFLAGS) -felf64

all: $(MODULE)

$(MODULE): $(OBJS)
	$(GCC) $(CFLAGS) -nostdlib -T userCodeModule.ld _loader.c -o $(MODULE) $(OBJS)

%.o: %.c
	$(GCC) $(CFLAGS) -c $< -o $@

asm/%.o: asm/%.asm
	$(ASM) $(ASFLAGS) $< -o $@

# clean:
# 	rm -f $(OBJS) $(MODULE)

clean:
	rm -rf asm/*.o *.o *.bin pongis/*.o
