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
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fat.h>
#include "common/nds_loader_arm9.h"
#include "common/inifile.h"
//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	std::string bootA = "/_nds/Relaunch/extras/bootA.nds";
	std::string bootB = "/_nds/Relaunch/extras/bootB.nds";
	std::string bootX = "/_nds/Relaunch/extras/bootX.nds";
	std::string bootY = "/_nds/Relaunch/extras/bootY.nds";
	std::string bootR = "/_nds/Relaunch/extras/bootR.nds";
	std::string bootL = "/_nds/Relaunch/extras/bootL.nds";
	std::string bootDown = "/_nds/Relaunch/extras/bootDown.nds";
	std::string bootUp = "/_nds/Relaunch/extras/bootUp.nds";
	std::string bootLeft = "/_nds/Relaunch/extras/bootLeft.nds";
	std::string bootRight = "/_nds/Relaunch/extras/bootRight.nds";
	std::string bootStart = "/_nds/Relaunch/extras/bootStart.nds";
	std::string bootSelect = "/_nds/Relaunch/extras/bootSelect.nds";
	std::string bootTouch = "/_nds/Relaunch/extras/bootTouch.nds";
	std::string bootDefault = "/boot.nds";

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);
	vramSetBankH(VRAM_H_SUB_BG);
	consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);

	if (!fatInitDefault()) {
		iprintf ("fatInitDefault failed!\n");
		stop();
	}
	CIniFile ini("/_nds/Relaunch/Relaunch.ini");
	
	bootA = ini.GetString("RELAUNCH", "BOOT_A_PATH", bootA);
	bootB = ini.GetString("RELAUNCH", "BOOT_B_PATH", bootB);
	bootX = ini.GetString("RELAUNCH", "BOOT_X_PATH", bootX);
	bootY = ini.GetString("RELAUNCH", "BOOT_Y_PATH", bootY);
	bootR = ini.GetString("RELAUNCH", "BOOT_R_PATH", bootR);
	bootL = ini.GetString("RELAUNCH", "BOOT_L_PATH", bootL);
	bootDown = ini.GetString("RELAUNCH", "BOOT_DOWN_PATH", bootDown);
	bootUp = ini.GetString("RELAUNCH", "BOOT_UP_PATH", bootUp);
	bootLeft = ini.GetString("RELAUNCH", "BOOT_LEFT_PATH", bootLeft);
	bootRight = ini.GetString("RELAUNCH", "BOOT_RIGHT_PATH", bootRight);
	bootStart = ini.GetString("RELAUNCH", "BOOT_START_PATH", bootStart);
	bootSelect = ini.GetString("RELAUNCH", "BOOT_SELECT_PATH", bootSelect);
	bootTouch = ini.GetString("RELAUNCH", "BOOT_TOUCH_PATH", bootTouch);
	bootDefault = ini.GetString("RELAUNCH", "BOOT_DEFAULT_PATH", bootDefault);

	ini.SetString("RELAUNCH", "BOOT_A_PATH", bootA);
	ini.SetString("RELAUNCH", "BOOT_B_PATH", bootB);
	ini.SetString("RELAUNCH", "BOOT_X_PATH", bootX);
	ini.SetString("RELAUNCH", "BOOT_Y_PATH", bootY);
	ini.SetString("RELAUNCH", "BOOT_R_PATH", bootR);
	ini.SetString("RELAUNCH", "BOOT_L_PATH", bootL);
	ini.SetString("RELAUNCH", "BOOT_DOWN_PATH", bootDown);
	ini.SetString("RELAUNCH", "BOOT_UP_PATH", bootUp);
	ini.SetString("RELAUNCH", "BOOT_LEFT_PATH", bootLeft);
	ini.SetString("RELAUNCH", "BOOT_RIGHT_PATH", bootRight);
	ini.SetString("RELAUNCH", "BOOT_START_PATH", bootStart);
	ini.SetString("RELAUNCH", "BOOT_SELECT_PATH", bootSelect);
	ini.SetString("RELAUNCH", "BOOT_DEFAULT_PATH", bootDefault);

	mkdir("/_nds/",0777);
	mkdir("/_nds/Relaunch/",0777);
	mkdir("/_nds/Relaunch/extras",0777);
	ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");

  scanKeys();
	int pressed = keysHeld();

	if ((pressed & (KEY_A | KEY_B)) == (KEY_A | KEY_B)) { // menu
		if((access("_nds/Relaunch/menu.bin", F_OK) == 0)) {
			runNdsFile("_nds/Relaunch/menu.bin", 0, NULL, false);
		} else {
			printf("Error:\nmenu.bin wasn't found!");
			stop();
		}
	} else if (pressed & KEY_A) {
		if((access(bootA.c_str(), F_OK) == 0)) {
			runNdsFile(bootA.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootA.c_str());
			stop();
		}
	} else if (pressed & KEY_B) {
		if((access(bootB.c_str(), F_OK) == 0)) {
			runNdsFile(bootB.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootB.c_str());
			stop();
		}
	} else if (pressed & KEY_X) {
		if((access(bootX.c_str(), F_OK) == 0)) {
			runNdsFile(bootX.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootX.c_str());
			stop();
		}
	} else if (pressed & KEY_Y) {
		if((access(bootY.c_str(), F_OK) == 0)) {
			runNdsFile(bootY.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootY.c_str());
			stop();
		}
	} else if (pressed & KEY_R) {
		if((access(bootR.c_str(), F_OK) == 0)) {
			runNdsFile(bootR.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootR.c_str());
			stop();
		}
	} else if (pressed & KEY_L) {
		if((access(bootL.c_str(), F_OK) == 0)) {
			runNdsFile(bootL.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootL.c_str());
			stop();
		}
	} else if (pressed & KEY_RIGHT) {
		if((access(bootRight.c_str(), F_OK) == 0)) {
			runNdsFile(bootRight.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootRight.c_str());
			stop();
		}
	} else if (pressed & KEY_LEFT) {
		if((access(bootLeft.c_str(), F_OK) == 0)) {
			runNdsFile(bootLeft.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootLeft.c_str());
			stop();
		}
	} else if (pressed & KEY_DOWN) {
		if((access(bootDown.c_str(), F_OK) == 0)) {
			runNdsFile(bootDown.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootDown.c_str());
			stop();
		}
	} else if (pressed & KEY_UP) {
		if((access(bootUp.c_str(), F_OK) == 0)) {
			runNdsFile(bootUp.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootUp.c_str());
			stop();
		}
	} else if (pressed & KEY_START) {
		if((access(bootStart.c_str(), F_OK) == 0)) {
			runNdsFile(bootStart.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootStart.c_str());
			stop();
		}
	} else if (pressed & KEY_SELECT) {
		if((access(bootSelect.c_str(), F_OK) == 0)) {
			runNdsFile(bootSelect.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootSelect.c_str());
			stop();
		}
	} else if (pressed & KEY_TOUCH) {
		if((access(bootTouch.c_str(), F_OK) == 0)) {
			runNdsFile(bootTouch.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootTouch.c_str());
			stop();
		}
	} else {
		if((access(bootDefault.c_str(), F_OK) == 0)) {
			runNdsFile(bootDefault.c_str(), 0, NULL, false);
		} else {
			printf("Error:\n%s \nwasn't found!", bootDefault.c_str());
			stop();
		}
	}
}
