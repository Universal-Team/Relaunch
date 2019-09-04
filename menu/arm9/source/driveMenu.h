/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019
	Flame
	Epicpkmn11
	RocketRobz
	VoltZ
------------------------------------------------------------------*/
#ifndef DRIVE_MENU_H
#define DRIVE_MENU_H

#include <string>
#include <vector>

#include "fileBrowse.h"

extern bool flashcardMountSkipped;

extern void driveMenu (std::vector<DirEntry> ndsFiles);

extern u8 stored_SCFG_MC;

extern bool sdMounted;
extern bool sdMountedDone;	// true if SD mount is successful once
extern bool flashcardMounted;

extern bool secondaryDrive;	// false == SD card, true == Flashcard

#endif //MENU_H
