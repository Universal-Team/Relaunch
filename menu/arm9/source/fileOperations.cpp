/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019
	Flame
	Epicpkmn11
	RocketRobz
	VoltZ
------------------------------------------------------------------*/
#include "includes.h"

// char sdLabel[12], fatLabel[12];
// int sdSize = 0, fatSize = 0;
bool sdMounted = false, sdMountedDone = false;	// true if SD mount is successful once

/*void fixLabel(bool fat) {
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
}*/

TWL_CODE bool sdMount(void) {
	fatMountSimple("sd", get_io_dsisd());
	if (access("sd:/", F_OK) == 0) {
		sdMountedDone = true;
		// fatGetVolumeLabel("sd", sdLabel);
		// fixLabel(false);
		return true;
	}
	return false;
}

bool flashcardMount(void) {
	fatInitDefault();
	if (access("fat:/", F_OK) == 0) {
		// fatGetVolumeLabel("fat", fatLabel);
		// fixLabel(true);
		return true;
	}
	return false;
}
