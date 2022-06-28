
FROZEN_MANIFEST ?= manifest.py

# Include the core environment definitions; this will set $(TOP).
include ../../py/mkenv.mk

# Include py core make definitions.
include $(TOP)/py/py.mk


# Set makefile-level MicroPython feature configurations. 
MICROPY_ROM_TEXT_COMPRESSION ?= 0

# Define toolchain and other tools.
CROSS_COMPILE ?= arm-none-eabi-

# Set CFLAGS.
CFLAGS += -I. -I$(TOP) -I$(BUILD)
CFLAGS += -Wall -std=c99
#-Werror 
CFLAGS += -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -msoft-float -marm
CSUPEROPT = 
# save some code space for performance-critical code
DEBUG = 1
# Select debugging or optimisation build.
ifeq ($(DEBUG), 1)
CFLAGS += -Og
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

# Set linker flags.
LDFLAGS += -static -nostartfiles 
#-T sys_ld.script
LIBS = -lm -lc
#LIBS += lib/libm.a lib/libc.a
#LIBS += lib/libm.a lib/libc.a

# Define the required source files.
SRC_COMMON_C += entry.c src/start.c

SRC_COMMON_C += \
	shared/readline/readline.c \
	shared/runtime/pyexec.c	\
	shared/runtime/stdout_helpers.c \
	shared/runtime/sys_stdio_mphal.c 


#SRC_FROZEN_C += $(BUILD)/_frozen_mpy.c

# Define the required object files.
OBJ += $(PY_CORE_O)
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_COMMON_C:.c=.o))
#OBJ += $(addprefix $(BUILD)/, $(SRC_FROZEN_C:.c=.o))

# List of sources for qstr extraction
SRC_QSTR += $(SRC_COMMON_C) $(SRC_RUN_C) $(LIB_SRC_C)

ifneq ($(FROZEN_MANIFEST),)
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
endif

LIBSDIR = ../../../../
# Define the top-level target, the main firmware.
all: $(LIBSDIR)/libmpy.libc

#$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
#	$(ECHO) "MISC freezing bytecode"
#	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=mpz $< > $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(LIBSDIR)/libmpy.libc: $(BUILD)/libmpy.a
	cp $(BUILD)/libmpy.a ../../../../libmpy.libc

$(BUILD)/libmpy.a: $(OBJ)
	$(ECHO) "AR $@"
	$(Q)$(AR) -crv $@ $^
	
#	$(Q)$(SIZE) $@

$(BUILD)/ExistOS.exe: $(BUILD)/firmware.elf
	$(ECHO) "Create $@"
	$(Q)$(OBJCOPY) -O binary -I elf32-little -O binary -R .note -R .comment $^ $@

# Include remaining core make rules.
include $(TOP)/py/mkrules.mk
