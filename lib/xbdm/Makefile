$(NXDK_DIR)/lib/libxbdm.lib: $(NXDK_DIR)/lib/xbdm/xbdm.dll.def
	llvm-dlltool -d "$<" -m i386 -l "$@"
