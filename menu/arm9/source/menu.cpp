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

#define SCREEN_COLS 30
#define ENTRIES_PER_SCREEN 19
#define ENTRIES_START_ROW 3
#define ENTRIES_START_ROW_EQ 3
#define ENTRY_PAGE_LENGTH 10
using namespace std;
static char path[PATH_MAX];

static bool dmTextPrinted = false;
static int dmCursorPosition = 0;
static int dmAssignedOp[3] = {-1};
static int dmMaxCursors = -1;
static bool eqTextPrinted = false;
static int eqCursorPosition = 0;
static int eqAssignedOp[3] = {-1};
static int eqMaxCursors = -1;
static bool fileMenu = false;
static bool noLock = false;
static bool aLock = false;
static bool bLock = false;
static bool xLock = false;
static bool yLock = false;
static bool lLock = false;
static bool rLock = false;
static bool startLock = false;
static bool selectLock = false;
static bool upLock = false;
static bool downLock = false;
static bool leftLock = false;
static bool rightLock = false;
static bool touchLock = false;
static bool errorLock = false;

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
	runNdsFile("fat:/_nds/TWiLightMenu/gbaswitch.srldr", 0, NULL, false);
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
	fileMenu = true;
	int pressed = 0;
	int held = 0;

	while (true) {
		for (int i = 0; i < 3; i++) {
			dmAssignedOp[i] = -1;
		}
		dmMaxCursors = -1;
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 0;
		}
		if (access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0 && isRegularDS) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 1;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 2;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 3;
		}

		if (!dmTextPrinted) {
		setFontSub();
			dm_drawBottomScreen();
		setFontTop();
			dm_drawTopScreen();
			//browseForFile2();

			dmTextPrinted = true;
		}

		stored_SCFG_MC = REG_SCFG_MC;

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
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A));

		if ((pressed & KEY_UP) && dmMaxCursors != -1 && dmCursorPosition != 0) {
			dmCursorPosition -= 1;
			dmTextPrinted = false;
		}
		if ((pressed & KEY_DOWN) && dmMaxCursors != -1 && dmCursorPosition != 3) {
			dmCursorPosition += 1;
			dmTextPrinted = false;
		}

		if (pressed & KEY_A) {
			if (dmAssignedOp[dmCursorPosition] == 0) {
				dmTextPrinted = false;
				if (flashcardMounted) {
				runNdsFile("fat:/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, false);
				} else {
				runNdsFile("sd:/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, false);
				}
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 1 && isRegularDS) {
				dmTextPrinted = false;
				loadGbaCart();
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 2) {
				dmTextPrinted = false;
				if (flashcardMounted) {
				runNdsFile("fat:/_nds/Relaunch/WIFIBOOT.NDS", 0, NULL, false);
				} else {
				runNdsFile("sd:/_nds/Relaunch/WIFIBOOT.NDS", 0, NULL, false);
				}
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 3) {
				dmTextPrinted = false;
				fileMenu = false;
				screenMode = 2;
				break;
			}
		}
	}
}

// file browse stuff below!
// file browse stuff below! (yes this is on 2 lines)

