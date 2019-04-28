#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all package main menu

all:	main menu

package-release: release
	@mkdir -p "$(PACKAGE)"
	@cp "retail/bin/nds-bootstrap-release.nds" "$(PACKAGE)/nds-bootstrap-release.nds"
	@cp "hb/bin/nds-bootstrap-hb-release.nds" "$(PACKAGE)/nds-bootstrap-hb-release.nds"

main:
	@$(MAKE) -C main

menu:
	@$(MAKE) -C menu

clean:
	@echo clean build directories
	@$(MAKE) -C main clean
	@$(MAKE) -C menu clean

	@echo clean package files
