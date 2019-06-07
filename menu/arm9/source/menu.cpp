/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019
	Evan "Flame" Rodgers
	Ben "Epicpkmn11" Bogie
	Rojelio "RocketRobz" Reyes
------------------------------------------------------------------*/

#include "includes.h"

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
static sNDSHeader nds;
bool sdMounted = false;
bool sdMountedDone = false;				// true if SD mount is successful once
bool flashcardMounted = false;
bool secondaryDrive = false;				// false == SD card, true == Flashcard
char sdLabel[12];
char fatLabel[12];
int sdSize = 0;
int fatSize = 0;

void dm_drawTopScreen(std::vector<DirEntry> ndsFiles) {

	if (fileMenu = true) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nRelaunch.nds v0.3");
	} else {
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
	}
	// Move to 4th row
	printf ("\x1b[3;0H");

	for (int i = dmCursorPosition+0; i <= dmCursorPosition+18; i++) {
		iprintf ("\x1b[%d;0H", i + ENTRIES_START_ROW);
		if (dmCursorPosition == i) {
			//printf ("\x1b[46m# ");		// Print foreground cyan color
			printf("# ");
		} else {
			//printf ("\x1b[42m  ");		// Print foreground green color
			printf("  ");
		}
		printf((ndsFiles[i].name + "\n").c_str());
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
	} else if (dmAssignedOp[dmCursorPosition] > 3) {
		printf ("\n\n\n\n\n\nPUB SIZE: 00000000");
		printf ("\nPRV SIZE: 00000000\n");
		printf ("nds app");
	/*char fullPath[256];
	snprintf(fullPath, sizeof(fullPath), "%s%s", path, entry->name.c_str());*/
		//printf (fullPath);
	}
}

void driveMenu (void) {
	fileMenu = true;
	int pressed = 0;
	
if (flashcardMounted) {
	secondaryDrive = true;
	chdir("fat:/");
} else {
	secondaryDrive = false;
	chdir("sd:/");
}
	std::vector<DirEntry> ndsFiles;
	findNdsFiles(ndsFiles);

	if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
	|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
		DirEntry options;
		options.name = "OPTIONS";
		ndsFiles.insert(ndsFiles.begin(), options);
	}
	if (access("sd:/_nds/Relaunch/WIFIBOOT.NDS", F_OK) == 0 
	|| access("fat:/_nds/Relaunch/WIFIBOOT.NDS", F_OK) == 0) {
		DirEntry wifiboot;
		wifiboot.name = "WIFIBOOT";
		ndsFiles.insert(ndsFiles.begin(), wifiboot);
	}
	if (access("fat:/_nds/TWiLightMenu/gbaswitch.srldr", F_OK) == 0 && isRegularDS) {
		DirEntry gbaGame;
		gbaGame.name = "GBA GAME";
		ndsFiles.insert(ndsFiles.begin(), gbaGame);
	}
	if (access("sd:/_nds/TWiLightMenu/slot1launch.srldr", F_OK) == 0 
	|| access("fat:/_nds/TWiLightMenu/slot1launch.srldr", F_OK) == 0) {
		DirEntry dsGame;
		dsGame.name = "DS GAME";
		ndsFiles.insert(ndsFiles.begin(), dsGame);
	}

