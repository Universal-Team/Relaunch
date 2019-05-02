
#----------------------------------------------------------------------------------------
# Goals for Build
#----------------------------------------------------------------------------------------

3DS: .PHONY3DS

DSi: .PHONYDSi

Flashcart: .PHONYFlashcart

.PHONY3DS: all3DS mainDSi3DS menuDSi3DS renameDSi3DS moveDSi3DS makecia

.PHONYDSi: allDSi mainDSi3DS menuDSi3DS renameDSi3DS moveDSi3DS

.PHONYFlashcart: allFlashcart mainFlashcart menuFlashcart renameFlashcart moveFlashcart

allDSi:	mainDSi3DS menuDSi3DS renameDSi3DS moveDSi3DS

all3DS:	mainDSi3DS menuDSi3DS renameDSi3DS moveDSi3DS makecia

allFlashcart: mainFlashcart menuFlashcart renameFlashcart moveFlashcart

mainDSi3DS:
	@mv "main/Makefile_DSi-3DS" "main/Makefile"
	@$(MAKE) -C main

menuDSi3DS:
	@mv "menu/Makefile_Flashcart" "menu/Makefile"
	@$(MAKE) -C menu
	@mv "menu/Makefile" "menu/Makefile_Flashcart"
	@mv "menu/Makefile_DSi-3DS" "menu/Makefile"
	@$(MAKE) -C menu
	
renameDSi3DS:
	@mv "main/Makefile" "main/Makefile_DSi-3DS"
	@mv "menu/Makefile" "menu/Makefile_DSi-3DS"
	@mv "menu/menu.nds" "menu/menu.bin"
	
moveDSi3DS:
	@mv "menu/menu.bin" "./menu_DSi.bin"
	@mv "main/Relaunch.nds" "./Relaunch_DSi.nds"
	
makecia:
	@mv "./Relaunch_DSi.nds" "./Relaunch_3DS.nds"
	@./make_cia --srl="Relaunch_3DS.nds"
	@rm -fr Relaunch_3DS.nds
	@mv "./menu_DSi.bin" "./menu_3DS.bin"

mainFlashcart:
	@mv "main/Makefile_Flashcart" "main/Makefile"
	@$(MAKE) -C main

menuFlashcart:
	@mv "menu/Makefile_Flashcart" "menu/Makefile"
	@$(MAKE) -C menu
	
renameFlashcart:
	@mv "main/Makefile" "main/Makefile_Flashcart"
	@mv "menu/Makefile" "menu/Makefile_Flashcart"
	@mv "menu/menu.nds" "menu/menu.bin"
	
moveFlashcart:
	@mv "menu/menu.bin" "./menu_Flashcart.bin"
	@mv "main/Relaunch.nds" "./Relaunch_Flashcart.nds"

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
