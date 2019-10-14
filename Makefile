#----------------------------------------------------------------------------------------
# Goals for Build
#----------------------------------------------------------------------------------------

.PHONY: all

all: buildAA makecia

buildAA:
	@$(MAKE) -C main
	@$(MAKE) -C menu
	@mv "menu/menu.nds" "./menu.bin"
	@mv "main/Relaunch.nds" "./Relaunch.nds"

makecia:
	@./make_cia --srl="Relaunch.nds"

clean:
	@echo clean build directories
	@$(MAKE) -C main clean
	@$(MAKE) -C menu clean

	@echo clean package files
