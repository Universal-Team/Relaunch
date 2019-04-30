#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdio.h>

#include "main.h"

static sNDSHeader nds;

u8 stored_SCFG_MC = 0;

static bool slot1Enabled = true;

bool sdMounted = false;
bool sdMountedDone = false;				// true if SD mount is successful once
bool flashcardMounted = false;

bool secondaryDrive = false;				// false == SD card, true == Flashcard

char sdLabel[12];
char fatLabel[12];

int sdSize = 0;
int fatSize = 0;

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

bool bothSDandFlashcard(void) {
	if (sdMounted && flashcardMounted) {
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

TWL_CODE void sdUnmount(void) {
	fatUnmount("sd");
	sdLabel[0] = '\0';
	sdSize = 0;
	sdMounted = false;
}

TWL_CODE DLDI_INTERFACE* dldiLoadFromBin (const u8 dldiAddr[]) {
	// Check that it is a valid DLDI
	if (!dldiIsValid ((DLDI_INTERFACE*)dldiAddr)) {
		return NULL;
	}

	DLDI_INTERFACE* device = (DLDI_INTERFACE*)dldiAddr;
	size_t dldiSize;

	// Calculate actual size of DLDI
	// Although the file may only go to the dldiEnd, the BSS section can extend past that
	if (device->dldiEnd > device->bssEnd) {
		dldiSize = (char*)device->dldiEnd - (char*)device->dldiStart;
	} else {
		dldiSize = (char*)device->bssEnd - (char*)device->dldiStart;
	}
	dldiSize = (dldiSize + 0x03) & ~0x03; 		// Round up to nearest integer multiple
	
	// Clear unused space
	memset(device+dldiSize, 0, 0x4000-dldiSize);

	dldiFixDriverAddresses (device);

	if (device->ioInterface.features & FEATURE_SLOT_GBA) {
		sysSetCartOwner(BUS_OWNER_ARM9);
	}
	if (device->ioInterface.features & FEATURE_SLOT_NDS) {
		sysSetCardOwner(BUS_OWNER_ARM9);
	}
	
	return device;
}

TWL_CODE bool UpdateCardInfo(char* gameid, char* gamename) {
	cardReadHeader((uint8*)0x02000000);
	memcpy(&nds, (void*)0x02000000, sizeof(sNDSHeader));
	memcpy(gameid, &nds.gameCode, 4);
	gameid[4] = 0x00;
	memcpy(gamename, &nds.gameTitle, 12);
	gamename[12] = 0x00;
	return true;
}

TWL_CODE void ShowGameInfo(const char gameid[], const char gamename[]) {
	iprintf("Game id: %s\nName:    %s", gameid, gamename);
}

TWL_CODE bool twl_flashcardMount(void) {
	if (REG_SCFG_MC != 0x11) {
		sysSetCardOwner (BUS_OWNER_ARM9);

		// Reset Slot-1 to allow reading title name and ID
		if (slot1Enabled) {
			disableSlot1();
			for(int i = 0; i < 25; i++) { swiWaitForVBlank(); }
			slot1Enabled = false;
		}
		if (appInited) {
			for(int i = 0; i < 35; i++) { swiWaitForVBlank(); }	// Make sure cart is inserted correctly
		}
		if (REG_SCFG_MC == 0x11) {
			sysSetCardOwner (BUS_OWNER_ARM7);
			return false;
		}
		if (!slot1Enabled) {
			enableSlot1();
			for(int i = 0; i < 15; i++) { swiWaitForVBlank(); }
			slot1Enabled = true;
		}

		nds.gameCode[0] = 0;
		nds.gameTitle[0] = 0;
		char gamename[13];
		char gameid[5];

		UpdateCardInfo(&gameid[0], &gamename[0]);

		sysSetCardOwner (BUS_OWNER_ARM7);	// 3DS fix

		if (gameid[0] >= 0x00 && gameid[0] < 0x20) {
			return false;
		}

		if (flashcardFound()) {
			fatGetVolumeLabel("fat", fatLabel);
			fixLabel(true);
			return true;
		}
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
	} else {
		return twl_flashcardMount();
	}
}

void flashcardUnmount(void) {
	fatUnmount("fat");
	fatLabel[0] = '\0';
	fatSize = 0;
	flashcardMounted = false;
}
