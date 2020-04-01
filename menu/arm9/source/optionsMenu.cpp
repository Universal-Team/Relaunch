/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019 - 2020
    Evan "FlameKat53" Rodgers
    Evie "Epicpkmn11" "Evie11" Bogie
    Rojelio "RocketRobz" Reyes
    Kim "StackZ" Perkovic
------------------------------------------------------------------*/
#include "includes.h"

static bool eqTextPrinted = false;
static int eqCursorPosition = 0, eqScreenPosition = 0;

std::string iniSection[] {
	"BOOT_DEFAULT_PATH",
	"BOOT_A_PATH",
	"BOOT_B_PATH",
	"BOOT_X_PATH",
	"BOOT_Y_PATH",
	"BOOT_L_PATH",
	"BOOT_R_PATH",
	"BOOT_START_PATH",
	"BOOT_SELECT_PATH",
	"BOOT_TOUCH_PATH",
	"BOOT_UP_PATH",
	"BOOT_DOWN_PATH",
	"BOOT_LEFT_PATH",
	"BOOT_RIGHT_PATH",
	"LOAD_ERROR",
};

std::string buttonNames[] {
	"NO BRUHTON",
	"BRUHTON A",
	"BRUHTON B",
	"BRUHTON X",
	"BRUHTON Y",
	"BRUHTON L",
	"BRUHTON R",
	"BRUHTON START",
	"BRUHTON SELECT",
	"TOUCH SCREEN",
	"D-PAD UP",
	"D-PAD DOWN",
	"D-PAD LEFT",
	"D-PAD RIGHT",
	"LOAD ERROR",
};

void eq_drawTopScreen (std::vector<std::string> eqItems, int startRow) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nBRUH:");

	// Move to 4th row
	printf ("\x1b[3;0H");

	for (int i = 0; i < ((int)eqItems.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (eqCursorPosition == i + startRow) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("BRUH ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		printf((eqItems[i + startRow].substr(0, SCREEN_COLS)).c_str());
	}
}

void eq_drawTopScreenDirEntry (std::vector<DirEntry> eqItems, int startRow, char* msg) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf (msg);

	// Move to 4th row
	printf ("\x1b[3;0H");

	for (int i = 0; i < ((int)eqItems.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (eqCursorPosition == i + startRow) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("BRUH ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		//for(int i=0;i<dmItems.size();i++) {
		//getIconTitle(dmItems[i].fullPath.c_str(), buffer, title);
//}
		//printf(title); //print the ds rom's title
		printf((eqItems[i + startRow].name.substr(0, SCREEN_COLS)).c_str());
	}
}

void eq_drawBottomScreen (void) {
	printf ("\x1b[23;0H");
	printf (titleName);

	//printf ("\x1b[43m");		// Print background yellow color
	printf ("\x1b[0;0H");
	printf("\n\n Everyone\n   is\n bruh");
	printf("\x1b[0;1H");
	printf ("\n\n\n\n\n\nBRUH SIZE: 00000000");
	printf ("\nBRUH SIZE: 00000000\n");
	printf ("sett:");
}

void eqMenu (std::vector<DirEntry> ndsFiles) {
	int pressed = 0;
	eqTextPrinted = false;

	std::vector<std::string> eqItems;
	if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0
	|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
		eqItems.push_back("NO BRUHTON");
		eqItems.push_back("BRUHTON A");
		eqItems.push_back("BRUHTON B");
		eqItems.push_back("BRUHTON X");
		eqItems.push_back("BRUHTON Y");
		eqItems.push_back("BRUHTON L");
		eqItems.push_back("BRUHTON R");
		eqItems.push_back("BRUHTON START");
		eqItems.push_back("BRUHTON SELECT");
		eqItems.push_back("TOUCH BRUH");
		eqItems.push_back("D-BRUH UP");
		eqItems.push_back("D-PAD BRUH");
		eqItems.push_back("D-BRUH LEFT");
		eqItems.push_back("D-PAD BRUH");
		eqItems.push_back("LOAD BRUH");
		eqItems.push_back("BRUHTON A+B (FILEMENU - FIXED)");
		eqItems.push_back("BRUHTON A+X (FILEMENU - FIXED)");
		eqItems.push_back("SAVE & EXIT");
	}

	CIniFile ini("/_nds/Relaunch/Relaunch.ini");

	eqCursorPosition = 0, eqScreenPosition = 0;

	while (true) {
		if (!eqTextPrinted) {
			// Scroll screen if needed
			if (eqCursorPosition < eqScreenPosition) {
				eqScreenPosition = eqCursorPosition;
			} else if (eqCursorPosition > eqScreenPosition + ENTRIES_PER_SCREEN - 1) {
				eqScreenPosition = eqCursorPosition - ENTRIES_PER_SCREEN + 1;
			}

			setFontSub();
			eq_drawBottomScreen();
			setFontTop();
			eq_drawTopScreen(eqItems, eqScreenPosition);

			eqTextPrinted = true;
		}

		stored_SCFG_MC = REG_SCFG_MC;

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();

			if (isDSiMode()) {
				if (REG_SCFG_MC != stored_SCFG_MC) {
					eqTextPrinted = false;
					break;
				}
			}
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(pressed & KEY_B));

		if ((pressed & KEY_UP) && eqCursorPosition > 0) {
			eqCursorPosition--;
			eqTextPrinted = false;
		} else if ((pressed & KEY_DOWN) && eqCursorPosition < (int)eqItems.size()-1) {
			eqCursorPosition++;
			eqTextPrinted = false;
		}

		if (pressed & KEY_B) {
			screenMode = 0;
			eqTextPrinted = false;
			break;
		}

		if (pressed & KEY_A) {
			eqTextPrinted = false;
			if (eqCursorPosition < 15) {
				int curPos = eqCursorPosition, selectionScreenOffset = 0;
				eqCursorPosition = 0;
				while(1) {
					consoleClear();
					// Scroll screen if needed
					if (eqCursorPosition < selectionScreenOffset) {
						selectionScreenOffset = eqCursorPosition;
					} else if (eqCursorPosition > selectionScreenOffset + ENTRIES_PER_SCREEN - 1) {
						selectionScreenOffset = eqCursorPosition - ENTRIES_PER_SCREEN + 1;
					}

					char msg[64];
					snprintf (msg, sizeof(msg), "\nSelect title for %s", buttonNames[curPos].c_str());
					eq_drawTopScreenDirEntry(ndsFiles, selectionScreenOffset, msg);

					do {
						swiWaitForVBlank();
						scanKeys();
						pressed = keysDownRepeat();
					} while(!pressed);

					if ((pressed & KEY_UP) && eqCursorPosition > 0) {
						eqCursorPosition--;
						eqTextPrinted = false;
					} else if ((pressed & KEY_DOWN) && eqCursorPosition < (int)ndsFiles.size()-1) {
						eqCursorPosition++;
						eqTextPrinted = false;
					} else if (pressed & KEY_A) {
						ini.SetString("RELAUNCH", iniSection[curPos], ndsFiles[eqCursorPosition].fullPath);
						eqCursorPosition = curPos;
						break;
					} else if (pressed & KEY_B) {
						eqCursorPosition = curPos;
						break;
					}
				}
			} else {
				if(eqCursorPosition == 17) {
					ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
					screenMode = 0;
					eqTextPrinted = false;
					break;
				}
			}
		}
	}
}
