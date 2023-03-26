# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

ifeq ($(strip $(BLOCKSDS)),)
    $(error "Environment variable BLOCKSDS not found")
endif

# User config
# ===========

NAME			:= WordleDS

GAME_TITLE		:= Wordle DS
GAME_SUBTITLE	:=
GAME_AUTHOR		:= Pk11
GAME_ICON		:= banner.bin

# DLDI and internal SD slot of DSi
# --------------------------------

# Root folder of the SD image
SDROOT		:=
# Name of the generated image it "DSi-1.sd" for no$gba in DSi mode
SDIMAGE		:=

# Source code paths
# -----------------

SOURCEDIRS	:= source
INCLUDEDIRS	:= include include/libqrencode build/version
GFXDIRS		:= gfx
BINDIRS		:= data
AUDIODIRS	:= audio
NITROFATDIR	:=

# Defines passed to all files
# ---------------------------

DEFINES		:=

# Get version number from git
# ---------------------------

ifneq ($(shell echo $(shell git tag -l --points-at HEAD) | head -c 1),) # If on a tagged commit, use just tag
GIT_VER := $(shell git tag -l --points-at HEAD)
else # Otherwise include commit
GIT_VER := $(shell git describe --abbrev=0 --tags)-$(shell git rev-parse --short=7 HEAD)
endif

# Print new version if changed
ifeq (,$(findstring $(GIT_VER),$(shell cat build/version/version.hpp)))
$(shell mkdir -p build/version && printf "#ifndef VERSION_HPP\n#define VERSION_HPP\n\n#define VER_NUMBER \"$(GIT_VER)\"\n\n#endif\n" > build/version/version.hpp)
endif

# Libraries
# ---------

LIBS		:= -lmm9 -ldswifi9 -lnds9 -lstdc++ -lc
LIBDIRS		:= $(BLOCKSDS)/libs/maxmod \
			   $(BLOCKSDS)/libs/dswifi \
			   $(BLOCKSDS)/libs/libnds \
			   $(BLOCKSDS)/libs/libstdc++9 \
			   $(BLOCKSDS)/libs/libc9

# Build artifacts
# ---------------

BUILDDIR		:= build
ELF				:= build/$(NAME).elf
DUMP			:= build/$(NAME).dump
NITROFAT_IMG	:= build/nitrofat.bin
MAP				:= build/$(NAME).map
SOUNDBANKDIR	:= $(BUILDDIR)/maxmod
ROM				:= $(NAME).nds

# Tools
# -----

PREFIX		:= arm-none-eabi-
CC			:= $(PREFIX)gcc
CXX			:= $(PREFIX)g++
OBJDUMP		:= $(PREFIX)objdump
MKDIR		:= mkdir
RM			:= rm -rf

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Source files
# ------------

ifneq ($(GFXDIRS),)
    SOURCES_PNG	:= $(shell find -L $(GFXDIRS) -name "*.png")
    INCLUDEDIRS	+= $(addprefix $(BUILDDIR)/,$(GFXDIRS))
endif
ifneq ($(AUDIODIRS),)
    SOURCES_AUDIO	:= $(shell find -L $(AUDIODIRS) -regex '.*\.\(it\|mod\|s3m\|wav\|xm\)')
    ifneq ($(SOURCES_AUDIO),)
        INCLUDEDIRS	+= $(SOUNDBANKDIR)
    endif
endif
ifneq ($(BINDIRS),)
    SOURCES_BIN	:= $(shell find -L $(BINDIRS) -name "*.bin") $(SOURCES_PNG:.png=.grf)
    INCLUDEDIRS	+= $(addprefix $(BUILDDIR)/,$(BINDIRS))
endif

SOURCES_S	:= $(shell find -L $(SOURCEDIRS) -name "*.s")
SOURCES_C	:= $(shell find -L $(SOURCEDIRS) -name "*.c")
SOURCES_CPP	:= $(shell find -L $(SOURCEDIRS) -name "*.cpp")

# Compiler and linker flags
# -------------------------

DEFINES		+= -D__NDS__ -DARM9

ARCH		:= -march=armv5te -mtune=arm946e-s

WARNFLAGS	:= -Wall

ifeq ($(SOURCES_CPP),)
    LD	:= $(CC)
else
    LD	:= $(CXX)
endif

INCLUDEFLAGS	:= $(foreach path,$(INCLUDEDIRS),-I$(path)) \
				   $(foreach path,$(LIBDIRS),-I$(path)/include)

LIBDIRSFLAGS	:= $(foreach path,$(LIBDIRS),-L$(path)/lib)

ASFLAGS		+= -x assembler-with-cpp $(DEFINES) $(ARCH) \
			   -mthumb -mthumb-interwork $(INCLUDEFLAGS) \
			   -ffunction-sections -fdata-sections

CFLAGS		+= -std=gnu11 $(WARNFLAGS) $(DEFINES) $(ARCH) \
			   -mthumb -mthumb-interwork $(INCLUDEFLAGS) -O2 \
			   -ffunction-sections -fdata-sections \
			   -fomit-frame-pointer

