# Makefile wrapper - delega para o Makefile em src/
.PHONY: all clean debug

all:
	@$(MAKE) -C src

clean:
	@$(MAKE) -C src clean

debug:
	@$(MAKE) -C src debug

%:
	@$(MAKE) -C src $@
