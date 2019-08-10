#include "ndsHeader.h"
#include "fileBrowse.h"
//#include "graphics.h"
#include <fstream>
//#include "colors.h"

/*void loadIcon(u8 *tilesSrc, u16 *palSrc, std::vector<u16> &imageBuffer, bool twl) {
	// Load pixels
	int PY = 32;
	if(twl)	PY *= 8;
	const int PX = 32;
	imageBuffer.reserve(PX*PY);
	const int TILE_SIZE_Y = 8;
	const int TILE_SIZE_X = 8;
	int index = 0;
	for (int tileY = 0; tileY < PY / TILE_SIZE_Y; ++tileY) {
		for (int tileX = 0; tileX < PX / TILE_SIZE_X; ++tileX) {
			for (int pY = 0; pY < TILE_SIZE_Y; pY++) {
				for (int pX = 0; pX < TILE_SIZE_X;pX+=2) {
					if((tilesSrc[index]&0xF) == 0) {
						imageBuffer[pX + (tileX * TILE_SIZE_X) + PX * (pY + tileY * TILE_SIZE_Y)] = 0<<15;
					} else {
						imageBuffer[pX + (tileX * TILE_SIZE_X) + PX * (pY + tileY * TILE_SIZE_Y)] = palSrc[tilesSrc[index]&0xF] | BIT(15);
					}
					if((tilesSrc[index]>>4) == 0) {
						imageBuffer[pX+1 + (tileX * TILE_SIZE_X) + PX * (pY + tileY * TILE_SIZE_Y)] = 0<<15;
					} else {
						imageBuffer[pX+1 + (tileX * TILE_SIZE_X) + PX * (pY + tileY * TILE_SIZE_Y)] = palSrc[tilesSrc[index]>>4] | BIT(15);
					}
					index++;
				}
			}
		}
	}
}*/

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

		/*// Icon
		if (ndsBanner.version == NDS_BANNER_VER_DSi) {
			loadIcon(ndsBanner.dsi_icon[0], ndsBanner.dsi_palette[0], imageBuffer, true);
		} else {
			loadIcon(ndsBanner.icon, ndsBanner.palette, imageBuffer, false);
		}*/

		// Title
		title = StringUtils::UTF16toUTF8((char16_t*)ndsBanner.titles[0]);
	}
}
