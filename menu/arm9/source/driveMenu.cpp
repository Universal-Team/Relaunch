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

#include <nds.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <dswifi9.h>
#include <netinet/in.h>

#include "main.h"
#include "Operations.h"
#include "nds_loader_arm9.h"
#include "font.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 22
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10
using namespace std;

bool flashcardMountSkipped = true;
static bool flashcardMountRan = true;
static bool dmTextPrinted = false;
static int dmCursorPosition = 0;
static int dmAssignedOp[3] = {-1};
static int dmMaxCursors = -1;
static u8 gbaFixedValue = 0; 

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
	runNdsFile("_nds/Relaunch/gba.bin", 0, NULL, false);
	}

void wifiTest(void) {
	wifiInit(); //start wifi initiation
// start FTP server
	printf("Error!");
	}

void dm_drawTopScreen(void) {
	//printf ("\x1b[43m"); //yellow
	printf ("\x1b[0;0H");
	printf ("\nRelaunch.nds v0.2");

	// Move to 4th row
	printf ("\x1b[3;0H");

	if (dmMaxCursors == -1) {
		printf ("No drives found!");
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
			printf ("[sd:]");
			if (sdLabel[0] != '\0') {
				iprintf (" %s", sdLabel);
			}
		} else if (dmAssignedOp[i] == 1) {
			printf ("[fat:]");
			if (fatLabel[0] != '\0') {
				iprintf (" %s", fatLabel);
			}
		} else if (dmAssignedOp[i] == 2) {
			printf ("Launch Slot-2 Cart");
			if (gbaFixedValue != 0x96) {
				iprintf ("\x1b[%d;29H", i + ENTRIES_START_ROW);
				printf ("[x]");
			}
		} else if (dmAssignedOp[i] == 3) {
			printf ("WifiFTP");
		}
	}
}
			    
void dm_drawBottomScreen(void) {
	//printf ("\x1B[47m");		// Print foreground white color
	printf ("\x1b[23;0H");
	printf (titleName);

	//printf ("\x1b[43m");		// Print background yellow color
	printf ("\x1b[0;0H");
	printf("\n\n Everyone\n   is\n  legal");
	//printf("\x1B[47m");
	printf("\x1b[0;1H");
	if (dmAssignedOp[dmCursorPosition] == 0) {
		printf ("[sd:] SDCARD");
		if (sdLabel[0] != '\0') {
			iprintf (" (%s)", sdLabel);
		}
		printf ("\n(SD FAT)");
	} else if (dmAssignedOp[dmCursorPosition] == 1) {
		printf ("\n\n\n\n\n\n[fat:] FLASHCART");
		if (fatLabel[0] != '\0') {
			iprintf (" \n(%s)", fatLabel);
		}
		printf ("\n(Slot-1 SD FAT)");
	} else if (dmAssignedOp[dmCursorPosition] == 2) {
		printf ("\n\n\n\n\n\nLaunch Slot-2 Cart\n");
		printf ("\n(GBA Game)");
	} else if (dmAssignedOp[dmCursorPosition] == 3) {
		printf ("\n\n\n\n\n\nWifiFTP\n");
		printf ("\n(Wireless FTP Server)");
	}
}

void driveMenu (void) {
	int pressed = 0;
	int held = 0;

	while (true) {
		if (!isDSiMode() && isRegularDS) {
			gbaFixedValue = *(u8*)(0x080000B2);
		}

		for (int i = 0; i < 3; i++) {
			dmAssignedOp[i] = -1;
		}
		dmMaxCursors = -1;
		if (isDSiMode() && sdMounted){
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 0;
		}
		if (flashcardMounted) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 1;
		}
		if (!isDSiMode() && isRegularDS) {
			dmMaxCursors++;
			dmAssignedOp[dmMaxCursors] = 2;
		}
		if (!isDSiMode() && isRegularDS) {
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

			dmTextPrinted = true;
		}

		stored_SCFG_MC = REG_SCFG_MC;

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
	
			scanKeys();
			pressed = keysDownRepeat();
			held = keysHeld();
			swiWaitForVBlank();

			if (!isDSiMode() && isRegularDS) {
				if (*(u8*)(0x080000B2) != gbaFixedValue) {
					dmTextPrinted = false;
					break;
				}
			} else if (isDSiMode()) {
				if (REG_SCFG_MC != stored_SCFG_MC) {
					dmTextPrinted = false;
					break;
				}
			}
		} while (!(pressed & KEY_UP) && !(pressed & KEY_DOWN) && !(pressed & KEY_A) && !(held & KEY_R));
	
		printf ("\x1B[47m");		// Print foreground white color

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
			if (dmAssignedOp[dmCursorPosition] == 0 && isDSiMode() && sdMounted) {
				dmTextPrinted = false;
				secondaryDrive = false;
				chdir("sd:/");
				screenMode = 1;
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 1 && flashcardMounted) {
				dmTextPrinted = false;
				secondaryDrive = true;
				chdir("fat:/");
				screenMode = 1;
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 2 && isRegularDS && flashcardMounted && gbaFixedValue == 0x96) {
				dmTextPrinted = false;
				loadGbaCart();
				break;
			} else if (dmAssignedOp[dmCursorPosition] == 3) {
				dmTextPrinted = false;
				wifiTest();
				break;
				}
			}
		}

		if (isDSiMode() && !flashcardMountSkipped && !pressed && !held) {
			if (REG_SCFG_MC == 0x11) {
				if (flashcardMounted) {
					flashcardUnmount();
				}
			} else if (!flashcardMountRan) {
				flashcardMounted = flashcardMount();	// Try to mount flashcard
			}
			flashcardMountRan = false;
		}
	}
