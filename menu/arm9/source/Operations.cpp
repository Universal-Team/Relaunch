#include <nds.h>
#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "menu.h"
#include "Operations.h"
#include "main.h"
using namespace std;

#define copyBufSize 0x8000

u32 copyBuf[copyBufSize];

char clipboard[256];
char clipboardFilename[256];
bool clipboardFolder = false;
bool clipboardOn = false;
bool clipboardUsed = false;
bool clipboardDrive = false;	// false == SD card, true == Flashcard

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

void printBytes(int bytes)
{
	if (abs(bytes) == 1)
		iprintf("%d Byte", bytes);

	else if (abs(bytes) < 1024)
		iprintf("%d Bytes", bytes);

	else if (abs(bytes) < 1024 * 1024)
		printf("%.1f KB", (float)bytes / 1024);

	else if (abs(bytes) < 1024 * 1024 * 1024)
		printf("%.1f MB", (float)bytes / 1024 / 1024);

	else
		printf("%.1f GB", (float)bytes / 1024 / 1024 / 1024);
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

void dirCopy(DirEntry* entry, int i, const char *destinationPath, const char *sourcePath) {
	vector<DirEntry> dirContents;
	dirContents.clear();
	if (entry->isDirectory)	chdir((sourcePath + ("/" + entry->name)).c_str());
	getDirectoryContents(dirContents);
	if (((int)dirContents.size()) == 1)	mkdir((destinationPath + ("/" + entry->name)).c_str(), 0777);
	if (((int)dirContents.size()) != 1)	fcopy((sourcePath + ("/" + entry->name)).c_str(), (destinationPath + ("/" + entry->name)).c_str());
}

int fcopy(const char *sourcePath, const char *destinationPath)
{
	DIR *isDir = opendir (sourcePath);
	
	if (isDir != NULL) {
		closedir(isDir);

		// Source path is a directory
		chdir(sourcePath);
		vector<DirEntry> dirContents;
		getDirectoryContents(dirContents);
		DirEntry* entry = &dirContents.at(1);
		
		mkdir(destinationPath, 0777);
		for (int i = 1; i < ((int)dirContents.size()); i++) {
			chdir(sourcePath);
			entry = &dirContents.at(i);
			dirCopy(entry, i, destinationPath, sourcePath);
		}

		chdir (destinationPath);
		chdir ("..");
		return 1;
	} else {
		closedir(isDir);

		// Source path is a file
	    FILE* sourceFile = fopen(sourcePath, "rb");
	    off_t fsize = 0;
	    if (sourceFile) {
	        fseek(sourceFile, 0, SEEK_END);
	        fsize = ftell(sourceFile);			// Get source file's size
			fseek(sourceFile, 0, SEEK_SET);
		} else {
			fclose(sourceFile);
			return -1;
		}

	    FILE* destinationFile = fopen(destinationPath, "wb");
		//if (destinationFile) {
			fseek(destinationFile, 0, SEEK_SET);

		off_t offset = 0;
		int numr;
		while (1)
		{
			scanKeys();
			if (keysHeld() & KEY_B) {
				// Cancel copying
				fclose(sourceFile);
				fclose(destinationFile);
				return -1;
				break;
			}
			printf ("\x1b[16;0H");
			printf ("Progress:\n");
			printf ("%i/%i Bytes                       ", (int)offset, (int)fsize);

			// Copy file to destination path
			numr = fread(copyBuf, 2, copyBufSize, sourceFile);
			fwrite(copyBuf, 2, numr, destinationFile);
			offset += copyBufSize;

			if (offset > fsize) {
				fclose(sourceFile);
				fclose(destinationFile);

				printf ("\x1b[17;0H");
				printf ("%i/%i Bytes                       ", (int)fsize, (int)fsize);
				for (int i = 0; i < 30; i++) swiWaitForVBlank();

				return 1;
				break;
			}
		}

		return -1;
	}
}