while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A));

		if ((pressed & KEY_UP) && dmCursorPosition > 0) {
			dmCursorPosition -= 1;
			dmTextPrinted = false;
		}
		if ((pressed & KEY_DOWN) && dmCursorPosition < ndsFiles.size()) {
			dmCursorPosition += 1;
			dmTextPrinted = false;
		}

		if (pressed & KEY_A) {
			if (dmCursorPosition > 3) {
			applaunch = true;
			}
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

void findNdsFiles(vector<DirEntry>& dirContents) {
	struct stat st;
	DIR *pdir = opendir(".");

	if (pdir == NULL) {
		iprintf("Unable to open the directory, Please report to https://github.com/FlameKat53/Relaunch/issues");
		for(int i=0;i<120;i++)
			swiWaitForVBlank();
	} else {
		while (true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if (pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;
			if(!(dirEntry.isDirectory) && dirEntry.name.length() >= 3) {
				if (strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "nds") == 0)
				|| (strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "dsi") == 0)
				|| (strcasecmp(dirEntry.name.substr(dirEntry.name.length()-3, 3).c_str(), "app") == 0) {
					dirContents.push_back(dirEntry);
				}
			} else if (dirEntry.isDirectory && dirEntry.name.compare(".") != 0 && dirEntry.name.compare("..") != 0) {
				chdir(dirEntry.name.c_str());
				findNdsFiles(dirContents);
				chdir("..");
			}
		}
		closedir(pdir);
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
	int screenOffset = 0;
	int fileOffset = 0;
	vector<DirEntry> dirContents;

	getDirectoryContents (dirContents);

	while (true) {
		DirEntry* entry = &dirContents.at(fileOffset);

		setFontSub();
		fileBrowse_drawBottomScreen(entry, fileOffset);
		setFontTop();
		dm_drawTopScreen(ndsFiles);

while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(pressed & KEY_B));

		iprintf ("\x1b[%d;0H", fileOffset - screenOffset + ENTRIES_START_ROW);

		if (pressed & KEY_UP && fileOffset > 0) {	fileOffset -= 1;}
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
				// Enter the directory
				chdir (entry->name.c_str());
				getDirectoryContents(dirContents);
				screenOffset = 0;
				fileOffset = 0;
			} else if (entry->isApp) {
	setFontSub();
	printf ("\x1b[0;27H");
	char fullPath[256];
	snprintf(fullPath, sizeof(fullPath), "%s%s", path, entry->name.c_str());

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

// OPTIONS MENU THINGS BELOW
// OPTIONS MENU THINGS BELOW

void eq_drawTopScreen(void) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nCHANGE BOOT DEFAULT/HOTKEYS:");

	// Move to 4th row
	printf ("\x1b[3;0H");

	if (eqMaxCursors == -1) {
		printf ("This Shouldn't Happen! Please report to https://github.com/FlameKat53/Relaunch/issues");
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
			printf ("BUTTON L");
		} else if (eqAssignedOp[i] == 6) {
			printf ("BUTTON R");
		} else if (eqAssignedOp[i] == 7) {
			printf ("BUTTON START");
		} else if (eqAssignedOp[i] == 8) {
			printf ("BUTTON SELECT");
		} else if (eqAssignedOp[i] == 9) {
			printf ("TOUCH SCREEN");
		} else if (eqAssignedOp[i] == 10) {
			printf ("D-PAD UP");
		} else if (eqAssignedOp[i] == 11) {
			printf ("D-PAD DOWN");
		} else if (eqAssignedOp[i] == 12) {
			printf ("D-PAD LEFT");
		} else if (eqAssignedOp[i] == 13) {
			printf ("D-PAD RIGHT");
		} else if (eqAssignedOp[i] == 14) {
			printf ("LOAD ERROR");
		} else if (eqAssignedOp[i] == 15) {
			printf ("BUTTON A+B (FILEMENU - FIXED)");
		} else if (eqAssignedOp[i] == 16) {
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
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
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
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 8;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 9;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 10;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 11;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 12;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 13;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 14;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 15;
		}
		if (access("sd:/_nds/Relaunch/menu.bin", F_OK) == 0 
		|| access("fat:/_nds/Relaunch/menu.bin", F_OK) == 0) {
			eqMaxCursors++;
			eqAssignedOp[eqMaxCursors] = 16;
		}

		if (!eqTextPrinted) {
		setFontSub();
			eq_drawBottomScreen();
		setFontTop();
			eq_drawTopScreen();

			eqTextPrinted = true;
		}

while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(pressed & KEY_B));

		if ((pressed & KEY_UP) && eqMaxCursors != -1 && eqCursorPosition != 0) {
			eqCursorPosition -= 1;
			eqTextPrinted = false;
		}
		if ((pressed & KEY_DOWN) && eqMaxCursors != -1 && eqCursorPosition != 16) {
			eqCursorPosition += 1;
			eqTextPrinted = false;
		}

		if (pressed & KEY_B) {
		screenMode = 0;
		eqTextPrinted = false;
		break;
		}

		if (pressed & KEY_A) {
			eqTextPrinted = false;
		if (flashcardMounted) {
			secondaryDrive = true;
			chdir("fat:/");
		} else {
			secondaryDrive = false;
			chdir("sd:/");
		}
			if (eqAssignedOp[eqCursorPosition] == 0) {
				noLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 1) {
				aLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 2) {
				bLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 3) {
				xLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 4) {
				yLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 5) {
				lLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 6) {
				rLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 7) {
				startLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 8) {
				selectLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 9) {
				touchLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 10) {
				upLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 11) {
				downLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 12) {
				leftLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 13) {
				rightLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 14) {
				errorLock = true;
				screenMode = 1;
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 15) {
				break;
			} else if (eqAssignedOp[eqCursorPosition] == 16) {
				eqTextPrinted = false;
				//ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");
				screenMode = 0;
				break;
			}
		}
	}
}

// file operation things
// file operation things

void fixLabel(bool fat) {
	if (fat) {
		for (int i = 0; i < 12; i++) {
			if (((fatLabel[i] == ' ') && (fatLabel[i+1] == ' ') && (fatLabel[i+2] == ' '))
			|| ((fatLabel[i] == ' ') && (fatLabel[i+1] == ' '))
			|| (fatLabel[i] == ' ')) {
				fatLabel[i] = '\0';
				break;
			}
		}
	} else {
		for (int i = 0; i < 12; i++) {
			if (((sdLabel[i] == ' ') && (sdLabel[i+1] == ' ') && (sdLabel[i+2] == ' '))
			|| ((sdLabel[i] == ' ') && (sdLabel[i+1] == ' '))
			|| (sdLabel[i] == ' ')) {
				sdLabel[i] = '\0';
				break;
			}
		}
	}
}

bool sdFound(void) {
	if (access("sd:/", F_OK) == 0) {
		return true;
	} else {
		return false;
	}
}

bool flashcardFound(void) {
	if (access("fat:/", F_OK) == 0) {
		return true;
	} else {
		return false;
	}
}

TWL_CODE bool sdMount(void) {
	fatMountSimple("sd", get_io_dsisd());
	if (sdFound()) {
		sdMountedDone = true;
		fatGetVolumeLabel("sd", sdLabel);
		fixLabel(false);
		return true;
	}
	return false;
}

bool flashcardMount(void) {
	if ((!isDSiMode()) || (arm7SCFGLocked && !sdMountedDone)) {
		fatInitDefault();
		if (flashcardFound()) {
			fatGetVolumeLabel("fat", fatLabel);
			fixLabel(true);
			return true;
		}
		return false;
	}
}

off_t getFileSize(const char *fileName)
{
    FILE* fp = fopen(fileName, "rb");
    off_t fsize = 0;
    if (fp) {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);			// Get source file's size
		fseek(fp, 0, SEEK_SET);
	}
	fclose(fp);

	return fsize;
}
