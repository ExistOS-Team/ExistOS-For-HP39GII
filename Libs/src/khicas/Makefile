CXX	= arm-none-eabi-g++
CC	= arm-none-eabi-gcc
AR	= arm-none-eabi-ar
RANLIB	= arm-none-eabi-gcc-ranlib
OBJCOPY = arm-none-eabi-objcopy

RM	= /bin/rm

#CFLAGS = -Os -mb -m4a-nofpu -mhitachi -flto -fno-strict-aliasing  -fno-exceptions -DHAVE_CONFIG_H -DTIMEOUT -DRELEASE -I. -I../include -I../include/fx
#CXXFLAGS = -Os -mb -m4a-nofpu -mhitachi -std=c++98 -fpermissive -flto -fno-use-cxa-atexit -fno-strict-aliasing -fno-rtti -fno-exceptions -DHAVE_CONFIG_H -DTIMEOUT -DRELEASE -I. -I../include -I../include/ustl -I../include/fx 
#-DTIMEOUT -flto 
CFLAGS = -O3 -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -fno-strict-aliasing -mthumb -fno-exceptions -DHP39 -DKHICAS -DHAVE_CONFIG_H -I../libtommath-0.39 -I. -ffunction-sections -fdata-sections -fomit-frame-pointer 
CXXFLAGS = -Os -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -mthumb -std=c++11  -fpermissive -fno-use-cxa-atexit -fno-strict-aliasing -fno-rtti -fno-exceptions -DHP39 -DKHICAS -DHAVE_CONFIG_H -I../libtommath-0.39 -I. -ffunction-sections -fdata-sections 
# -Werror=reorder

LDFLAGS = -static -nostdlib -Taddin.ld  -Wl,--gc-sections

STDLIBS =  -L. -L../lib  -ltommath -lustl -lm -lc -lfx -lgcc 

LIBS = -L. -L../lib -Wl,--start-group -lgui -lcas -ltommath -lustl -lm -lc -lfx -lgcc -Wl,--end-group

CAS_OBJS =sym2poly.o gausspol.o threaded.o maple.o ti89.o mathml.o moyal.o misc.o permu.o desolve.o input_parser.o symbolic.o index.o modpoly.o modfactor.o ezgcd.o derive.o solve.o intg.o intgab.o risch.o lin.o series.o subst.o vecteur.o sparse.o csturm.o tex.o global.o ifactor.o alg_ext.o gauss.o isom.o help.o plot.o plot3d.o rpn.o prog.o cocoa.o pari.o unary.o usual.o identificateur.o gen.o input_lexer.o tinymt32.o first.o quater.o k_cppsdk.o #softmath.o
# memmgr.o mem.o 

#  = dConsole.o fileGUI.o inputGUI.o menuGUI.o textGUI.o fileProvider.o graphicsProvider.o stringsProvider.o history.o kdisplay.o

GUI_OBJS = kdisplay.o kadd.o file.o

#.PRECIOUS: libcas.a libgui.a
all: libcas.libcpp libkcasgui.libcpp
#all: khicas.g1a khicasen.g1a # pour la version console, supprimer le lien iostream -> iostream.new
	/bin/cp libcas.libcpp libkcasgui.libcpp ../../

%.o: %.s
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

libgui.a: $(GUI_OBJS)
	$(RM) -f $@
	$(AR) cru $@ $^
	$(RANLIB) $@
	$(OBJCOPY) --redefine-sym mp_init=mp_tm_init $@

libcas.a: $(CAS_OBJS)
	$(RM) -f $@
	$(AR) cru $@ $^
	$(RANLIB) $@
	$(OBJCOPY) --redefine-sym mp_init=mp_tm_init $@

libcas.libcpp: libcas.a
	mv libcas.a libcas.libcpp

libkcasgui.libcpp: libgui.a
	mv libgui.a libkcasgui.libcpp 

khicas.elf: libcas.a libgui.a crt0.o syscalls.o menufr.o #catalogfr.o main.o
#	$(CXX) $(LDFLAGS) crt0.o syscalls.o $(CAS_OBJS) $(GUI_OBJS) $(STDLIBS) -o $@
#	$(CXX)  -m3 -mhitachi  -mb -Os -nostdlib $(LDFLAGS) crt0.o syscalls.o  $(CAS_OBJS) $(GUI_OBJS) $(STDLIBS) -o $@
	$(CXX) $(LDFLAGS) crt0.o syscalls.o menufr.o $(LIBS) -o $@

khicas.g1a: khicas.elf
	sh3eb-elf-objcopy -R .comment -R .bss -O binary khicas.elf khicas.bin
	g1a-wrapper  khicas.bin -o khicas.g1a # -i khicas.bmp -v 1.5.1
	sh3eb-elf-nm khicas.elf > testelf
	sh3eb-elf-objdump -C -t khicas.elf | sort > dump

khicasen.elf: libcas.a libgui.a crt0.o syscalls.o menuen.o #catalogfr.o main.o
#	$(CXX) $(LDFLAGS) crt0.o syscalls.o $(CAS_OBJS) $(GUI_OBJS) $(STDLIBS) -o $@
#	$(CXX)  -m3 -mhitachi  -mb -Os -nostdlib $(LDFLAGS) crt0.o syscalls.o  $(CAS_OBJS) $(GUI_OBJS) $(STDLIBS) -o $@
	$(CXX) $(LDFLAGS) crt0.o syscalls.o menuen.o $(LIBS) -o $@

khicasen.g1a: khicasen.elf
	sh3eb-elf-objcopy -R .comment -R .bss -O binary khicasen.elf khicasen.bin
	g1a-wrapper khicasen.bin -o khicasen.g1a # -i khicasen.bmp
	sh3eb-elf-nm khicasen.elf > testelfen
	sh3eb-elf-objdump -C -t khicasen.elf | sort > dumpen

clean:
	$(RM) -f *.o libcas.a libgui.a 
