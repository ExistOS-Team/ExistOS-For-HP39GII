
.DEFAULT_GOAL := all

SCRIPT_DIR	= .\script

LDSCRIPT 		= .\script\ld.script

CC            = ..\tools\gcc-arm\bin\arm-none-eabi-gcc
GPP            = ..\tools\gcc-arm\bin\arm-none-eabi-g++
AS            = ..\tools\gcc-arm\bin\arm-none-eabi-as
CFLAGS        = -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -O1 -pipe -Wstringop-overflow=0

LINKER        = ..\tools\gcc-arm\bin\arm-none-eabi-gcc
LIBS          = -L..\tools\gcc-arm\lib\gcc\arm-none-eabi\9.3.1 -lgcc -lstdc++
#-lc -lstdc++ -lm
LFLAGS        = -T$(LDSCRIPT) 
#-nodefaultlibs -nostdlib 
DEL_FILE      = del /Q /S
DEL_DIR       = rmdir
ELF2ROM		  = ..\tools\sbtools\elftosb.exe
SBLOADER	  = ..\tools\sbtools\sb_loader.exe

GCCINCPATH       = -I. -I..\tools\gcc-arm\arm-none-eabi\include 
INCPATH  		= -Iinclude -Ihardware\include\registers -Ihardware\include -Ihal\include -Iinclude\freetype

CSRCS 	= $(wildcard  ./*.c ./hardware/*.c ./filesystem/fatfs/*.c)
CPPSRCS = $(wildcard ./*.cpp ./hal/*.cpp)

FREETPYE_CSRCS = $(wildcard ./freetype/src/base/ftbase.c ./freetype/src/gzip/ftgzip.c ./freetype/src/base/ftinit.c ./freetype/src/lzw/ftlzw.c ./freetype/src/base/ftsystem.c ./freetype/src/sfnt/sfnt.c ./freetype/src/smooth/smooth.c ./freetype/src/truetype/truetype.c ./freetype/src/base/ftbitmap.c ./freetype/*.c)

#所有的.o文件列表 原文件中所有以.c .cpp结尾的文件替换成以.o结尾
COBJS := $(CSRCS:.c=.o) 
CPPOBJS := $(CPPSRCS:.cpp=.o)
#FREETPYE_OBJS := $(FREETPYE_CSRCS:.c=.o)

#$(FREETPYE_OBJS) : %.o : %.c 
#	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 
	
$(COBJS) : %.o : %.c 
	$(CC) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS) 


$(CPPOBJS) : %.o : %.cpp 
	$(GPP) -c $< -o $@ $(GCCINCPATH) $(INCPATH) $(CFLAGS)

all: firmware.sb updater

rom.elf: $(COBJS) $(CPPOBJS) $(FREETPYE_OBJS)
	$(LINKER) -o rom.elf $(LFLAGS) $(COBJS) $(CPPOBJS) $(LIBS)

updater: rom.elf
	$(ELF2ROM) -z -c $(SCRIPT_DIR)\build_updater.bd -o updater.sb rom.elf

firmware.sb: rom.elf
	$(ELF2ROM) -z -c $(SCRIPT_DIR)\build_fw.bd -o firmware.sb rom.elf

flash:
	$(SBLOADER) -f updater.sb

clean:
	@$(DEL_FILE) *.o
	@$(DEL_FILE) *.a
	@$(DEL_FILE) *.tmp
	@$(DEL_FILE) *.elf
	@$(DEL_FILE) *.sb