bool dirEntryPredicate (const DirEntry& lhs, const DirEntry& rhs) {

	if (!lhs.isDirectory && rhs.isDirectory) {
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory) {
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

bool nameEndsWith (const string& name) {

	if (name.size() == 0) return false;

	return true;
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
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "APP")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "bin")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "srldr")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "dat")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "DAT"))
				{
					dirEntry.isApp = true;
				} else {
					dirEntry.isApp = false;
				}

				if (dirEntry.name.compare(".") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name))) {
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

	// Clear the screen
	iprintf ("\x1b[2J");

	// Print the path
	printf ("\x1b[1;0H");
	if (noLock == true) { printf("Select title for NO BUTTON"); } else {}
	if (aLock == true) { printf("Select title for BUTTON A"); } else {}
	if (bLock == true) { printf("Select title for BUTTON B"); } else {}
	if (xLock == true) { printf("Select title for BUTTON X"); } else {}
	if (yLock == true) { printf("Select title for BUTTON Y"); } else {}
	if (lLock == true) { printf("Select title for BUTTON L"); } else {}
	if (rLock == true) { printf("Select title for BUTTON R"); } else {}
	if (startLock == true) { printf("Select title for BUTTON START"); } else {}
	if (selectLock == true) { printf("Select title for BUTTON SELECT"); } else {}
	if (touchLock == true) { printf("Select title for TOUCH SCREEN"); } else {}
	if (upLock == true) { printf("Select title for DPAD UP"); } else {}
	if (downLock == true) { printf("Select title for DPAD DOWN"); } else {}
	if (leftLock == true) { printf("Select title for DPAD LEFT"); } else {}
	if (rightLock == true) { printf("Select title for DPAD RIGHT"); } else {}
	if (errorLock == true) { printf("Select title for LOAD ERROR"); } else {}

	// Move to 2nd row
	iprintf ("\x1b[1;0H");

	// Print directory listing
	for (int i = 0; i < ((int)dirContents.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		char entryName[SCREEN_COLS + 1];

		// Set row
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if ((fileOffset - startRow) == i) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}

		strncpy (entryName, entry->name.c_str(), SCREEN_COLS);
		entryName[SCREEN_COLS] = '\0';
		printf (entryName);
		if (entry->isDirectory) {
			printf ("\x1b[%d;27H", i + ENTRIES_START_ROW);
			printf ("(dir)");
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

		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(pressed & KEY_B));

		iprintf ("\x1b[%d;0H", fileOffset - screenOffset + ENTRIES_START_ROW);

		if (pressed & KEY_UP && fileOffset > 0) {		fileOffset -= 1;}
		if (pressed & KEY_DOWN && fileOffset < (int)dirContents.size() - 1) {	fileOffset += 1;}

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
				iprintf("  Please Wait...\n");
				// Enter selected directory
				chdir (entry->name.c_str());
				getDirectoryContents(dirContents);
				screenOffset = 0;
				fileOffset = 0;
			} else if (entry->isApp) {
	setFontSub();
	printf ("\x1b[0;27H");
	char fullPath[256];
	snprintf(fullPath, sizeof(fullPath), "%s%s", path, entry->name.c_str());

		if (fileMenu == true) {
				applaunch = true;
		} else {

	CIniFile ini("/_nds/Relaunch/Relaunch.ini");

	if (noLock == true) { ini.SetString("RELAUNCH", "BOOT_DEFAULT_PATH", fullPath); } else {}
	if (aLock == true) { ini.SetString("RELAUNCH", "BOOT_A_PATH", fullPath); } else {}
	if (bLock == true) { ini.SetString("RELAUNCH", "BOOT_B_PATH", fullPath); } else {}
	if (xLock == true) { ini.SetString("RELAUNCH", "BOOT_X_PATH", fullPath); } else {}
	if (yLock == true) { ini.SetString("RELAUNCH", "BOOT_Y_PATH", fullPath); } else {}
	if (lLock == true) { ini.SetString("RELAUNCH", "BOOT_L_PATH", fullPath); } else {}
	if (rLock == true) { ini.SetString("RELAUNCH", "BOOT_R_PATH", fullPath); } else {}
	if (startLock == true) { ini.SetString("RELAUNCH", "BOOT_START_PATH", fullPath); } else {}
	if (selectLock == true) { ini.SetString("RELAUNCH", "BOOT_SELECT_PATH", fullPath); } else {}
	if (touchLock == true) { ini.SetString("RELAUNCH", "BOOT_TOUCH_PATH", fullPath); } else {}
	if (upLock == true) { ini.SetString("RELAUNCH", "BOOT_UP_PATH", fullPath); } else {}
	if (downLock == true) { ini.SetString("RELAUNCH", "BOOT_DOWN_PATH", fullPath); } else {}
	if (leftLock == true) { ini.SetString("RELAUNCH", "BOOT_LEFT_PATH", fullPath); } else {}
	if (rightLock == true) { ini.SetString("RELAUNCH", "BOOT_RIGHT_PATH", fullPath); } else {}
	if (errorLock == true) { ini.SetString("RELAUNCH", "LOAD_ERROR", fullPath); } else {}

	ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");

		screenMode = 2;
		return "null";
}
				if (fileMenu == true) {
					// Return the chosen file
					entry->name;
				}
			}
		}
		if (pressed & KEY_B) {
			if ((strcmp (path, "sd:/") == 0) || (strcmp (path, "fat:/") == 0)) {
				screenMode = 2;
				return "null";
			}
			// Go up a directory
			chdir ("..");
			getDirectoryContents (dirContents);
			screenOffset = 0;
			fileOffset = 0;
		}
	}
}

