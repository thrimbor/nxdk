ifeq ($(NXDK_DIR),)
NXDK_DIR = $(shell pwd)
endif

ifeq ($(XBE_TITLE),)
XBE_TITLE = nxdk_app
endif

ifeq ($(OUTPUT_DIR),)
OUTPUT_DIR = bin
endif

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

LD  = nxdk-link
LIB = nxdk-lib
AS  = nxdk-as
CC  = nxdk-cc
CXX = nxdk-cxx

ifeq ($(UNAME_S),Linux)
ifneq ($(UNAME_M),x86_64)
CGC          = $(NXDK_DIR)/tools/cg/linux/cgc.i386
else
CGC          = $(NXDK_DIR)/tools/cg/linux/cgc
endif #UNAME_M != x86_64
endif
ifeq ($(UNAME_S),Darwin)
CGC          = $(NXDK_DIR)/tools/cg/mac/cgc
endif
ifneq (,$(findstring MSYS_NT,$(UNAME_S)))
$(error Please use a MinGW64 shell)
endif

ifneq (,$(findstring MINGW,$(UNAME_S)))
CGC          = $(NXDK_DIR)/tools/cg/win/cgc
endif

TARGET       = $(OUTPUT_DIR)/default.xbe
CXBE         = $(NXDK_DIR)/tools/cxbe/cxbe
VP20COMPILER = $(NXDK_DIR)/tools/vp20compiler/vp20compiler
FP20COMPILER = $(NXDK_DIR)/tools/fp20compiler/fp20compiler
EXTRACT_XISO = $(NXDK_DIR)/tools/extract-xiso/build/extract-xiso
TOOLS        = cxbe vp20compiler fp20compiler extract-xiso

# Debug flags used to build the debug variants of the toolchain libraries (always available)
NXDK_DBG_ASFLAGS := $(NXDK_ASFLAGS) -g -gdwarf-4
NXDK_DBG_CFLAGS  := $(NXDK_CFLAGS) -g -gdwarf-4
NXDK_DBG_CXXFLAGS:= $(NXDK_CXXFLAGS) -g -gdwarf-4

# Keep the existing behavior that sets linker debug option when user requested DEBUG
ifeq ($(DEBUG),y)
NXDK_LDFLAGS += -debug
endif

ifeq ($(LTO),y)
NXDK_ASFLAGS += -flto
NXDK_CFLAGS += -flto
NXDK_CXXFLAGS += -flto
endif

# library suffix for linking user programs: use debug variants when DEBUG=y
ifeq ($(DEBUG),y)
NXDK_LIB_SUFFIX = d
else
NXDK_LIB_SUFFIX =
endif

# list of core toolchain libraries to pass explicitly to the linker (use suffix)
NXDK_LINK_LIBS := \
  $(NXDK_DIR)/lib/libwinapi$(NXDK_LIB_SUFFIX).lib \
  $(NXDK_DIR)/lib/xboxkrnl/libxboxkrnl$(NXDK_LIB_SUFFIX).lib \
  $(NXDK_DIR)/lib/libxboxrt$(NXDK_LIB_SUFFIX).lib \
  $(NXDK_DIR)/lib/libpdclib$(NXDK_LIB_SUFFIX).lib \
  $(NXDK_DIR)/lib/libnxdk_hal$(NXDK_LIB_SUFFIX).lib \
  $(NXDK_DIR)/lib/libnxdk$(NXDK_LIB_SUFFIX).lib \
  $(NXDK_DIR)/lib/nxdk_usb$(NXDK_LIB_SUFFIX).lib

ifeq ($(NXDK_DIR),)
NXDK_DIR = $(shell pwd)
endif

ifeq ($(XBE_TITLE),)
XBE_TITLE = nxdk_app
endif

all: $(TARGET)

include $(NXDK_DIR)/lib/Makefile
OBJS = $(addsuffix .obj, $(basename $(SRCS)))

