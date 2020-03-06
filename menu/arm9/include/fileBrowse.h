/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019 - 2020
    Evan "FlameKat53" Rodgers
    Evie "Epicpkmn11" "Evie11" Bogie
    Rojelio "RocketRobz" Reyes
    Kim "StackZ" Perkovic
------------------------------------------------------------------*/
#ifndef FILE_BROWSE_H
#define FILE_BROWSE_H

#include <string>
#include <vector>

struct DirEntry {
	std::string name;
	std::string fullPath;
	off_t size;
	bool isDirectory;
	bool isApp;
};

void findNdsFiles(std::vector<DirEntry>& dirContents);
void findFiles(std::vector<DirEntry>& dirContents, std::vector<std::string> extensionList);

#endif
