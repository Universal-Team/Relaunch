/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019 - 2020
    FlameKat53
    Pk11
    RocketRobz
    StackZ
------------------------------------------------------------------*/
#include "includes.h"

// char sdLabel[12], fatLabel[12];
// int sdSize = 0, fatSize = 0;
bool sdMounted = false, sdMountedDone = false;	// true if SD mount is successful once

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
