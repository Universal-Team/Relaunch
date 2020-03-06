/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019 - 2020
    Evan "FlameKat53" Rodgers
    Evie "Epicpkmn11" "Evie11" Bogie
    Rojelio "RocketRobz" Reyes
    Kim "StackZ" Perkovic
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
