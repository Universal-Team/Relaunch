/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019 - 2020
    FlameKat53
    Pk11
    RocketRobz
    StackZ
------------------------------------------------------------------*/
#ifndef MAIN_H
#define MAIN_H

#define SCREEN_COLS 30
#define ENTRIES_PER_SCREEN 19
#define ENTRIES_START_ROW 3
#define ENTRIES_START_ROW_EQ 3
#define ENTRY_PAGE_LENGTH 10

extern char filePath[PATH_MAX];
extern char titleName[32];

extern int screenMode;

extern bool appInited;
extern bool applaunch;

extern bool arm7SCFGLocked;
extern bool isRegularDS;

extern void setFontTop();
extern void setFontSub();

#endif //MAIN_H