ifneq ($(NXDK_CXX),)
include $(NXDK_DIR)/lib/libcxx/Makefile.nxdk
endif

include $(NXDK_DIR)/lib/net/Makefile

ifneq ($(NXDK_SDL),)
include $(NXDK_DIR)/lib/sdl/SDL2/Makefile.xbox
include $(NXDK_DIR)/lib/sdl/Makefile
endif

V = 0
VE_0 := @
VE_1 :=
VE = $(VE_$(V))

ifeq ($(V),1)
QUIET=
else
QUIET=>/dev/null
endif

DEPS := $(filter %.c.d, $(SRCS:.c=.c.d))
DEPS += $(filter %.cpp.d, $(SRCS:.cpp=.cpp.d))

$(OUTPUT_DIR)/default.xbe: main.exe $(OUTPUT_DIR) $(CXBE)
	@echo "[ CXBE     ] $@"
	$(VE)$(CXBE) -OUT:$@ -TITLE:$(XBE_TITLE) $< $(QUIET)

$(OUTPUT_DIR):
	@mkdir -p $(OUTPUT_DIR);

ifneq ($(GEN_XISO),)
$(GEN_XISO): $(OUTPUT_DIR)/default.xbe $(EXTRACT_XISO)
	@echo "[ XISO     ] $@"
	$(VE) $(EXTRACT_XISO) -c $(OUTPUT_DIR) $(XISO_FLAGS) "$@" $(QUIET)
endif

$(SRCS): $(SHADER_OBJS)

ifneq ($(NXDK_ONLY),)
.PHONY: main.exe
main.exe: $(OBJS)
else
main.exe: $(OBJS) $(NXDK_DIR)/lib/xboxkrnl/libxboxkrnl.lib
	@echo "[ LD       ] $@"
	$(VE) $(LD) $(NXDK_LDFLAGS) $(LDFLAGS) -out:'$@' $^ $(NXDK_LINK_LIBS)
endif

%.lib:
	@echo "[ LIB      ] $@"
	$(VE) $(LIB) -out:'$@' $^

%.obj: %.cpp
	@echo "[ CXX      ] $@"
	$(VE) $(CXX) $(NXDK_CXXFLAGS) $(CXXFLAGS) -MD -MP -MT '$@' -MF '$(patsubst %.obj,%.cpp.d,$@)' -c -o '$@' '$<'

%.obj: %.c
	@echo "[ CC       ] $@"
	$(VE) $(CC) $(NXDK_CFLAGS) $(CFLAGS) -MD -MP -MT '$@' -MF '$(patsubst %.obj,%.c.d,$@)' -c -o '$@' '$<'

%.obj: %.s
	@echo "[ AS       ] $@"
	$(VE) $(AS) $(NXDK_ASFLAGS) $(ASFLAGS) -c -o '$@' '$<'

# Build debug objects that won't collide with normal objects (source.c -> source.d.obj)
%.d.obj: %.cpp
	@echo "[ CXX DBG  ] $@"
	$(VE) $(CXX) $(NXDK_DBG_CXXFLAGS) $(CXXFLAGS) -MD -MP -MT '$@' -MF '$(patsubst %.d.obj,%.cpp.d,$@)' -c -o '$@' '$<'

%.d.obj: %.c
	@echo "[ CC DBG   ] $@"
	$(VE) $(CC) $(NXDK_DBG_CFLAGS) $(CFLAGS) -MD -MP -MT '$@' -MF '$(patsubst %.d.obj,%.c.d,$@)' -c -o '$@' '$<'

%.d.obj: %.s
	@echo "[ AS DBG   ] $@"
	$(VE) $(AS) $(NXDK_DBG_ASFLAGS) $(ASFLAGS) -c -o '$@' '$<'

ifneq ($(GEN_XISO),)
DEPS += $(filter %.c.d, $(SRCS:.c=.c.d))
endif