string browseForFile2 (void) {
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

		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(pressed & KEY_B));

		iprintf ("\x1b[%d;0H", fileOffset - screenOffset + ENTRIES_START_ROW);

		if (pressed & KEY_UP && fileOffset > 0) {		fileOffset -= 1;}
		if (pressed & KEY_DOWN && fileOffset < (int)dirContents.size() - 1) {	fileOffset += 1;}

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
				iprintf("  Please Wait...\n");
				// Enter selected directory
				chdir (entry->name.c_str());
				getDirectoryContents(dirContents);
				screenOffset = 0;
				fileOffset = 0;
			} else if (entry->isApp) {
	setFontSub();
	printf ("\x1b[0;27H");
	char fullPath[256];
	snprintf(fullPath, sizeof(fullPath), "%s%s", path, entry->name.c_str());

		if (fileMenu == true) {
				applaunch = true;
		} else {

	CIniFile ini("/_nds/Relaunch/Relaunch.ini");

	if (aLock == true) { ini.SetString("RELAUNCH", "BOOT_A_PATH", fullPath); } else {}
	if (noLock == true) { ini.SetString("RELAUNCH", "BOOT_DEFAULT_PATH", fullPath); } else {}
	if (bLock == true) { ini.SetString("RELAUNCH", "BOOT_B_PATH", fullPath); } else {}
	if (xLock == true) { ini.SetString("RELAUNCH", "BOOT_X_PATH", fullPath); } else {}
	if (yLock == true) { ini.SetString("RELAUNCH", "BOOT_Y_PATH", fullPath); } else {}
	if (lLock == true) { ini.SetString("RELAUNCH", "BOOT_L_PATH", fullPath); } else {}
	if (rLock == true) { ini.SetString("RELAUNCH", "BOOT_R_PATH", fullPath); } else {}
	if (startLock == true) { ini.SetString("RELAUNCH", "BOOT_START_PATH", fullPath); } else {}
	if (selectLock == true) { ini.SetString("RELAUNCH", "BOOT_SELECT_PATH", fullPath); } else {}
	if (touchLock == true) { ini.SetString("RELAUNCH", "BOOT_TOUCH_PATH", fullPath); } else {}
	if (upLock == true) { ini.SetString("RELAUNCH", "BOOT_UP_PATH", fullPath); } else {}
	if (downLock == true) { ini.SetString("RELAUNCH", "BOOT_DOWN_PATH", fullPath); } else {}
	if (leftLock == true) { ini.SetString("RELAUNCH", "BOOT_LEFT_PATH", fullPath); } else {}
	if (errorLock == true) { ini.SetString("RELAUNCH", "LOAD_ERROR", fullPath); } else {}
	if (rightLock == true) { ini.SetString("RELAUNCH", "BOOT_RIGHT_PATH", fullPath); } else {}

	ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");

		screenMode = 2;
		return "null";
}
				if (fileMenu == true) {
					// Return the chosen file
					entry->name;
				}
			}
		}
		if (pressed & KEY_B) {
			if ((strcmp (path, "sd:/") == 0) || (strcmp (path, "fat:/") == 0)) {
				screenMode = 2;
				return "null";
			}
			// Go up a directory
			chdir ("..");
			getDirectoryContents (dirContents);
			screenOffset = 0;
			fileOffset = 0;
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
	noLock = false;
	aLock = false;
	bLock = false;
	xLock = false;
	yLock = false;
	lLock = false;
	rLock = false;
	upLock = false;
	downLock = false;
	leftLock = false;
	rightLock = false;
	startLock = false;
	selectLock = false;
	touchLock = false;
	errorLock = false;
	eqTextPrinted = false;

	while (true) {
		for (int i = 0; i < 3; i++) {
			eqAssignedOp[i] = -1;
		}
		eqMaxCursors = -1;
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0){
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 0;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 1;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 2;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 3;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 4;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 5;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 6;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 7;
		}

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

		if ((pressed & KEY_UP) && eqMaxCursors != -1 && eqCursorPosition != 0) {
			eqCursorPosition -= 1;
			eqTextPrinted = false;
		}
		if ((pressed & KEY_DOWN) && eqMaxCursors != -1 && eqCursorPosition != 7) {
			eqCursorPosition += 1;
			eqTextPrinted = false;
		}
		if (pressed & KEY_B) {
		screenMode = 0;
		eqTextPrinted = false;
		break;
		}

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
				noLock = true;
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
				aLock = true;
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
				bLock = true;
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
				xLock = true;
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
				yLock = true;
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
				errorLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 6) {
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 7) {
				eqTextPrinted = false;
				/*CIniFile ini("/_nds/Relaunch/Relaunch.ini");
				ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");*/
				screenMode = 0;
				break;
			}
		}
	}
}
