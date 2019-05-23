/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

//basic things
#include <nds.h>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include "nds_loader_arm9.h"
#include "main.h"
#include "menu.h"
#include "Operations.h"
#include "inifile.h"
#include "font.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 19
#define ENTRIES_START_ROW 3
#define OPTIONS_ENTRIES_START_ROW 2
#define ENTRIES_START_ROW_EQ 3
#define ENTRY_PAGE_LENGTH 10
bool bigJump = false;
using namespace std;
static char path[PATH_MAX];

bool flashcardMountSkipped = true;
static bool dmTextPrinted = false;
static int dmCursorPosition = 0;
static int dmAssignedOp[3] = {-1};
static int dmMaxCursors = -1;
static bool eqTextPrinted = false;
static int eqCursorPosition = 0;
static int eqAssignedOp[3] = {-1};
static int eqMaxCursors = -1;
static int noLock = 0;
static int aLock = 0;
static int bLock = 0;
static int xLock = 0;
static int yLock = 0;
static int errorLock = 0;

void loadGbaCart(void) {
	irqDisable(IRQ_VBLANK);
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	// Clear VRAM A and B to show black border for GBA mode
	for (u32 i = 0; i < 0x80000; i++) {
		*(u32*)(0x06000000+i) = 0;
		*(u32*)(0x06200000+i) = 0;
	}
// Switch to GBA mode
	runNdsFile("/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, false);
	}
void loadDSCart() {
runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, false);
}
void dm_drawTopScreen(void) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nRelaunch.nds v0.2");

	// Move to 4th row
	printf ("\x1b[3;0H");

	if (dmMaxCursors == -1) {
		printf ("This Shouldn't Happen!");
	} else
	for (int i = 0; i <= dmMaxCursors; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (dmCursorPosition == i) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		if (dmAssignedOp[i] == 0) {
			printf ("DS GAME");
		} else if (dmAssignedOp[i] == 1) {
			printf ("GBA GAME");
		} else if (dmAssignedOp[i] == 2) {
			printf ("WIFIBOOT");
		} else if (dmAssignedOp[i] == 3) {
			printf ("OPTIONS");
		}
	}
}
			    
void dm_drawBottomScreen(void) {
	printf ("\x1b[23;0H");
	printf (titleName);

	//printf ("\x1b[43m");		// Print background yellow color
	printf ("\x1b[0;0H");
	printf("\n\n No one\n   is\n illegal");
	printf("\x1b[0;1H");
	if (dmAssignedOp[dmCursorPosition] == 0) {
		printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
		printf ("\nPRV SIZE: 00000000");
		printf ("\ncart:");
	} else if (dmAssignedOp[dmCursorPosition] == 1) {
		printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
		printf ("\nPRV SIZE: 00000000");
		printf ("\nslot2:");
	} else if (dmAssignedOp[dmCursorPosition] == 2) {
		printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
		printf ("\nPRV SIZE: 00000000");
		printf ("\nwifi:");
	} else if (dmAssignedOp[dmCursorPosition] == 3) {
		printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
		printf ("\nPRV SIZE: 00000000");
		printf ("\nsett:");
	}
}

