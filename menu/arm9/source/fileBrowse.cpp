#include "fileBrowse.h"
#include <algorithm>
#include <dirent.h>
#include <nds.h>
#include <unistd.h>

off_t getFileSize(const char *fileName) {
	FILE* fp = fopen(fileName, "rb");
	off_t fsize = 0;
	if (fp) {
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);			// Get source file's size
		fseek(fp, 0, SEEK_SET);
	}
	fclose(fp);

	return fsize;
}


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

void getDirectoryContents (std::vector<DirEntry>& dirContents) {
	std::vector<std::string> extensionList;
	getDirectoryContents(dirContents, extensionList);
}

void getDirectoryContents (std::vector<DirEntry>& dirContents, std::vector<std::string>& extensionList) {
	struct stat st;
	dirContents.clear();

	DIR *pdir = opendir (".");

	if (pdir == NULL) {
		iprintf ("Unable to open the directory.\n");
	} else {

		while(true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;

			stat(pent->d_name, &st);
			if (strcmp(pent->d_name, "..") != 0) {
				dirEntry.name = pent->d_name;
				dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;
				if (!dirEntry.isDirectory) {
					dirEntry.size = getFileSize(dirEntry.name.c_str());
				}
				if((dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "nds")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "NDS")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "dsi")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "DSI")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "srl")
				|| (dirEntry.name.substr(dirEntry.name.find_last_of(".") + 1) == "SRL"))
				{
					dirEntry.isApp = true;
				} else {
					dirEntry.isApp = false;
				}

				if (dirEntry.name.compare(".") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name, extensionList))) {
					dirContents.push_back (dirEntry);
				}
			}

		}

		closedir(pdir);
	}

	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);

	DirEntry dirEntry;
	dirEntry.isDirectory = true;
	dirEntry.isApp = false;
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
			} else if (dirEntry.isDirectory && dirEntry.name.compare(".") != 0 && dirEntry.name.compare("..") != 0) {
				chdir(dirEntry.name.c_str());
				findNdsFiles(dirContents);
				chdir("..");
			}
		}
		closedir(pdir);
	}
}
