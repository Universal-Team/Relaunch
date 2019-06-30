#include "optionsMenu.h"
#include <nds.h>
#include <unistd.h>

#include "driveMenu.h"
#include "main.h"

static bool eqTextPrinted = false;
static int eqCursorPosition = 0, eqScreenPosition = 0;

static bool noLock = false, aLock = false, bLock = false,
			xLock = false, yLock = false, lLock = false, rLock = false,
			startLock = false, selectLock = false, upLock = false, downLock = false,
			leftLock = false, rightLock = false, touchLock = false, errorLock = false;

void eq_drawTopScreen(std::vector<std::string> eqItems, int startRow) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nRelaunch.nds v0.3");

	// Move to 4th row
	printf ("\x1b[3;0H");

	for (int i = 0; i < ((int)eqItems.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (eqCursorPosition == i + startRow) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		printf((eqItems[i + startRow].substr(0, SCREEN_COLS)).c_str());
	}
}

void eq_drawTopScreenDirEntry(std::vector<DirEntry> eqItems, int startRow) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nRelaunch.nds v0.3");

	// Move to 4th row
	printf ("\x1b[3;0H");

	for (int i = 0; i < ((int)eqItems.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (eqCursorPosition == i + startRow) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		printf((eqItems[i + startRow].name.substr(0, SCREEN_COLS)).c_str());
	}
}

void eq_drawBottomScreen(void) {
	printf ("\x1b[23;0H");
	printf (titleName);

	//printf ("\x1b[43m");		// Print background yellow color
	printf ("\x1b[0;0H");
	printf("\n\n No one\n   is\n illegal");
	printf("\x1b[0;1H");
	printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
	printf ("\nPRV SIZE: 00000000\n");
	printf ("sett:");
}

void eqMenu (std::vector<DirEntry> ndsFiles) {
	int pressed = 0;
	noLock = false, aLock = false, bLock = false, xLock = false,
	yLock = false, lLock = false, rLock = false, upLock = false,
	downLock = false, leftLock = false, rightLock = false, startLock = false,
	selectLock = false, touchLock = false, errorLock = false, eqTextPrinted = false;

	std::vector<std::string> eqItems;
	if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0
	|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
		eqItems.push_back("NO BUTTON");
		eqItems.push_back("BUTTON A");
		eqItems.push_back("BUTTON B");
		eqItems.push_back("BUTTON X");
		eqItems.push_back("BUTTON Y");
		eqItems.push_back("BUTTON L");
		eqItems.push_back("BUTTON R");
		eqItems.push_back("BUTTON START");
		eqItems.push_back("BUTTON SELECT");
		eqItems.push_back("TOUCH SCREEN");
		eqItems.push_back("D-PAD UP");
		eqItems.push_back("D-PAD DOWN");
		eqItems.push_back("D-PAD LEFT");
		eqItems.push_back("D-PAD RIGHT");
		eqItems.push_back("LOAD ERROR");
		eqItems.push_back("BUTTON A+B (FILEMENU - FIXED)");
		eqItems.push_back("SAVE & EXIT");
	}

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
			consoleClear();
			eqTextPrinted = false;
			if (eqCursorPosition < 15) {
				int curPos = eqCursorPosition, selectionScreenOffset = 0;
				eqCursorPosition = 0;
				while(1) {
					// Scroll screen if needed
					if (eqCursorPosition < selectionScreenOffset) {
						selectionScreenOffset = eqCursorPosition;
					} else if (eqCursorPosition > selectionScreenOffset + ENTRIES_PER_SCREEN - 1) {
						selectionScreenOffset = eqCursorPosition - ENTRIES_PER_SCREEN + 1;
					}

					eq_drawTopScreenDirEntry(ndsFiles, selectionScreenOffset);
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
						eqCursorPosition = curPos;
						break;
					} else if (pressed & KEY_B) {
						eqCursorPosition = curPos;
						break;
					}
				}
			}
		}
			// if (eqAssignedOp[eqCursorPosition] == 0) {
			// 	noLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 1) {
			// 	aLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 2) {
			// 	bLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 3) {
			// 	xLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 4) {
			// 	yLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 5) {
			// 	lLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 6) {
			// 	rLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 7) {
			// 	startLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 8) {
			// 	selectLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 9) {
			// 	touchLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 10) {
			// 	upLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 11) {
			// 	downLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 12) {
			// 	leftLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 13) {
			// 	rightLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 14) {
			// 	errorLock = true;
			// 	screenMode = 1;
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 15) {
			// 	break;
			// } else if (eqAssignedOp[eqCursorPosition] == 16) {
			// 	eqTextPrinted = false;
			// 	fileMenu = true;
			// 	//ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			// 	screenMode = 0;
			// 	break;
			// }
		// }
	}
}