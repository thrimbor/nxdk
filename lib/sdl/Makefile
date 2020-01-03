SDL_TTF_DIR = $(NXDK_DIR)/lib/sdl/SDL_ttf
SDL_TTF_SRCS += $(SDL_TTF_DIR)/SDL_ttf.c
SDL_TTF_OBJS = $(addsuffix .obj, $(basename $(SDL_TTF_SRCS)))

$(NXDK_DIR)/lib/libSDL_ttf.lib: $(SDL_TTF_OBJS)

FREETYPE_DIR = $(NXDK_DIR)/lib/sdl/SDL_ttf/external/freetype-2.4.12
FREETYPE_SRCS += $(FREETYPE_DIR)/src/base/ftbase.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/base/ftglyph.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/base/ftbitmap.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/base/ftstroke.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/base/ftinit.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/base/ftsystem.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/truetype/truetype.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/raster/raster.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/smooth/smooth.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/autofit/autofit.c
FREETYPE_SRCS += $(FREETYPE_DIR)/src/sfnt/sfnt.c
FREETYPE_OBJS = $(addsuffix .obj, $(basename $(FREETYPE_SRCS)))

$(NXDK_DIR)/lib/libfreetype.lib: $(FREETYPE_OBJS)

CFLAGS += -I$(NXDK_DIR)/lib/sdl \
          -I$(SDL_TTF_DIR) \
          -I$(FREETYPE_DIR)/include \
          -DXBOX

CXXFLAGS += -I$(NXDK_DIR)/lib/sdl \
          -I$(SDL_TTF_DIR) \
          -I$(FREETYPE_DIR)/include \
          -DXBOX

main.exe: $(NXDK_DIR)/lib/libSDL_ttf.lib $(NXDK_DIR)/lib/libfreetype.lib

CLEANRULES += clean-sdl_ttf

.PHONY: clean-sdl_ttf
clean-sdl_ttf:
	$(VE)rm -f $(SDL_TTF_OBJS) \
	           $(FREETYPE_OBJS) \
	           $(NXDK_DIR)/lib/libSDL_ttf.lib \
	           $(NXDK_DIR)/lib/libfreetype.lib

LIBSDLIMG_DIR = $(NXDK_DIR)/lib/sdl/SDL_image
LIBSDLIMG_SRCS = $(LIBSDLIMG_DIR)/IMG.c \
                 $(LIBSDLIMG_DIR)/IMG_png.c \
                 $(LIBSDLIMG_DIR)/IMG_bmp.c \
                 $(LIBSDLIMG_DIR)/IMG_gif.c \
                 $(LIBSDLIMG_DIR)/IMG_jpg.c \
                 $(LIBSDLIMG_DIR)/IMG_lbm.c \
                 $(LIBSDLIMG_DIR)/IMG_pcx.c \
                 $(LIBSDLIMG_DIR)/IMG_pnm.c \
                 $(LIBSDLIMG_DIR)/IMG_svg.c \
                 $(LIBSDLIMG_DIR)/IMG_tga.c \
                 $(LIBSDLIMG_DIR)/IMG_tif.c \
                 $(LIBSDLIMG_DIR)/IMG_webp.c \
                 $(LIBSDLIMG_DIR)/IMG_WIC.c \
                 $(LIBSDLIMG_DIR)/IMG_xcf.c \
                 $(LIBSDLIMG_DIR)/IMG_xpm.c \
                 $(LIBSDLIMG_DIR)/IMG_xv.c \
                 $(LIBSDLIMG_DIR)/IMG_xxx.c

SRCS += $(LIBSDLIMG_SRCS)
CFLAGS += -isystem $(LIBSDLIMG_DIR) -DLOAD_BMP -DLOAD_GIF -DLOAD_JPG -DLOAD_LBM -DLOAD_PCX -DLOAD_PNG -DLOAD_PNM -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM -DLOAD_XV -DLOAD_XXX
