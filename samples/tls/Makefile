XBE_TITLE = nxdk\ sample\ -\ hello
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.c
NXDK_DIR ?= $(CURDIR)/../..
NXDK_NET = y

NXDK_CFLAGS=-I/home/venom/Sync/xbox_dev/mbedtls/include
NXDK_LDFLAGS=/home/venom/Sync/xbox_dev/mbedtls/build/library/libmbedtls.lib /home/venom/Sync/xbox_dev/mbedtls/build/library/libmbedx509.lib /home/venom/Sync/xbox_dev/mbedtls/build/library/libmbedcrypto.lib

include $(NXDK_DIR)/Makefile
