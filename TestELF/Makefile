CC 		:= arm-none-eabi-gcc
LINKER 	:= arm-none-eabi-gcc

SRC ?= $(wildcard *.c)
OBJ ?= $(SRC:%.c=%.o)
INC ?= include

LIBS ?= -static

CFLAGS ?= -std=c99 -Wall -Os -I$(INC) \
	-mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -msoft-float \
	-Os -pipe \
	-fdata-sections -ffunction-sections -fcommon 

LDFLAGS :=  -Wl,--gc-sections

all: test.elf

clean:
	rm *.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
%.elf: $(OBJ) Makefile
	$(LINKER) -o test.elf $(LDFLAGS) $(OBJ) $(LIBS)
	
	