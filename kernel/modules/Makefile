MODULES := $(shell find . -maxdepth 1 -mindepth 1 -type d -not -name template -printf '%P\n')

modules:
	@for mod in $(MODULES); do \
		echo "  > [Mod] $$mod"; \
		$(MAKE) --no-print-directory -C $$mod module install; \
	done
