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
void getDirectoryContents (vector<DirEntry>& dirContents);
extern bool flashcardMountSkipped;

extern void driveMenu (void);



#endif //MENU_H
