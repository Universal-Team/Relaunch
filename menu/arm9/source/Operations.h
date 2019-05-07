#include <nds.h>

#ifndef FILE_COPY
#define FILE_COPY

extern char clipboard[256];
extern char clipboardFilename[256];
extern bool clipboardFolder;
extern bool clipboardOn;
extern bool clipboardUsed;
extern bool clipboardDrive;	// false == SD card, true == Flashcard

extern void printBytes(int bytes);

extern off_t getFileSize(const char *fileName);
extern int fcopy(const char *sourcePath, const char *destinationPath);
extern u8 stored_SCFG_MC;

extern bool sdMounted;
extern bool sdMountedDone;				// true if SD mount is successful once
extern bool flashcardMounted;

extern bool secondaryDrive;			// false == SD card, true == Flashcard

extern char sdLabel[12];
extern char fatLabel[12];

extern int sdSize;
extern int fatSize;

extern bool sdFound(void);
extern bool flashcardFound(void);
extern bool bothSDandFlashcard(void);
extern bool sdMount(void);
extern void sdUnmount(void);
extern bool flashcardMount(void);
extern void flashcardUnmount(void);

#endif // FILE_COPY
