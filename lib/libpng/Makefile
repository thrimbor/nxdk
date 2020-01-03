#FIXME: Needs to copy $(LIBPNG_DIR)/scripts/pnglibconf.h.prebuilt to $(LIBPNG_DIR)/pnglibconf.h; modify:
#       //#define PNG_FLOATING_ARITHMETIC_SUPPORTED
#       //#define PNG_FLOATING_POINT_SUPPORTED

#LIBPNG_DIR = $(NXDK_DIR)/lib/libpng
LIBPNG_DIR = $(NXDK_DIR)/lib/libpng/libpng

LIBPNG_SRCS = $(LIBPNG_DIR)/png.c \
              $(LIBPNG_DIR)/pngerror.c \
              $(LIBPNG_DIR)/pngget.c \
              $(LIBPNG_DIR)/pngmem.c \
              $(LIBPNG_DIR)/pngpread.c \
              $(LIBPNG_DIR)/pngread.c \
              $(LIBPNG_DIR)/pngrio.c \
              $(LIBPNG_DIR)/pngrtran.c \
              $(LIBPNG_DIR)/pngrutil.c \
              $(LIBPNG_DIR)/pngset.c \
              $(LIBPNG_DIR)/pngtrans.c \
              $(LIBPNG_DIR)/pngwio.c \
              $(LIBPNG_DIR)/pngwrite.c \
              $(LIBPNG_DIR)/pngwtran.c \
              $(LIBPNG_DIR)/pngwutil.c

SRCS += $(LIBPNG_SRCS)
CFLAGS += -isystem $(LIBPNG_DIR) -I$(NXDK_DIR)/lib/libpng
