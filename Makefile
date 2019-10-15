#----------------------------------------------------------------------------------------
# Goals for Build
#----------------------------------------------------------------------------------------

.PHONY: all

all: buildAA makecia package

buildAA:
	@$(MAKE) -C main
	@$(MAKE) -C menu
	@mv "menu/menu.nds" "./menu.bin"
	@mv "main/Relaunch.nds" "./Relaunch.nds"

makecia:
	@./make_cia --srl="Relaunch.nds"

package:
	@mkdir Relaunch/
    	@mkdir Relaunch/_nds/
    	@mkdir Relaunch/_nds/Relaunch/
    	@mv './menu.bin' 'Relaunch/_nds/Relaunch/menu.bin'
    	@mv './Relaunch.cia' 'Relaunch/Relaunch.cia'
    	@mv './Relaunch.nds' 'Relaunch/Relaunch.nds'

clean:
	@echo clean build directories
	@$(MAKE) -C main clean
	@$(MAKE) -C menu clean

	@echo clean package files
