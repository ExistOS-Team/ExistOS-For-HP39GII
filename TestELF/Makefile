

CC 		:= arm-none-eabi-gcc
LINKER 	:= arm-none-eabi-gcc


SRC ?= $(wildcard *.c)
OBJ ?= $(SRC:%.c=%.o)
INC ?= include

LDSCRIPT ?= ld.script

LIBS ?= -lm

CFLAGS ?= -std=c99 -Wall -Os -I$(INC) \
	-mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -msoft-float \
	-Os -pipe \
	-fomit-frame-pointer -fdata-sections -ffunction-sections
	
LDFLAGS := -T$(LDSCRIPT)

all: test.elf

clean:
	rm *.o *.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
%.elf: $(OBJ)
	$(LINKER) -o test.elf $(LDFLAGS) $(OBJ) $(LIBS)
	
	