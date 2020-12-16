
.DEFAULT_GOAL := all

SCRIPT_DIR	= .\script

LDSCRIPT 		= .\script\ld.script

CC             = ..\tools\gcc-arm\bin\arm-none-eabi-gcc
GPP            = ..\tools\gcc-arm\bin\arm-none-eabi-g++
AS             = ..\tools\gcc-arm\bin\arm-none-eabi-as
CFLAGS         = -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -Os -pipe -ffunction-sections
CFLAGS_TESTING = -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -O3 -pipe -Wstringop-overflow=0

LINKER        = ..\tools\gcc-arm\bin\arm-none-eabi-gcc
LIBS          = 
#-L..\tools\gcc-arm\lib\gcc\arm-none-eabi\9.3.1 
#-lc -lstdc++ -lm -lgcc
LFLAGS        = -T$(LDSCRIPT) -Wl,--gc-sections
#-nodefaultlibs -nostdlib 
DEL_FILE      = del /Q /S
DEL_DIR       = rmdir
ELF2ROM		  = ..\tools\sbtools\elftosb.exe
SBLOADER	  = ..\tools\sbtools\sb_loader.exe

GCCINCPATH       = -I. -I..\tools\gcc-arm\arm-none-eabi\include 
INCPATH  		=  -Ikernel\porting -Ikernel\include -Ilibrary\stmp3770\inc -Ilibrary\stmp3770\inc\registers -ITestApp -IServices\inc -ISystemApp -IConfig -ILibrary\TinyUSB -ILibrary\TinyUSB\device -ILibrary\FreeRTOS-Plus-CLI -ILibrary\dhara -ILibrary\FatFs


CSRCS 	= $(wildcard  ./*.c ./kernel/*.c   ./kernel/porting/*.c ./library/stmp3770/src/*.c  ./TestApp/*.c ./Services/src/*.c ./SystemApp/*.c ./Config/*.c ./Library/FreeRTOS-Plus-CLI/*.c)


CTINYUSBCSRCS	=	$(wildcard ./Library/TinyUSB/*.c ./Library/TinyUSB/common/*.c ./Library/TinyUSB/device/*.c ./Library/TinyUSB/class/cdc/*.c ./Library/TinyUSB/class/msc/*.c )

CDHARASRCS = $(wildcard ./Library/dhara/*.c)

FatFsSrcs = $(wildcard ./Library/FatFs/*.c)



SSRCS	= $(wildcard ./*.s)

CSRCS_TESTING = $(wildcard ./benchmark/*.c)
CPPSRCS = $(wildcard ./*.cpp ./hal/*.cpp)
#LVGL_DIR = ./hal
#LVGL_DIR_NAME = lvgl
#include ./hal/lvgl/lvgl.mk

#所有的.o文件列表 原文件中所有以.c .cpp结尾的文件替换成以.o结尾
COBJS := $(CSRCS:.c=.o) 
CTINYUSBCOBJS := $(CTINYUSBCSRCS:.c=.o) 
CDHARAOBJS := $(CDHARASRCS:.c=.o) 
FatFsObjs := $(FatFsSrcs:.c=.o) 

SOBJS := $(SSRCS:.s=.o) 
CASMS := $(CSRCS:.c=.s) 
COBJS_TESTING := $(CSRCS_TESTING:.c=.o)
CPPOBJS := $(CPPSRCS:.cpp=.o)


#给coremark传递编译器设置
CFLAGS_TMP := $(CFLAGS_TESTING)
CFLAGS_TESTING += -D COMPILER_FLAGS="\"$(CFLAGS_TMP)\""

#$(FREETPYE_OBJS) : %.o : %.c 
#	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 
$(SOBJS) :  %.o : %.s 
	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 
	
$(COBJS) : %.o : %.c 
	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 
	
$(CTINYUSBCOBJS) : %.o : %.c 
	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 

$(CDHARAOBJS) : %.o : %.c 
	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 
	
$(FatFsObjs) : %.o : %.c 
	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 

$(COBJS_TESTING) : %.o : %.c 
	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS_TESTING) 

$(CPPOBJS) : %.o : %.cpp 
	$(GPP) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS)

all: firmware.sb updater

debug:
	$(CC) -S $(CSRCS) $(GCCINCPATH) $(INCPATH)
# $(COBJS_TESTING)
rom.elf: $(COBJS) $(CPPOBJS) $(SOBJS)  $(CTINYUSBCOBJS) $(CDHARAOBJS) $(FatFsObjs)
	$(LINKER) -o rom.elf $(LFLAGS)  $(SOBJS) $(COBJS) $(CTINYUSBCOBJS) $(CDHARAOBJS) $(FatFsObjs) $(CPPOBJS) $(LIBS)

updater: rom.elf
	$(ELF2ROM) -z -c $(SCRIPT_DIR)\build_updater.bd -o updater.sb rom.elf

firmware.sb: rom.elf
	$(ELF2ROM) -z -c $(SCRIPT_DIR)\build_fw.bd -o firmware.sb rom.elf

flash:
	$(SBLOADER) -f updater.sb

clean:
	@$(DEL_FILE) *.o
	@$(DEL_FILE) *.a
	@$(DEL_FILE) *.s
	@$(DEL_FILE) *.tmp
	@$(DEL_FILE) *.elf
	@$(DEL_FILE) *.sb