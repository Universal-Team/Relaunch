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
	ini.SetString("RELAUNCH", "BOOT_DOWN_PATH", bootDown);
	ini.SetString("RELAUNCH", "BOOT_UP_PATH", bootUp);
	ini.SetString("RELAUNCH", "BOOT_LEFT_PATH", bootLeft);
	ini.SetString("RELAUNCH", "BOOT_RIGHT_PATH", bootRight);
	ini.SetString("RELAUNCH", "BOOT_START_PATH", bootStart);
	ini.SetString("RELAUNCH", "BOOT_SELECT_PATH", bootSelect);
	ini.SetString("RELAUNCH", "BOOT_DEFAULT_PATH", bootDefault);

	ini.SaveIniFile("/_nds/Relaunch/Relaunch.ini");


printf("Menu Not Implemented Yet");
	}