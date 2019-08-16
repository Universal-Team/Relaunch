#include <algorithm>
#include <dirent.h>
#include <fat.h>
#include <limits.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <vector>

#include "driveMenu.h"
#include "fileBrowse.h"
#include "fileOperations.h"
#include "font.h"
#include "common/nds_loader_arm9.h"
#include "common/inifile.h"
#include "common/nitrofs.h"
#include "common/ndsHeader.h"
#include "main.h"
#include "optionsMenu.h"

char sdLabel[12], fatLabel[12];
int sdSize = 0, fatSize = 0;
bool sdMounted = false, sdMountedDone = false;	// true if SD mount is successful once

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
	fatInitDefault();
	if (flashcardFound()) {
		fatGetVolumeLabel("fat", fatLabel);
		fixLabel(true);
		return true;
	}
	return false;
}