CXXFLAGS	+= -std=gnu++14 $(WARNFLAGS) $(DEFINES) $(ARCH) \
			   -mthumb -mthumb-interwork $(INCLUDEFLAGS) -O2 \
			   -ffunction-sections -fdata-sections \
			   -fno-exceptions -fno-rtti \
			   -fomit-frame-pointer

LDFLAGS		:= -mthumb -mthumb-interwork $(LIBDIRSFLAGS) \
			   -Wl,-Map,$(MAP) -Wl,--gc-sections -nostdlib \
			   -T$(BLOCKSDS)/sys/crts/ds_arm9.mem \
			   -T$(BLOCKSDS)/sys/crts/ds_arm9.ld \
			   -Wl,--start-group $(LIBS) -lgcc -Wl,--end-group

# Intermediate build files
# ------------------------

OBJS_ASSETS	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN)))

HEADERS_ASSETS	:= $(patsubst %.bin,%_bin.h,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN)))

ifneq ($(SOURCES_AUDIO),)
    OBJS_ASSETS		+= $(SOUNDBANKDIR)/soundbank.c.o
    HEADERS_ASSETS	+= $(SOUNDBANKDIR)/soundbank.h
endif

OBJS_SOURCES	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_S))) \
				   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_C))) \
				   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_CPP)))

OBJS		:= $(OBJS_ASSETS) $(OBJS_SOURCES)

DEPS		:= $(OBJS:.o=.d)

# Targets
# -------

.PHONY: all clean dump dldipatch sdimage

all: $(ROM)

ifneq ($(strip $(NITROFATDIR)),)
# Additional arguments for ndstool
NDSTOOL_FAT	:= -F $(NITROFAT_IMG)

$(NITROFAT_IMG): $(NITROFATDIR)
	@echo "  IMGBUILD $@ $(NITROFATDIR)"
	$(V)sh $(BLOCKSDS)/tools/imgbuild/imgbuild.sh $@ $(NITROFATDIR)

# Make the NDS ROM depend on the filesystem image only if it is needed
$(ROM): $(NITROFAT_IMG)
endif

$(ROM): $(ELF)
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 $(BLOCKSDS)/sys/default_arm7/arm7.elf -9 $(ELF) \
		-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
		$(NDSTOOL_FAT)

$(ELF): $(OBJS)
	@echo "  LD      $@"
	$(V)$(LD) -o $@ $(OBJS) $(BLOCKSDS)/sys/crts/ds_arm9_crt0.o $(LDFLAGS)

$(DUMP): $(ELF)
	@echo "  OBJDUMP   $@"
	$(V)$(OBJDUMP) -h -C -S $< > $@

dump: $(DUMP)

clean:
	@echo "  CLEAN"
	$(V)$(RM) $(ROM) $(DUMP) $(BUILDDIR) $(SDIMAGE)

sdimage:
	@echo "  IMGBUILD $(SDIMAGE) $(SDROOT)"
	$(V)sh $(BLOCKSDS)/tools/imgbuild/imgbuild.sh $(SDIMAGE) $(SDROOT)

dldipatch: $(ROM)
	@echo "  DLDITOOL $(ROM)"
	$(V)$(BLOCKSDS)/tools/dlditool/dlditool \
		$(BLOCKSDS)/tools/dldi/r4tfv2.dldi $(ROM)

# Rules
# -----

$(BUILDDIR)/%.s.o : %.s
	@echo "  AS      $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(ASFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.c.o : %.c
	@echo "  CC      $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.cpp.o : %.cpp
	@echo "  CXX     $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.bin.o $(BUILDDIR)/%_bin.h : %.bin
	@echo "  BIN2C   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(BLOCKSDS)/tools/bin2c/bin2c $< $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(BUILDDIR)/$*.bin.o $(BUILDDIR)/$*_bin.c

$(BUILDDIR)/%.grf.o $(BUILDDIR)/%_grf.h : $(BUILDDIR)/%.grf
	@echo "  BIN2C   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(BLOCKSDS)/tools/bin2c/bin2c $< $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(BUILDDIR)/$*.grf.o $(BUILDDIR)/$*_grf.c

$(BUILDDIR)/%.grf : %.png %.grit
	@echo "  GRIT    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(BLOCKSDS)/tools/grit/grit $< -ftr -fh! -W1 -o$(BUILDDIR)/$*

$(SOUNDBANKDIR)/soundbank.c.o $(SOUNDBANKDIR)/soundbank.h : $(SOURCES_AUDIO)
	@echo "  MMUTIL $^"
	@$(MKDIR) -p $(@D)
	@$(BLOCKSDS)/tools/mmutil/mmutil $^ -d \
		-o$(SOUNDBANKDIR)/soundbank.bin -h$(SOUNDBANKDIR)/soundbank.h
	$(V)$(BLOCKSDS)/tools/bin2c/bin2c $(SOUNDBANKDIR)/soundbank.bin \
		$(SOUNDBANKDIR)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(SOUNDBANKDIR)/soundbank.c.o \
		$(SOUNDBANKDIR)/soundbank_bin.c

# All assets must be built before the source code
# -----------------------------------------------

$(SOURCES_S) $(SOURCES_C) $(SOURCES_CPP): $(HEADERS_ASSETS)

# Include dependency files if they exist
# --------------------------------------

-include $(DEPS)
