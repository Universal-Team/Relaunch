/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019
	Flame
	Epicpkmn11
	RocketRobz
------------------------------------------------------------------*/
#include "includes.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

char titleName[32] = {" "};

int screenMode = 0;

bool appInited = false;
bool arm7SCFGLocked = false;
bool isRegularDS = true;

static u16 bmpImageBuffer[256*192]; //for background

std::vector<DirEntry> ndsFiles;

using namespace std;

void setFontTop() {
	PrintConsole *console = consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);
	ConsoleFont font;
	font.gfx = (u16*)fontTiles;
	font.pal = (u16*)fontPal;
	font.numChars = 95;
	font.numColors =  fontPalLen / 2;
	font.bpp = 4;
	font.asciiOffset = 32;
	font.convertSingleColor = false;
	consoleSetFont(console, &font);
}
void setFontSub() {
	PrintConsole *console = consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 0, 15, false, true);
	ConsoleFont font;
	font.gfx = (u16*)fontTiles;
	font.pal = (u16*)fontPal;
	font.numChars = 95;
	font.numColors =  fontPalLen / 2;
	font.bpp = 4;
	font.asciiOffset = 32;
	font.convertSingleColor = false;
	consoleSetFont(console, &font);
}
//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;
	defaultExceptionHandler();

	std::string filename;


	// initialize video mode
	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	// initialize all the VRAM banks
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_TEX_PALETTE_SLOT5); // 16Kb of palette ram, and font textures take up 8*16 bytes.
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	//background before loading
	REG_BG3CNT = BG_MAP_BASE(1) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;

	REG_BG3CNT_SUB = BG_MAP_BASE(1) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;
	//done initing things for background

	setFontSub();
	fifoWaitValue32(FIFO_USER_06);
	if (fifoGetValue32(FIFO_USER_03) == 0) arm7SCFGLocked = true;
	u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	if (arm7_SNDEXCNT != 0) isRegularDS = false;	// If sound frequency setting is found, then the console is not a DS Phat/Lite
	fifoSendValue32(FIFO_USER_07, 0);

	sysSetCartOwner (BUS_OWNER_ARM9);	// Allow arm9 to access GBA ROM

	if (isDSiMode()) {
		sdMounted = sdMount();
	} else {
		flashcardMounted = flashcardMount();
	}

	keysSetRepeat(25,5);

	appInited = true;

	if(appInited) {
		if(sdMounted) {
		nitroFSInit("sd:/_nds/Relaunch/menu.bin");	
	} else {
		nitroFSInit("fat:/_nds/Relaunch/menu.bin");
	}

	FILE* fileBottom = fopen("nitro:/bg.bmp", "rb");

	if (fileBottom) {
		// Start loading
		fseek(fileBottom, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(fileBottom) + 0xe;
		fseek(fileBottom, pixelStart, SEEK_SET);
		fread(bmpImageBuffer, 2, 0x1A000, fileBottom);
		u16* src = bmpImageBuffer;
		int x = 0;
		int y = 191;
		for (int i=0; i<256*192; i++) {
			if (x >= 256) {
				x = 0;
				y--;
			}
			u16 val = *(src++);
			BG_GFX[(y+32)*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			BG_GFX_SUB[(y+32)*256+x] = ((val>>10)&0x1f) | ((val)&(0x1f<<5)) | (val&0x1f)<<10 | BIT(15);
			x++;
		}
	}
		//print bottom screen before top screen appears to (try and) prevent github issues about it freezing on launch :p
		printf ("\x1b[0;0H"); //this is line 1 (sometimes first is equal to 0)
		printf("\n\n No one\n   is\n illegal");
		setFontTop();
		printf (appVersion);
	}

	if (flashcardMounted) {
		secondaryDrive = true;
		chdir("fat:/");
	} else {
		secondaryDrive = false;
		chdir("sd:/");
	}

	findNdsFiles(ndsFiles);

	while(1) {

		if (screenMode == 0) {
			driveMenu(ndsFiles);
		} 
		if (screenMode == 1) {
			// filename = browseForFile();
		}
		if (screenMode == 2) {
			eqMenu(ndsFiles);
		}

		while (1) {
			swiWaitForVBlank();
			scanKeys();
			if (!(keysHeld() & KEY_A)) break;
		}
	}
	return 0;
}