void driveMenu (void) {
	int pressed = 0;
	int held = 0;
	/*int fileOffset = 0;
	int screenOffset;
	if (!isDSiMode()) {
		screenOffset = 5;
	} else { 
		screenOffset = 4;
	}
	vector<DirEntry> dirContents;
	getDirectoryContents(dirContents);*/

	while (true) {
		for (int i = 0; i < 3; i++) {
			dmAssignedOp[i] = -1;
		}
		dmMaxCursors = -1;
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 0;
		}
		if (!isDSiMode() && isRegularDS) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 1;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 2;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 3;
		}

		if (dmCursorPosition < 0) 	dmCursorPosition = dmMaxCursors;		// Wrap around to bottom of list
		if (dmCursorPosition > dmMaxCursors)	dmCursorPosition = 0;		// Wrap around to top of list

		if (!dmTextPrinted) {
		setFontSub();
			dm_drawBottomScreen();
		setFontTop();
			dm_drawTopScreen();
			//showDirectoryContents(dirContents, fileOffset, screenOffset);

			dmTextPrinted = true;
		}

		stored_SCFG_MC = REG_SCFG_MC;

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
	
			scanKeys();
			pressed = keysDownRepeat();
			held = keysHeld();
			swiWaitForVBlank();

			if (isDSiMode()) {
				if (REG_SCFG_MC != stored_SCFG_MC) {
					dmTextPrinted = false;
					break;
				}
			}
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(held & KEY_R));

		if ((pressed & KEY_UP) && dmMaxCursors != -1) {
			dmCursorPosition -= 1;
			dmTextPrinted = false;
		}
		if ((pressed & KEY_DOWN) && dmMaxCursors != -1) {
			dmCursorPosition += 1;
			dmTextPrinted = false;
		}

		if (dmCursorPosition < 0) 	dmCursorPosition = dmMaxCursors;		// Wrap around to bottom of list
		if (dmCursorPosition > dmMaxCursors)	dmCursorPosition = 0;		// Wrap around to top of list

		if (pressed & KEY_A) {
			if (dmAssignedOp[dmCursorPosition] == 0) {
				dmTextPrinted = false;
				loadDSCart();
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 1 && isRegularDS) {
				dmTextPrinted = false;
				loadGbaCart();
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 2) {
				dmTextPrinted = false;
				runNdsFile("/_nds/Relaunch/WIFIBOOT.NDS", 0, NULL, false);
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 3) {
				dmTextPrinted = false;
				screenMode = 2;
				break;
			}
		}
	}
}

// file browse stuff below!
// file browse stuff below! (yes this is on 2 lines)

void OnKeyPressed(int key) {
	if(key > 0)
		iprintf("%c", key);
}

bool nameEndsWith (const string& name) {

	if (name.size() == 0) return false;

	return true;
}

bool dirEntryPredicate (const DirEntry& lhs, const DirEntry& rhs) {

	if (!lhs.isDirectory && rhs.isDirectory) {
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory) {
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents (vector<DirEntry>& dirContents) {
	struct stat st;
	dirContents.clear();

	DIR *pdir = opendir ("."); 
	
	if (pdir == NULL) {
		iprintf ("Unable to open the directory.\n");
	} else {

		while(true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;

			stat(pent->d_name, &st);
			if (strcmp(pent->d_name, "..") != 0) {
				dirEntry.name = pent->d_name;
				dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;
				if (!dirEntry.isDirectory) {
					dirEntry.size = getFileSize(dirEntry.name.c_str());
				}
				if((dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "nds")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "NDS")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "argv")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "ARGV")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "dsi")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "DSI")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "app")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "APP"))
				{
					dirEntry.isApp = true;
				} else {
					dirEntry.isApp = false;
				}

if ((strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "nds") == 0) ||
	(strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "NDS") == 0) ||
    (strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "argv") == 0) ||
	(strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "ARGV") == 0) ||
    (strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "dsi") == 0) ||
	(strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "DSI") == 0) ||
	(strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "app") == 0) ||
	(strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "APP") == 0) ||
    (dirEntry.name.compare(".") != 0 && (dirEntry.isDirectory))) {
    dirContents.push_back (dirEntry);
}
		}
	}
		
		closedir(pdir);
}
	
	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);

	DirEntry dirEntry;
	dirEntry.isDirectory = true;
	dirEntry.isApp = false;
}

void showDirectoryContents (const vector<DirEntry>& dirContents, int fileOffset, int startRow) {
	getcwd(path, PATH_MAX);
	
	// Print the path
	//printf ("\x1b[43m"); 	// Print yellow color
	printf ("\x1b[0;0H");
	if (strlen(path) < SCREEN_COLS) {
		iprintf ("\n%s", path);
	} else {
		iprintf ("\n%s", path + strlen(path) - SCREEN_COLS);
	}
	
	// Move to 2nd row
	iprintf ("\x1b[1;0H");
	
	// Print directory listing
	for (int i = 0; i < ((int)dirContents.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		char entryName[SCREEN_COLS + 1];

		// Set row if in the file browser
		if (screenMode == 1) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if ((fileOffset - startRow) == i) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
	}
}

		strncpy (entryName, entry->name.c_str(), SCREEN_COLS);
		entryName[SCREEN_COLS] = '\0';
		printf (entryName);
		if (entry->isDirectory) {
			printf ("\x1b[%d;27H", i + ENTRIES_START_ROW);
			printf ("(dir)");
		} else {
			printf ("\x1b[%d;23H", i + ENTRIES_START_ROW);
		}
	}
}

