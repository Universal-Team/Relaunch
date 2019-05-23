#----------------------------------------------------------------------------------------
# Goals for Build
#----------------------------------------------------------------------------------------

.PHONY: all

all: buildDSi3DS buildFlashcart makecia

buildDSi3DS:
	@mv "main/Makefile_DSi-3DS" "main/Makefile"
	@$(MAKE) -C main
	@mv "menu/Makefile_DSi-3DS" "menu/Makefile"
	@$(MAKE) -C menu
	@mv "main/Makefile" "main/Makefile_DSi-3DS"
	@mv "menu/Makefile" "menu/Makefile_DSi-3DS"
	@mv "menu/menu.nds" "menu/menu.bin"
	@mv "menu/menu.bin" "./menu_DSi.bin"
	@mv "main/Relaunch.nds" "./Relaunch_DSi.nds"

buildFlashcart:
	@mv "main/Makefile_Flashcart" "main/Makefile"
	@$(MAKE) -C main
	@mv "menu/Makefile_Flashcart" "menu/Makefile"
	@$(MAKE) -C menu
	@mv "main/Makefile" "main/Makefile_Flashcart"
	@mv "menu/Makefile" "menu/Makefile_Flashcart"
	@mv "menu/menu.nds" "menu/menu.bin"
	@mv "menu/menu.bin" "./menu_Flashcart.bin"
	@mv "main/Relaunch.nds" "./Relaunch_Flashcart.nds"

makecia:
	@cp "./Relaunch_DSi.nds" "./Relaunch_3DS.nds"
	@./make_cia --srl="Relaunch_3DS.nds"
	@rm -fr Relaunch_3DS.nds
	@cp "./menu_DSi.bin" "./menu_3DS.bin"

clean:
	@echo clean build directories
	@mv "main/Makefile_Flashcart" "main/Makefile"
	@mv "menu/Makefile_Flashcart" "menu/Makefile"
	@$(MAKE) -C main clean
	@$(MAKE) -C menu clean
	@mv "main/Makefile" "main/Makefile_Flashcart"
	@mv "menu/Makefile" "menu/Makefile_Flashcart"
	@mv "main/Makefile_DSi-3DS" "main/Makefile"
	@mv "menu/Makefile_DSi-3DS" "menu/Makefile"
	@$(MAKE) -C main clean
	@$(MAKE) -C menu clean
	@mv "main/Makefile" "main/Makefile_DSi-3DS"
	@mv "menu/Makefile" "menu/Makefile_DSi-3DS"

	@echo clean package files
