# Include LWIP sources
LWIPDIR = $(NXDK_DIR)/lib/net/lwip/src
include $(LWIPDIR)/Filelists.mk
NETIFFILES := $(filter-out $(LWIPDIR)/netif/slipif.c,$(NETIFFILES))
LWIPSRCS := $(COREFILES) \
            $(CORE4FILES) \
            $(CORE6FILES) \
            $(APIFILES) \
            $(NETIFFILES)

# Include driver sources
DRIVERSRCS := $(wildcard $(NXDK_DIR)/lib/net/nforceif/src/*.c) \
              $(NXDK_DIR)/lib/nvnetdrv/nvnetdrv.c

NET_SRCS := \
	$(LWIPSRCS) \
	$(DRIVERSRCS)

NET_OBJS = $(addsuffix .obj, $(basename $(NET_SRCS)))

NXDK_CFLAGS += -I$(LWIPDIR)/include
NXDK_CFLAGS += -I$(NXDK_DIR)/lib/net/nforceif/include
NXDK_CXXFLAGS += -I$(LWIPDIR)/include
NXDK_CXXFLAGS += -I$(NXDK_DIR)/lib/net/nforceif/include

$(NXDK_DIR)/lib/libnxdk_net.lib: $(NET_OBJS)

main.exe: $(NXDK_DIR)/lib/libnxdk_net.lib

CLEANRULES += clean-net

.PHONY: clean-net
clean-net:
	$(VE)rm -f $(NET_OBJS) $(NXDK_DIR)/lib/libnxdk_net.lib
