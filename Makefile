#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>/devkitPPC")
endif

include $(DEVKITPPC)/gamecube_rules

LIBTP := Externals/libtp
ELF2REL := ../$(LIBTP)/bin/elf2rel.exe	# or: ../$(LIBTP)/bin/elf2rel
GCIPACK := python ../$(LIBTP)/bin/gcipack.py # has to start with ".." since when we call it, we're in the build.*/ dir


ifeq ($(VERSION),)
all: us jp eu
us:
	@$(MAKE) --no-print-directory VERSION=us
jp:
	@$(MAKE) --no-print-directory VERSION=jp
eu:
	@$(MAKE) --no-print-directory VERSION=eu

clean:
	@$(MAKE) --no-print-directory clean_target VERSION=us
	@$(MAKE) --no-print-directory clean_target VERSION=jp
	@$(MAKE) --no-print-directory clean_target VERSION=eu

.PHONY: all clean us jp eu
else

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	Rando.$(VERSION)
BUILD		:=	build.$(VERSION)
SOURCES		:=	source $(wildcard source/*) $(LIBTP)/source $(wildcard $(LIBTP)/source/*)
DATA		:=	data
INCLUDES	:=	include $(LIBTP)/include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

MACHDEP		= -mno-sdata -mgcn -DGEKKO -mcpu=750 -meabi -mhard-float

CFLAGS		= -nostdlib -ffreestanding -ffunction-sections -fdata-sections -g -Os -Wall -Werror $(MACHDEP) $(INCLUDE)
CXXFLAGS	= -fno-exceptions -fno-rtti -std=gnu++17 $(CFLAGS)

LDFLAGS		= -r -e _prolog -u _prolog -u _epilog -u _unresolved -Wl,--gc-sections -nostdlib -g $(MACHDEP) -Wl,-Map,$(notdir $@).map

# Platform options
ifeq ($(VERSION),us)
	CFLAGS += -DTP_US
	ASFLAGS += -DTP_US
	GAMECODE = "GZ2E"
	PRINTVER = "US"
else ifeq ($(VERSION),eu)
	CFLAGS += -DTP_EU
	ASFLAGS += -DTP_EU
	GAMECODE = "GZ2P"
	PRINTVER = "EU"
else ifeq ($(VERSION),jp)
	CFLAGS += -DTP_JP
	ASFLAGS += -DTP_JP
	GAMECODE = "GZ2J"
	PRINTVER = "JP"
endif


#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= 

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= 

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)

export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

# For REL linking
export LDFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ld)))
export MAPFILE		:= $(realpath $(LIBTP)/assets/$(VERSION).lst)
export BANNERFILE	:= $(realpath $(LIBTP)/assets/banner.raw)
export ICONFILE		:= $(realpath $(LIBTP)/assets/icon.raw)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
			-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean_target

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean_target:
	@echo clean ... $(VERSION)
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol $(OUTPUT).rel $(OUTPUT).gci

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).gci: $(OUTPUT).rel $(BANNERFILE) $(ICONFILE)
$(OUTPUT).rel: $(OUTPUT).elf $(MAPFILE)
$(OUTPUT).elf: $(LDFILES) $(OFILES)

$(OFILES_SOURCES) : $(HFILES)

# REL linking
%.rel: %.elf
	@echo output ... $(notdir $@)
	@$(ELF2REL) $< -s $(MAPFILE)

%.gci: %.rel
	@echo packing ... $(notdir $@)
	@$(GCIPACK) $< "Custom REL File" "Twilight Princess" "($(PRINTVER)) Rando" $(BANNERFILE) $(ICONFILE) $(GAMECODE)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o	%_jpg.h :	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
endif