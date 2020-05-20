/*-----------------------------------------------------------------
 Not Copyright (ɔ) 2019 - 2020
    FlameKat53
    Pk11
    RocketRobz
    StackZ
------------------------------------------------------------------*/
#include "includes.h"

bool nameEndsWith(const std::string& name, const std::vector<std::string>& extensionList) {
	if(name.substr(0, 2) == "._") return false;

	if(name.size() == 0) return false;

	if(extensionList.size() == 0) return true;

	for(int i = 0; i <(int)extensionList.size(); i++) {
		const std::string ext = extensionList.at(i);
		if(strcasecmp(name.c_str() + name.size() - ext.size(), ext.c_str()) == 0) return true;
	}
	return false;
}

bool dirEntryPredicate (const DirEntry& lhs, const DirEntry& rhs) {
	if (!lhs.isDirectory && rhs.isDirectory) {
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory) {
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void findNdsFiles(std::vector<DirEntry>& dirContents) {
	std::vector<std::string> extensionList = {"nds", "dsi", "srl"};
	findFiles(dirContents, extensionList);
}

void findFiles(std::vector<DirEntry>& dirContents, std::vector<std::string> extensionList) {
	struct stat st;
	DIR *pdir = opendir(".");

	if (pdir == NULL) {
		iprintf("Internal error, unable to open the directory.");
		for(int i=0;i<120;i++)
			swiWaitForVBlank();
	} else {
		char path[PATH_MAX];
		getcwd(path, PATH_MAX);
		while (true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if (pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.fullPath = path + dirEntry.name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;
			if(!(dirEntry.isDirectory) && dirEntry.name.length() >= 3) {
				if (nameEndsWith(dirEntry.name, extensionList)) {
					dirContents.push_back(dirEntry);
				}
			} else if (dirEntry.isDirectory 
				&& dirEntry.name.compare(".") != 0 
				&& dirEntry.name.compare("..") != 0
				&& dirEntry.name.compare(".") != 0
				&& dirEntry.name.compare("_nds/nds-bootstrap") != 0
				&& dirEntry.name.compare("_nds/TWiLightMenu") != 0
				&& dirEntry.name.compare("_gba") != 0
				&& dirEntry.name.compare("3ds") != 0
				&& dirEntry.name.compare("DCIM") != 0
				&& dirEntry.name.compare("gm9") != 0
				&& dirEntry.name.compare("luma") != 0
				&& dirEntry.name.compare("Nintendo 3DS") != 0
				&& dirEntry.name.compare("private") != 0
				&& dirEntry.name.compare("retroarch") != 0
				&& dirEntry.name.compare("roms/sms") != 0
				&& dirEntry.name.compare("roms/sg1000") != 0
				&& dirEntry.name.compare("roms/gen") != 0
				&& dirEntry.name.compare("roms/snes") != 0
				&& dirEntry.name.compare("roms/nes") != 0
				&& dirEntry.name.compare("roms/gb") != 0
				&& dirEntry.name.compare("roms/gbc") != 0
				&& dirEntry.name.compare("roms/gba") != 0
				&& dirEntry.name.compare("roms/gg") != 0
				&& dirEntry.name.compare("roms/nds/saves") != 0) {
				chdir(dirEntry.name.c_str());
				findNdsFiles(dirContents);
				chdir("..");
			}
		}
		closedir(pdir);
	}
}