int fileBrowse_A(DirEntry* entry, char path[PATH_MAX]) {
	int pressed = 0;
	int assignedOp[3] = {0};
	int optionOffset = 0;
	int cursorScreenPos = 0;
	int maxCursors = -1;

	printf ("\x1b[0;27H");
	setFontSub();
	char fullPath[256];
	snprintf(fullPath, sizeof(fullPath), "%s%s", path, entry->name.c_str());
	printf(fullPath);
	// Position cursor, depending on how long the full file path is
	for (int i = 0; i < 256; i++) {
		if (i == 33 || i == 65 || i == 97 || i == 129 || i == 161 || i == 193 || i == 225) {
			cursorScreenPos++;
		}
		if (fullPath[i] == '\0') {
			break;
		}
	}
	//printf ("\x1b[42m");
	iprintf ("\x1b[%d;0H", cursorScreenPos + OPTIONS_ENTRIES_START_ROW);
	if (entry->isApp) {
		maxCursors++;
		assignedOp[maxCursors] = 0;
		printf("   Boot file\n");
	}
	if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
		maxCursors++;
		assignedOp[maxCursors] = 1;
		printf("   Set as hotkey app\n");
	}
	while (true) {
		// Clear old cursors
		for (int i = OPTIONS_ENTRIES_START_ROW+cursorScreenPos; i < (maxCursors+1) + OPTIONS_ENTRIES_START_ROW+cursorScreenPos; i++) {
			iprintf ("\x1b[%d;0H  ", i);
		}
		// Show cursor
		//iprintf ("\x1b[46m"); // cyan
		iprintf ("\x1b[%d;0H >", optionOffset + OPTIONS_ENTRIES_START_ROW+cursorScreenPos);
		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN)
				&& !(pressed & KEY_A) && !(pressed & KEY_B));

		if (pressed & KEY_UP) 		optionOffset -= 1;
		if (pressed & KEY_DOWN) 	optionOffset += 1;

		if (optionOffset < 0) 				optionOffset = maxCursors;		// Wrap around to bottom of list
		if (optionOffset > maxCursors)		optionOffset = 0;		// Wrap around to top of list

		if (pressed & KEY_A) {
			if (assignedOp[optionOffset] == 0) {
				applaunch = true;
				iprintf ("\x1b[%d;3H", optionOffset + OPTIONS_ENTRIES_START_ROW+cursorScreenPos);
				printf("Now loading...");
			} else if (assignedOp[optionOffset] == 1) {
	// Clear the screen
	iprintf ("\x1b[2J");
				printf("Press the button to set\nas the hotkey");
				for (int i = 0; i < 60*1; i++) {
					swiWaitForVBlank();
				}
				CIniFile ini("/_nds/Relaunch/Relaunch.ini");
while (true) {
do {
	scanKeys();
	pressed = keysHeld();
	swiWaitForVBlank();
} while (!pressed);
	if (pressed & KEY_A) {
		ini.SetString("RELAUNCH", "BOOT_A_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_B) {
		ini.SetString("RELAUNCH", "BOOT_B_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_X) {
		ini.SetString("RELAUNCH", "BOOT_X_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_Y) {
		ini.SetString("RELAUNCH", "BOOT_Y_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_L) {
		ini.SetString("RELAUNCH", "BOOT_L_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_R) {
		ini.SetString("RELAUNCH", "BOOT_R_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_START) {
		ini.SetString("RELAUNCH", "BOOT_START_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_SELECT) {
		ini.SetString("RELAUNCH", "BOOT_SELECT_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_TOUCH) {
		ini.SetString("RELAUNCH", "BOOT_TOUCH_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_UP) {
		ini.SetString("RELAUNCH", "BOOT_UP_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_DOWN) {
		ini.SetString("RELAUNCH", "BOOT_DOWN_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_LEFT) {
		ini.SetString("RELAUNCH", "BOOT_LEFT_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else if (pressed & KEY_RIGHT) {
		ini.SetString("RELAUNCH", "BOOT_RIGHT_PATH", fullPath);
		ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
			break;
	} else {
			return (false);
	}
}
			}
			return assignedOp[optionOffset];
		}
		if (pressed & KEY_B) {
			return -1;
		}
	}
}

void fileBrowse_drawBottomScreen(DirEntry* entry, int fileOffset) {
	char fullPath[256];
	snprintf(fullPath, sizeof(fullPath), "%s%s", path, entry->name.c_str());

	printf ("\x1b[0;0H");
	printf("\n\n No one\n   is\n illegal");
	printf("\x1b[0;1H");
		printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
		printf ("\nPRV SIZE: 00000000\n");
		printf (fullPath);
}

string browseForFile (void) {
	int pressed = 0;
	int held = 0;
	int screenOffset = 0;
	int fileOffset = 0;
	vector<DirEntry> dirContents;
	
	getDirectoryContents (dirContents);

	while (true) {
		DirEntry* entry = &dirContents.at(fileOffset);

		setFontSub();
		fileBrowse_drawBottomScreen(entry, fileOffset);
		setFontTop();
		showDirectoryContents(dirContents, fileOffset, screenOffset);

		stored_SCFG_MC = REG_SCFG_MC;

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
	
			scanKeys();
			pressed = keysDownRepeat();
			held = keysHeld();
			swiWaitForVBlank();

			if (REG_SCFG_MC != stored_SCFG_MC) {
				break;
			}

			if ((held & KEY_R) && (pressed & KEY_L)) {
				break;
			}
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_LEFT) && !(pressed & KEY_RIGHT)
				&& !(pressed & KEY_A) && !(pressed & KEY_B));
	
		iprintf ("\x1b[%d;0H", fileOffset - screenOffset + ENTRIES_START_ROW);

		if (pressed & KEY_UP) {		fileOffset -= 1; bigJump = false; }
		if (pressed & KEY_DOWN) {	fileOffset += 1; bigJump = false; }
		if (pressed & KEY_LEFT) {	fileOffset -= ENTRY_PAGE_LENGTH; bigJump = true; }
		if (pressed & KEY_RIGHT) {	fileOffset += ENTRY_PAGE_LENGTH; bigJump = true; }
		
		if ((fileOffset < 0) & (bigJump == false))	fileOffset = dirContents.size() - 1;	// Wrap around to bottom of list (UP press)
		else if ((fileOffset < 0) & (bigJump == true))	fileOffset = 0;		// Move to bottom of list (RIGHT press)
		if ((fileOffset > ((int)dirContents.size() - 1)) & (bigJump == false))	fileOffset = 0;		// Wrap around to top of list (DOWN press)
		else if ((fileOffset > ((int)dirContents.size() - 1)) & (bigJump == true))	fileOffset = dirContents.size() - 1;	// Move to top of list (LEFT press)


		// Scroll screen if needed
		if (fileOffset < screenOffset) 	{
			screenOffset = fileOffset;
			showDirectoryContents (dirContents, fileOffset, screenOffset);
		}
		if (fileOffset > screenOffset + ENTRIES_PER_SCREEN - 1) {
			screenOffset = fileOffset - ENTRIES_PER_SCREEN + 1;
			showDirectoryContents (dirContents, fileOffset, screenOffset);
		}

		getcwd(path, PATH_MAX);

		if (pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if (entry->isDirectory) {
				//printf("\x1b[46m"); // print cyan color
				iprintf("  Entering directory\n");
				// Enter selected directory
				chdir (entry->name.c_str());
				getDirectoryContents(dirContents);
				screenOffset = 0;
				fileOffset = 0;
			} else if (entry->isApp) {
				int getOp = fileBrowse_A(entry, path);
				if (getOp == 0) {
					// Return the chosen file
					return entry->name;
				}
			}
		}
			if (pressed & KEY_B) {
				screenMode = 2;
				return "null";
		}
	}
}

// OPTIONS MENU THINGS BELOW
// OPTIONS MENU THINGS BELOW

void eq_drawTopScreen(void) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nCHANGE BOOT DEFAULT/HOTKEYS:");

	// Move to 4th row
	printf ("\x1b[3;0H");

	if (eqMaxCursors == -1) {
		printf ("This shouldn't happen!");
	} else
	for (int i = 0; i <= eqMaxCursors; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW_EQ);
		if (eqCursorPosition == i) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		if (eqAssignedOp[i] == 0) {
			printf ("NO BUTTON");
		} else if (eqAssignedOp[i] == 1) {
			printf ("BUTTON A");
		} else if (eqAssignedOp[i] == 2) {
			printf ("BUTTON B");
		} else if (eqAssignedOp[i] == 3) {
			printf ("BUTTON X");
		} else if (eqAssignedOp[i] == 4) {
			printf ("BUTTON Y");
		} else if (eqAssignedOp[i] == 5) {
			printf ("LOAD ERROR");
		} else if (eqAssignedOp[i] == 6) {
			printf ("BUTTON A+B (FILEMENU - FIXED)");
		} else if (eqAssignedOp[i] == 7) {
			printf ("SAVE & EXIT");
		}
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
void eqMenu (void) {
	int pressed = 0;
	int held = 0;
	noLock = 0;
	aLock = 0;
	bLock = 0;
	xLock = 0;
	yLock = 0;
	errorLock = 0;
	eqTextPrinted = false;

	while (true) {
		for (int i = 0; i < 3; i++) {
			eqAssignedOp[i] = -1;
		}
		eqMaxCursors = -1;
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0){
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 0;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 1;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 2;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 3;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 4;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 5;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 6;
		}
		if (access("/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 7;
		}

		if (eqCursorPosition < 0) 	eqCursorPosition = eqMaxCursors;		// Wrap around to bottom of list
		if (eqCursorPosition > eqMaxCursors)	eqCursorPosition = 0;		// Wrap around to top of list

		if (!eqTextPrinted) {
		setFontSub();
			eq_drawBottomScreen();
		setFontTop();
			eq_drawTopScreen();

			eqTextPrinted = true;
		}

		stored_SCFG_MC = REG_SCFG_MC;

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
	
			scanKeys();
			pressed = keysDownRepeat();
			held = keysHeld();
			swiWaitForVBlank();

			if (isDSiMode()) {
				if (REG_SCFG_MC != stored_SCFG_MC) {
					eqTextPrinted = false;
					break;
				}
			}
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(pressed & KEY_B));

		if ((pressed & KEY_UP) && eqMaxCursors != -1) {
			eqCursorPosition -= 1;
			eqTextPrinted = false;
		}
		if ((pressed & KEY_DOWN) && eqMaxCursors != -1) {
			eqCursorPosition += 1;
			eqTextPrinted = false;
		}
		if (pressed & KEY_B) {
		screenMode = 0;
		eqTextPrinted = false;
		break;
		}

		if (eqCursorPosition < 0) 	eqCursorPosition = eqMaxCursors;		// Wrap around to bottom of list
		if (eqCursorPosition > eqMaxCursors)	eqCursorPosition = 0;		// Wrap around to top of list

		if (pressed & KEY_A) {
			if (eqAssignedOp[eqCursorPosition] == 0) {
				eqTextPrinted = false;
				secondaryDrive = true;
				if (flashcardMounted) {
				secondaryDrive = true;
				chdir("fat:/");
				} else {
				secondaryDrive = false;
				chdir("sd:/");
				}
				noLock = 1;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 1) {
				eqTextPrinted = false;
				if (flashcardMounted) {
				secondaryDrive = true;
				chdir("fat:/");
				} else {
				secondaryDrive = false;
				chdir("sd:/");
				}
				aLock = 1;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 2) {
				eqTextPrinted = false;
				if (flashcardMounted) {
				secondaryDrive = true;
				chdir("fat:/");
				} else {
				secondaryDrive = false;
				chdir("sd:/");
				}
				bLock = 1;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 3) {
				eqTextPrinted = false;
				if (flashcardMounted) {
				secondaryDrive = true;
				chdir("fat:/");
				} else {
				secondaryDrive = false;
				chdir("sd:/");
				}
				xLock = 1;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 4) {
				eqTextPrinted = false;
				if (flashcardMounted) {
				secondaryDrive = true;
				chdir("fat:/");
				} else {
				secondaryDrive = false;
				chdir("sd:/");
				}
				yLock = 1;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 5) {
				eqTextPrinted = false;
				if (flashcardMounted) {
				secondaryDrive = true;
				chdir("fat:/");
				} else {
				secondaryDrive = false;
				chdir("sd:/");
				}
				errorLock = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 6) {
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 7) {
				eqTextPrinted = false;
				screenMode = 0;
				break;
			}
		}
	}
}