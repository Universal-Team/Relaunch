/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019
	Evan "Flame" Rodgers
	Ben "Epicpkmn11" Bogie
	Rojelio "RocketRobz" Reyes
	Kim "VoltZ" Perkovic
------------------------------------------------------------------*/

#include "includes.h"
#include "ndsHeader.h"
using namespace std;

static bool dmTextPrinted = false;
static int dmCursorPosition = 0, dmScreenPosition = 0;
u8 stored_SCFG_MC = 0;
bool flashcardMounted = false;
bool secondaryDrive = false;	// false == SD card, true == Flashcard

void dm_drawTopScreen(std::vector<DirEntry> dmItems, int startRow) {
	nocashMessage("running dm_drawTopScreen();");
	//printf ("\x1b[43m"); //yellow
	//nocashMessage("text color set to yellow");
	printf ("\x1b[0;0H");
	nocashMessage("Line switched to 0");
	printf (APP_VERSION);
	nocashMessage("app version printed");

	// Move to 4th row
	printf ("\x1b[3;0H");
	nocashMessage("Line switched to 4");

	for (int i = 0; i < ((int)dmItems.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (dmCursorPosition == i + startRow) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		//for(int i=0;i<dmItems.size();i++) {
    //getIconTitle(dmItems[i].fullPath.c_str(), buffer, title);
		//nocashMessage("icon title found");
//}
		//printf(title); //print the ds rom's title
		//nocashMessage("rom title printed");
		printf((dmItems[i + startRow].name.substr(0, SCREEN_COLS)).c_str());
	}
}

void dm_drawBottomScreen(std::vector<DirEntry> dmItems) {
	nocashMessage("running dm_drawBottomScreen();");
	printf ("\x1b[23;0H");
	nocashMessage("Switched to left of screen?");
	printf (titleName);
	nocashMessage("printed titleName");

	//printf ("\x1b[43m");		// Print background yellow color
	//nocashMessage("text changed to yellow");
	printf ("\x1b[0;0H");
	printf("\n\n No one\n   is\n illegal");
	nocashMessage("'No one is illegal' text printed");
	printf("\x1b[0;1H");
	nocashMessage("Line switched to 1");
	printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
	nocashMessage("fake PUB SIZE printed");
	printf ("\nPRV SIZE: 00000000");
	nocashMessage("fake PRV SIZE printed");
	if (dmItems[dmCursorPosition].name == "DS GAME") {
	nocashMessage("cursor is on DS GAME");
		printf ("\ncart:");
	} else if (dmItems[dmCursorPosition].name == "GBA GAME") {
	nocashMessage("cursor is on GBA GAME");
		printf ("\nslot2:");
	} else if (dmItems[dmCursorPosition].name == "WIFIBOOT") {
	nocashMessage("cursor is on WIFIBOOT");
		printf ("\nwifi:");
	} else if (dmItems[dmCursorPosition].name == "OPTIONS") {
	nocashMessage("cursor is on OPTIONS");
		printf ("\nsett:");
	} else {
	nocashMessage("cursor is on an nds/dsi/srl file");
		printf ("\n%s", dmItems[dmCursorPosition].fullPath.c_str());
	}
}

void driveMenu (std::vector<DirEntry> ndsFiles) {
	nocashMessage("running driveMenu();");
	std::vector<DirEntry> dmItems = ndsFiles;
	int pressed = 0;
	nocashMessage("int 'pressed' defined as 0");

	if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0
	|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
		nocashMessage("menu.bin has been found, printing entries");
		DirEntry options;
		options.name = "OPTIONS";
		dmItems.insert(dmItems.begin(), options);
		nocashMessage("OPTIONS added to list");

		DirEntry wifiboot;
		wifiboot.name = "WIFIBOOT";
		dmItems.insert(dmItems.begin(), wifiboot);
		nocashMessage("WIFIBOOT added to list");

		if (isRegularDS) {
			nocashMessage("This device seems to be a regularDS, adding the GBA GAME option");
			DirEntry gbaGame;
			gbaGame.name = "GBA GAME";
			dmItems.insert(dmItems.begin(), gbaGame);
			nocashMessage("GBA GAME added to list");
		}

		DirEntry dsGame;
		dsGame.name = "DS GAME";
		dmItems.insert(dmItems.begin(), dsGame);
		nocashMessage("DS GAME added to list");
	}

	dmScreenPosition = 0, dmCursorPosition = 0;

	while (true) {
		if (!dmTextPrinted) {
			// Scroll screen if needed
			if (dmCursorPosition < dmScreenPosition) {
				dmScreenPosition = dmCursorPosition;
			} else if (dmCursorPosition > dmScreenPosition + ENTRIES_PER_SCREEN - 1) {
				dmScreenPosition = dmCursorPosition - ENTRIES_PER_SCREEN + 1;
			}

			setFontSub();
			nocashMessage("initiated bottom screen with custom font");
			dm_drawBottomScreen(dmItems);
			nocashMessage("bottom screen drawn");
			setFontTop();
			nocashMessage("initiated top screen with custom font");
			dm_drawTopScreen(dmItems, dmScreenPosition);
			nocashMessage("top screen drawn");

			dmTextPrinted = true;
			nocashMessage("dmTextPrinted has been set to true");
		}

		stored_SCFG_MC = REG_SCFG_MC;

		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();

			if (isDSiMode()) {
				if (REG_SCFG_MC != stored_SCFG_MC) {
					dmTextPrinted = false;
					break;
				}
			}
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A));

		if ((pressed & KEY_UP) && dmCursorPosition > 0) {
			dmCursorPosition--;
			dmTextPrinted = false;
		} else if ((pressed & KEY_DOWN) && dmCursorPosition < (int)dmItems.size()-1) {
			dmCursorPosition++;
			dmTextPrinted = false;
		}

		if (pressed & KEY_A) {
			if (dmItems[dmCursorPosition].name == "DS GAME") {
				dmTextPrinted = false;
				if (flashcardMounted) {
					runNdsFile("fat:/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, false);
				} else {
					runNdsFile("sd:/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, false);
				}
				break;
			} else if (dmItems[dmCursorPosition].name == "GBA GAME" && isRegularDS) {
				dmTextPrinted = false;
				irqDisable(IRQ_VBLANK);
				vramSetBankA(VRAM_A_MAIN_BG);
				vramSetBankB(VRAM_B_MAIN_BG);
				nocashMessage("VRAM banks A and B set");
				// Clear VRAM A and B to show black border for GBA mode
				for (u32 i = 0; i < 0x80000; i++) {
					*(u32*)(0x06000000+i) = 0;
					*(u32*)(0x06200000+i) = 0;
				}
				nocashMessage("VRAM banks A and B have been cleared");
				// Switch to GBA mode
				nocashMessage("switching to GBA Mode");
				runNdsFile("fat:/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, false);
				break;
			} else if (dmItems[dmCursorPosition].name == "WIFIBOOT") {
				dmTextPrinted = false;
				if (flashcardMounted) {
					nocashMessage("Relaunch is running from a flashcard, running nds file from fat:/");
					runNdsFile("fat:/_nds/Relaunch/WIFIBOOT.NDS", 0, NULL, false);
				} else {
					nocashMessage("Relaunch is running from an SD card, running nds file from sd:/");
					runNdsFile("sd:/_nds/Relaunch/WIFIBOOT.NDS", 0, NULL, false);
				}
				break;
			} else if (dmItems[dmCursorPosition].name == "OPTIONS") {
				dmTextPrinted = false;
				screenMode = 2;
				break;
			} else {
				runNdsFile(dmItems[dmCursorPosition].fullPath.c_str(), 0, NULL, false);
				break;
			}
		}
	}
}
