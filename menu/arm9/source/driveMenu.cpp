/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019
	Evan "Flame" Rodgers
	Ben "Epicpkmn11" Bogie
	Rojelio "RocketRobz" Reyes
------------------------------------------------------------------*/

#include "includes.h"

using namespace std;

static bool dmTextPrinted = false;
static int dmCursorPosition = 0, dmScreenPosition = 0;
u8 stored_SCFG_MC = 0;
bool flashcardMounted = false;
bool secondaryDrive = false;	// false == SD card, true == Flashcard

void dm_drawTopScreen(std::vector<DirEntry> dmItems, int startRow) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf (appVersion);

	// Move to 4th row
	printf ("\x1b[3;0H");

	for (int i = 0; i < ((int)dmItems.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (dmCursorPosition == i + startRow) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		printf((dmItems[i + startRow].name.substr(0, SCREEN_COLS)).c_str());
	}
}

void dm_drawBottomScreen(std::vector<DirEntry> dmItems) {
	printf ("\x1b[23;0H");
	printf (titleName);

	//printf ("\x1b[43m");		// Print background yellow color
	printf ("\x1b[0;0H");
	printf("\n\n No one\n   is\n illegal");
	printf("\x1b[0;1H");
	printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
	printf ("\nPRV SIZE: 00000000");
	if (dmItems[dmCursorPosition].name == "DS GAME") {
		printf ("\ncart:");
	} else if (dmItems[dmCursorPosition].name == "GBA GAME") {
		printf ("\nslot2:");
	} else if (dmItems[dmCursorPosition].name == "WIFIBOOT") {
		printf ("\nwifi:");
	} else if (dmItems[dmCursorPosition].name == "OPTIONS") {
		printf ("\nsett:");
	} else {
		printf ("\n%s", dmItems[dmCursorPosition].fullPath.c_str());
	}
}

void driveMenu (std::vector<DirEntry> ndsFiles) {
	std::vector<DirEntry> dmItems = ndsFiles;
	int pressed = 0;

	if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0
	|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
		DirEntry options;
		options.name = "OPTIONS";
		dmItems.insert(dmItems.begin(), options);

		DirEntry wifiboot;
		wifiboot.name = "WIFIBOOT";
		dmItems.insert(dmItems.begin(), wifiboot);

		if (isRegularDS) {
			DirEntry gbaGame;
			gbaGame.name = "GBA GAME";
			dmItems.insert(dmItems.begin(), gbaGame);
		}

		DirEntry dsGame;
		dsGame.name = "DS GAME";
		dmItems.insert(dmItems.begin(), dsGame);
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
			dm_drawBottomScreen(dmItems);
			setFontTop();
			dm_drawTopScreen(dmItems, dmScreenPosition);

			dmTextPrinted = true;
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
				// Clear VRAM A and B to show black border for GBA mode
				for (u32 i = 0; i < 0x80000; i++) {
					*(u32*)(0x06000000+i) = 0;
					*(u32*)(0x06200000+i) = 0;
				}
				// Switch to GBA mode
				runNdsFile("fat:/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, false);
				break;
			} else if (dmItems[dmCursorPosition].name == "WIFIBOOT") {
				dmTextPrinted = false;
				if (flashcardMounted) {
					runNdsFile("fat:/_nds/Relaunch/WIFIBOOT.NDS", 0, NULL, false);
				} else {
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
