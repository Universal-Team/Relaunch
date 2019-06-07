#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

using namespace std;

struct DirEntry {
	string name;
	off_t size;
	bool isDirectory;
	bool isApp;
} ;

std::string browseForFile (void);
void findNdsFiles(vector<DirEntry>& dirContents);
void getDirectoryContents (vector<DirEntry>& dirContents);
extern void showDirectoryContents (const vector<DirEntry>& dirContents, int fileOffset, int startRow);
extern bool flashcardMountSkipped;

extern void driveMenu (void);
extern void eqMenu (void);

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
extern bool sdMount(void);
extern bool flashcardMount(void);

#endif //MENU_H
