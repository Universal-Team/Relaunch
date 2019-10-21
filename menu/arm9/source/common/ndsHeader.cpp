#include "ndsHeader.h"
#include "fileBrowse.h"
#include "utils.hpp"
#include <fstream>

extern bool nameEndsWith(const std::string& name, const std::vector<std::string> extensionList);

void getIconTitle(std::string name, std::vector<u16> &imageBuffer, std::string &title) {
	if(nameEndsWith(name, {"nds", "dsi", "srl"})) {
		NDSHeader ndsHeader;
		NDSBanner ndsBanner;

		FILE *rom = fopen(name.c_str(), "rb");
		if(!rom) return;

		fseek(rom, 0, SEEK_SET); // Seek to the start of the file
		fread(&ndsHeader, sizeof(ndsHeader), 1, rom); // Read the header
		fseek(rom, ndsHeader.bannerOffset, SEEK_SET); // Seek to the banner
		fread(&ndsBanner, sizeof(ndsBanner), 1, rom); // Read the banner

		// Title
		title = StringUtils::UTF16toUTF8((char16_t*)ndsBanner.titles[0]);
	}
}
