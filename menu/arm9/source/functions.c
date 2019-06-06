/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2010
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
------------------------------------------------------------------*/
#include <string.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <sys/stat.h>
#include <limits.h>

#include <unistd.h>
#include <fat.h>

#include "load_bin.h"

#ifndef _NO_BOOTSTUB_
#include "bootstub_bin.h"
#endif

#include "nds_loader_arm9.h"
#define LCDC_BANK_C (u16*)0x06840000
#define STORED_FILE_CLUSTER (*(((u32*)LCDC_BANK_C) + 1))
#define INIT_DISC (*(((u32*)LCDC_BANK_C) + 2))
#define WANT_TO_PATCH_DLDI (*(((u32*)LCDC_BANK_C) + 3))

#define STORED_FILE_CLUSTER_OFFSET 4 
#define INIT_DISC_OFFSET 8
#define WANT_TO_PATCH_DLDI_OFFSET 12
#define ARG_START_OFFSET 16
#define ARG_SIZE_OFFSET 20
#define HAVE_DSISD_OFFSET 28
#define DSIMODE_OFFSET 32
#define CLEAR_MASTER_BRIGHT_OFFSET 36


typedef signed int addr_t;
typedef unsigned char data_t;

#define FIX_ALL	0x01
#define FIX_GLUE	0x02
#define FIX_GOT	0x04
#define FIX_BSS	0x08

enum DldiOffsets {
	DO_magicString = 0x00,			// "\xED\xA5\x8D\xBF Chishm"
	DO_magicToken = 0x00,			// 0xBF8DA5ED
	DO_magicShortString = 0x04,		// " Chishm"
	DO_version = 0x0C,
	DO_driverSize = 0x0D,
	DO_fixSections = 0x0E,
	DO_allocatedSpace = 0x0F,

	DO_friendlyName = 0x10,

	DO_text_start = 0x40,			// Data start
	DO_data_end = 0x44,				// Data end
	DO_glue_start = 0x48,			// Interworking glue start	-- Needs address fixing
	DO_glue_end = 0x4C,				// Interworking glue end
	DO_got_start = 0x50,			// GOT start					-- Needs address fixing
	DO_got_end = 0x54,				// GOT end
	DO_bss_start = 0x58,			// bss start					-- Needs setting to zero
	DO_bss_end = 0x5C,				// bss end

	// IO_INTERFACE data
	DO_ioType = 0x60,
	DO_features = 0x64,
	DO_startup = 0x68,	
	DO_isInserted = 0x6C,	
	DO_readSectors = 0x70,	
	DO_writeSectors = 0x74,
	DO_clearStatus = 0x78,
	DO_shutdown = 0x7C,
	DO_code = 0x80
};

static addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

static void vramcpy (void* dst, const void* src, int len)
{
	u16* dst16 = (u16*)dst;
	u16* src16 = (u16*)src;
	
	//dmaCopy(src, dst, len);

	for ( ; len > 0; len -= 2) {
		*dst16++ = *src16++;
	}
}	

static addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
	const int* dataChunk = (const int*) data;
	int searchChunk = ((const int*)search)[0];
	addr_t i;
	addr_t dataChunkEnd = (addr_t)(dataLen / sizeof(int));

	for ( i = 0; i < dataChunkEnd; i++) {
		if (dataChunk[i] == searchChunk) {
			if ((i*sizeof(int) + searchLen) > dataLen) {
				return -1;
			}
			if (memcmp (&data[i*sizeof(int)], search, searchLen) == 0) {
				return i*sizeof(int);
			}
		}
	}

	return -1;
}

// Normal DLDI uses "\xED\xA5\x8D\xBF Chishm"
// Bootloader string is different to avoid being patched
static const data_t dldiMagicLoaderString[] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

#define DEVICE_TYPE_DLDI 0x49444C44

static bool dldiPatchLoader (data_t *binData, u32 binSize, bool clearBSS)
{
	addr_t memOffset;			// Offset of DLDI after the file is loaded into memory
	addr_t patchOffset;			// Position of patch destination in the file
	addr_t relocationOffset;	// Value added to all offsets within the patch to fix it properly
	addr_t ddmemOffset;			// Original offset used in the DLDI file
	addr_t ddmemStart;			// Start of range that offsets can be in the DLDI file
	addr_t ddmemEnd;			// End of range that offsets can be in the DLDI file
	addr_t ddmemSize;			// Size of range that offsets can be in the DLDI file

	addr_t addrIter;

	data_t *pDH;
	data_t *pAH;

	size_t dldiFileSize = 0;
	
	// Find the DLDI reserved space in the file
	patchOffset = quickFind (binData, dldiMagicLoaderString, binSize, sizeof(dldiMagicLoaderString));

	if (patchOffset < 0) {
		// does not have a DLDI section
		return false;
	}

	pDH = (data_t*)(io_dldi_data);
	
	pAH = &(binData[patchOffset]);

	if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) {
		// No DLDI patch
		return false;
	}

	if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) {
		// Not enough space for patch
		return false;
	}
	
	dldiFileSize = 1 << pDH[DO_driverSize];

	memOffset = readAddr (pAH, DO_text_start);
	if (memOffset == 0) {
			memOffset = readAddr (pAH, DO_startup) - DO_code;
	}
	ddmemOffset = readAddr (pDH, DO_text_start);
	relocationOffset = memOffset - ddmemOffset;

	ddmemStart = readAddr (pDH, DO_text_start);
	ddmemSize = (1 << pDH[DO_driverSize]);
	ddmemEnd = ddmemStart + ddmemSize;

	// Remember how much space is actually reserved
	pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
	// Copy the DLDI patch into the application
	vramcpy (pAH, pDH, dldiFileSize);

	// Fix the section pointers in the header
	writeAddr (pAH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
	writeAddr (pAH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
	writeAddr (pAH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
	writeAddr (pAH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
	writeAddr (pAH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
	writeAddr (pAH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
	writeAddr (pAH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
	writeAddr (pAH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
	// Fix the function pointers in the header
	writeAddr (pAH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
	writeAddr (pAH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
	writeAddr (pAH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
	writeAddr (pAH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
	writeAddr (pAH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
	writeAddr (pAH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);

	if (pDH[DO_fixSections] & FIX_ALL) { 
		// Search through and fix pointers within the data section of the file
		for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GLUE) { 
		// Search through and fix pointers within the glue section of the file
		for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GOT) { 
		// Search through and fix pointers within the Global Offset Table section of the file
		for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (clearBSS && (pDH[DO_fixSections] & FIX_BSS)) { 
		// Initialise the BSS to 0, only if the disc is being re-inited
		memset (&pAH[readAddr(pDH, DO_bss_start) - ddmemStart] , 0, readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start));
	}

	return true;
}

int runNds (const void* loader, u32 loaderSize, u32 cluster, bool initDisc, bool dldiPatchNds, int argc, const char** argv, bool clearMasterBright)
{
	char* argStart;
	u16* argData;
	u16 argTempVal = 0;
	int argSize;
	const char* argChar;

	irqDisable(IRQ_ALL);

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Load the loader/patcher into the correct address
	vramcpy (LCDC_BANK_C, loader, loaderSize);

	// Set the parameters for the loader
	// STORED_FILE_CLUSTER = cluster;
	writeAddr ((data_t*) LCDC_BANK_C, STORED_FILE_CLUSTER_OFFSET, cluster);
	// INIT_DISC = initDisc;
	writeAddr ((data_t*) LCDC_BANK_C, INIT_DISC_OFFSET, initDisc);

	writeAddr ((data_t*) LCDC_BANK_C, DSIMODE_OFFSET, isDSiMode());
	if(argv[0][0]=='s' && argv[0][1]=='d') {
		dldiPatchNds = false;
		writeAddr ((data_t*) LCDC_BANK_C, HAVE_DSISD_OFFSET, 1);
	}

	writeAddr ((data_t*) LCDC_BANK_C, CLEAR_MASTER_BRIGHT_OFFSET, clearMasterBright);

	// WANT_TO_PATCH_DLDI = dldiPatchNds;
	writeAddr ((data_t*) LCDC_BANK_C, WANT_TO_PATCH_DLDI_OFFSET, dldiPatchNds);
	// Give arguments to loader
	argStart = (char*)LCDC_BANK_C + readAddr((data_t*)LCDC_BANK_C, ARG_START_OFFSET);
	argStart = (char*)(((int)argStart + 3) & ~3);	// Align to word
	argData = (u16*)argStart;
	argSize = 0;
	
	for (; argc > 0 && *argv; ++argv, --argc) 
	{
		for (argChar = *argv; *argChar != 0; ++argChar, ++argSize) 
		{
			if (argSize & 1) 
			{
				argTempVal |= (*argChar) << 8;
				*argData = argTempVal;
				++argData;
			} 
			else 
			{
				argTempVal = *argChar;
			}
		}
		if (argSize & 1)
		{
			*argData = argTempVal;
			++argData;
		}
		argTempVal = 0;
		++argSize;
	}
	*argData = argTempVal;
	
	writeAddr ((data_t*) LCDC_BANK_C, ARG_START_OFFSET, (addr_t)argStart - (addr_t)LCDC_BANK_C);
	writeAddr ((data_t*) LCDC_BANK_C, ARG_SIZE_OFFSET, argSize);

		
	if(dldiPatchNds) {
		// Patch the loader with a DLDI for the card
		if (!dldiPatchLoader ((data_t*)LCDC_BANK_C, loaderSize, initDisc)) {
			return 3;
		}
	}

	irqDisable(IRQ_ALL);

	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;	
	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	*((vu32*)0x02FFFFFC) = 0;
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;

	resetARM7(0x06000000);

	swiSoftReset(); 
	return true;
}

int runNdsFile (const char* filename, int argc, const char** argv, bool clearMasterBright)  {
	struct stat st;
	char filePath[PATH_MAX];
	int pathLen;
	const char* args[1];

	
	if (stat (filename, &st) < 0) {
		return 1;
	}

	if (argc <= 0 || !argv) {
		// Construct a command line if we weren't supplied with one
		if (!getcwd (filePath, PATH_MAX)) {
			return 2;
		}
		pathLen = strlen (filePath);
		strcpy (filePath + pathLen, filename);
		args[0] = filePath;
		argv = args;
	}

	bool havedsiSD = false;

	if(argv[0][0]=='s' && argv[0][1]=='d') havedsiSD = true;
	
	installBootStub(havedsiSD);

	return runNds (load_bin, load_bin_size, st.st_ino, true, true, argc, argv, clearMasterBright);
}

bool installBootStub(bool havedsiSD) {
#ifndef _NO_BOOTSTUB_
	extern char *fake_heap_end;
	struct __bootstub *bootstub = (struct __bootstub *)fake_heap_end;
	u32 *bootloader = (u32*)(fake_heap_end+bootstub_bin_size);

	memcpy(bootstub,bootstub_bin,bootstub_bin_size);
	memcpy(bootloader,load_bin,load_bin_size);
	bool ret = false;

	bootloader[8] = isDSiMode();
	if( havedsiSD) {
		ret = true;
		bootloader[3] = 0; // don't dldi patch
		bootloader[7] = 1; // use internal dsi SD code
	} else {
		ret = dldiPatchLoader((data_t*)bootloader, load_bin_size,false);
	}
	bootstub->arm9reboot = (VoidFn)(((u32)bootstub->arm9reboot)+fake_heap_end);
	bootstub->arm7reboot = (VoidFn)(((u32)bootstub->arm7reboot)+fake_heap_end);
	bootstub->bootsize = load_bin_size;

	DC_FlushAll();

	return ret;
#else
	return true;
#endif

}
/*
	nitrofs.c - eris's wai ossum nitro filesystem device driver
		Based on information found at http://frangoassado.org/ds/rom_spec.txt and from the #dsdev ppls
		Kallisti (K) 2008-01-26 All rights reversed.
	2008-05-19  v0.2 - New And Improved!! :DDD
		* fix'd the fseek SEEK_CUR issue (my fseek funct should not have returned a value :/)
		* also thx to wintermute's input realized:
			* if you dont give ndstool the -o wifilogo.bmp option it will run on emulators in gba mode
			* you then dont need the gba's LOADEROFFSET, so it was set to 0x000
	
	2008-05-21  v0.3 - newer and more improved
		* fixed some issues with ftell() (again was fseek's fault u_u;;)
		* fixed possible error in detecting sc.gba files when using dldi
		* readded support for .gba files in addition to .nds emu
		* added stat() support for completedness :)
	2008-05-22  v0.3.1 - slight update
		* again fixed fseek(), this time SEEK_END oddly i kinda forgot about it >_> sry
		* also went ahead and inlined the functions, makes slight proformance improvement
	2008-05-26  v0.4 - added chdir
		* added proper chdir functionality
	2008-05-30  v0.5.Turbo - major speed improvement
		* This version uses a single filehandle to access the .nds file when not in GBA mode
		  improving the speed it takes to open a .nds file by around 106ms. This is great for 
		  situations requiring reading alot of seperate small files. However it does take a little
		  bit longer when reading from multiple files simultainously 
		  (around 122ms over 10,327 0x100 byte reads between 2 files).
	2008-06-09  
		* Fixed bug with SEEK_END where it wouldnt utilize the submitted position.. 
		  (now can fseek(f,-128,SEEK_END) to read from end of file :D)
	2008-06-18 v0.6.Turbo - . and .. :D
		* Today i have added full "." and ".." support.
		  dirnext() will return . and .. first, and all relevent operations will 
		  support . and .. in pathnames. 
	2009-05-10 v0.7.Turbo - small changes  @_@?!
	2009-08-08 v0.8.Turbo - fix fix fix
		* fixed problem with some cards where the header would be loaded to GBA ram even if running 
                  in NDS mode causing nitroFSInit() to think it was a valid GBA cart header and attempt to 
           	  read from GBA SLOT instead of SLOT 1. Fixed this by making it check that filename is not NULL
 		  and then to try FAT/SLOT1 first. The NULL option allows forcing nitroFS to use gba.
    
    2018-09-05 v0.9 - modernize devoptab (by RonnChyran)
        * Updated for libsysbase change in devkitARM r46 and above. 
*/

#include <string.h>
#include <errno.h>
#include "nds.h"
#include "nitrofs.h"

//This seems to be a typo! memory.h has REG_EXEMEMCNT
#ifndef REG_EXMEMCNT
#define REG_EXMEMCNT (*(vuint16 *)0x04000204)
#endif

#define __itcm __attribute__((section(".itcm")))

//Globals!
u32 fntOffset;   //offset to start of filename table
u32 fatOffset;   //offset to start of file alloc table
bool hasLoader;  //single global nds filehandle (is null if not in dldi/fat mode)
u16 chdirpathid; //default dir path id...
FILE *ndsFile;
off_t ndsFileLastpos; //Used to determine need to fseek or not

devoptab_t nitroFSdevoptab = {
    "nitro",                       //	const char *name;
    sizeof(struct nitroFSStruct),  //	int	structSize;
    &nitroFSOpen,                  //	int (*open_r)(struct _reent *r, void *fileStruct, const char *path,int flags,int mode);
    &nitroFSClose,                 //	int (*close_r)(struct _reent *r,void* fd);
    NULL,                          //	int (*write_r)(struct _reent *r,void* fd,const char *ptr,int len);
    &nitroFSRead,                  //	int (*read_r)(struct _reent *r,void* fd,char *ptr,int len);
    &nitroFSSeek,                  //	int (*seek_r)(struct _reent *r,void* fd,int pos,int dir);
    &nitroFSFstat,                 //	int (*fstat_r)(struct _reent *r,void* fd,struct stat *st);
    &nitroFSstat,                  //	int (*stat_r)(struct _reent *r,const char *file,struct stat *st);
    NULL,                          //	int (*link_r)(struct _reent *r,const char *existing, const char  *newLink);
    NULL,                          //	int (*unlink_r)(struct _reent *r,const char *name);
    &nitroFSChdir,                 //	int (*chdir_r)(struct _reent *r,const char *name);
    NULL,                          //	int (*rename_r) (struct _reent *r, const char *oldName, const char *newName);
    NULL,                          //	int (*mkdir_r) (struct _reent *r, const char *path, int mode);
    sizeof(struct nitroDIRStruct), //	int dirStateSize;
    &nitroFSDirOpen,               //	DIR_ITER* (*diropen_r)(struct _reent *r, DIR_ITER *dirState, const char *path);
    &nitroDirReset,                //	int (*dirreset_r)(struct _reent *r, DIR_ITER *dirState);
    &nitroFSDirNext,               //	int (*dirnext_r)(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
    &nitroFSDirClose               //	int (*dirclose_r)(struct _reent *r, DIR_ITER *dirState);

};

//K, i decided to inline these, improves speed slightly..
//these 2 'sub' functions deal with actually reading from either gba rom or .nds file :)
//what i rly rly rly wanna know is how an actual nds cart reads from itself, but it seems no one can tell me ~_~
//so, instead we have this weird weird haxy try gbaslot then try dldi method. If i (or you!!) ever do figure out
//how to read the proper way can replace these 4 functions and everything should work normally :)

//reads from rom image either gba rom or dldi
inline ssize_t nitroSubRead(off_t *npos, void *ptr, size_t len)
{
    if (ndsFile != NULL)
    { //read from ndsfile
        if (ndsFileLastpos != *npos)
            fseek(ndsFile, *npos, SEEK_SET); //if we need to, move! (might want to verify this succeed)
        len = fread(ptr, 1, len, ndsFile);
    }
    else
    {                                             //reading from gbarom
        memcpy(ptr, *npos + (void *)GBAROM, len); //len isnt checked here because other checks exist in the callers (hopefully)
    }
    if (len > 0)
        *npos += len;
    ndsFileLastpos = *npos; //save the current file nds pos
    return (len);
}

//seek around
inline void nitroSubSeek(off_t *npos, int pos, int dir)
{
    if ((dir == SEEK_SET) || (dir == SEEK_END)) //otherwise just set the pos :)
        *npos = pos;
    else if (dir == SEEK_CUR)
        *npos += pos; //see ez!
}

//Figure out if its gba or ds, setup stuff
int __itcm
nitroFSInit(const char *ndsfile)
{
    off_t pos = 0;
    char romstr[0x10];
    chdirpathid = NITROROOT;
    ndsFileLastpos = 0;
    ndsFile = NULL;
    if (ndsfile != NULL)
    {
        if ((ndsFile = fopen(ndsfile, "rb")))
        {
            nitroSubRead(&pos, romstr, strlen(LOADERSTR));
            if (strncmp(romstr, LOADERSTR, strlen(LOADERSTR)) == 0)
            {
                nitroSubSeek(&pos, LOADEROFFSET + FNTOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fntOffset, sizeof(fntOffset));
                nitroSubSeek(&pos, LOADEROFFSET + FATOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fatOffset, sizeof(fatOffset));
                fatOffset += LOADEROFFSET;
                fntOffset += LOADEROFFSET;
                hasLoader = true;
            }
            else
            {
                nitroSubSeek(&pos, FNTOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fntOffset, sizeof(fntOffset));
                nitroSubSeek(&pos, FATOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fatOffset, sizeof(fatOffset));
                hasLoader = false;
            }
            setvbuf(ndsFile, NULL, _IONBF, 0); //we dont need double buffs u_u
            AddDevice(&nitroFSdevoptab);
            return (1);
        }
    }
    REG_EXMEMCNT &= ~ARM7_OWNS_CARD; //give us gba slot ownership
    if (strncmp(((const char *)GBAROM) + LOADERSTROFFSET, LOADERSTR, strlen(LOADERSTR)) == 0)
    { // We has gba rahm
        printf("yes i think this is GBA?!\n");
        if (strncmp(((const char *)GBAROM) + LOADERSTROFFSET + LOADEROFFSET, LOADERSTR, strlen(LOADERSTR)) == 0)
        { //Look for second magic string, if found its a sc.nds or nds.gba
            printf("sc/gba\n");
            fntOffset = ((u32) * (u32 *)(((const char *)GBAROM) + FNTOFFSET + LOADEROFFSET)) + LOADEROFFSET;
            fatOffset = ((u32) * (u32 *)(((const char *)GBAROM) + FATOFFSET + LOADEROFFSET)) + LOADEROFFSET;
            hasLoader = true;
            AddDevice(&nitroFSdevoptab);
            return (1);
        }
        else
        { //Ok, its not a .gba build, so must be emulator
            printf("gba, must be emu\n");
            fntOffset = ((u32) * (u32 *)(((const char *)GBAROM) + FNTOFFSET));
            fatOffset = ((u32) * (u32 *)(((const char *)GBAROM) + FATOFFSET));
            hasLoader = false;
            AddDevice(&nitroFSdevoptab);
            return (1);
        }
    }
    return (0);
}

//Directory functs
DIR_ITER *nitroFSDirOpen(struct _reent *r, DIR_ITER *dirState, const char *path)
{
    struct nitroDIRStruct *dirStruct = (struct nitroDIRStruct *)dirState->dirStruct; //this makes it lots easier!
    struct stat st;
    char dirname[NITRONAMELENMAX];
    char *cptr;
    char mydirpath[NITROMAXPATHLEN]; //to hold copy of path string
    char *dirpath = mydirpath;
    bool pathfound;
    if ((cptr = strchr(path, ':')))
        path = cptr + 1;                           //move path past any device names (if it was nixy style wouldnt need this step >_>)
    strncpy(dirpath, path, sizeof(mydirpath) - 1); //copy the string (as im gonna mutalate it)
    dirStruct->pos = 0;
    if (*dirpath == '/')                   //if first character is '/' use absolute root path plz
        dirStruct->cur_dir_id = NITROROOT; //first root dir
    else
        dirStruct->cur_dir_id = chdirpathid; //else use chdirpath
    nitroDirReset(r, dirState);              //set dir to current path
    do
    {
        while ((cptr = strchr(dirpath, '/')) == dirpath)
        {
            dirpath++; //move past any leading / or // together
        }
        if (cptr)
            *cptr = 0; //erase /
        if (*dirpath == 0)
        {                     //are we at the end of the path string?? if so there is nothing to search for we're already here !
            pathfound = true; //mostly this handles searches for root or /  or no path specified cases
            break;
        }
        pathfound = false;
        while (nitroFSDirNext(r, dirState, dirname, &st) == 0)
        {
            if ((st.st_mode == S_IFDIR) && !(strcmp(dirname, dirpath)))
            {                                              //if its a directory and name matches dirpath
                dirStruct->cur_dir_id = dirStruct->dir_id; //move us to the next dir in tree
                nitroDirReset(r, dirState);                //set dir to current path we just found...
                pathfound = true;
                break;
            }
        };
        if (!pathfound)
            break;
        dirpath = cptr + 1; //move to right after last / we found
    } while (cptr);         // go till after the last /
    if (pathfound)
    {
        return (dirState);
    }
    else
    {
        r->_errno = ENOENT;
        return (NULL);
    }
}

int nitroFSDirClose(struct _reent *r, DIR_ITER *dirState)
{
    return (0);
}

/*Consts containing relative system path strings*/
const char *syspaths[2] = {
    ".",
    ".."};

//reset dir to start of entry selected by dirStruct->cur_dir_id which should be set in dirOpen okai?!
int nitroDirReset(struct _reent *r, DIR_ITER *dirState)
{
    struct nitroDIRStruct *dirStruct = (struct nitroDIRStruct *)dirState->dirStruct; //this makes it lots easier!
    struct ROM_FNTDir dirsubtable;
    off_t *pos = &dirStruct->pos;
    nitroSubSeek(pos, fntOffset + ((dirStruct->cur_dir_id & NITRODIRMASK) * sizeof(struct ROM_FNTDir)), SEEK_SET);
    nitroSubRead(pos, &dirsubtable, sizeof(dirsubtable));
    dirStruct->namepos = dirsubtable.entry_start;    //set namepos to first entry in this dir's table
    dirStruct->entry_id = dirsubtable.entry_file_id; //get number of first file ID in this branch
    dirStruct->parent_id = dirsubtable.parent_id;    //save parent ID in case we wanna add ../ functionality
    dirStruct->spc = 0;                              //system path counter, first two dirnext's deliver . and ..
    return (0);
}

int nitroFSDirNext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st)
{
    unsigned char next;
    struct nitroDIRStruct *dirStruct = (struct nitroDIRStruct *)dirState->dirStruct; //this makes it lots easier!
    off_t *pos = &dirStruct->pos;
    if (dirStruct->spc <= 1)
    {
        if (st)
            st->st_mode = S_IFDIR;
        if ((dirStruct->spc == 0) || (dirStruct->cur_dir_id == NITROROOT))
        { // "." or its already root (no parent)
            dirStruct->dir_id = dirStruct->cur_dir_id;
        }
        else
        { // ".."
            dirStruct->dir_id = dirStruct->parent_id;
        }
        strcpy(filename, syspaths[dirStruct->spc++]);
        return (0);
    }
    nitroSubSeek(pos, fntOffset + dirStruct->namepos, SEEK_SET);
    nitroSubRead(pos, &next, sizeof(next));
    // next: high bit 0x80 = entry isdir.. other 7 bits r size, the 16 bits following name are dir's entryid (starts with f000)
    //  00 = endoftable //
    if (next)
    {
        if (next & NITROISDIR)
        {
            if (st)
                st->st_mode = S_IFDIR;
            next &= NITROISDIR ^ 0xff; //invert bits and mask off 0x80
            nitroSubRead(pos, filename, next);
            nitroSubRead(&dirStruct->pos, &dirStruct->dir_id, sizeof(dirStruct->dir_id)); //read the dir_id
                                                                                          //grr cant get the struct member size?, just wanna test it so moving on...
                                                                                          //			nitroSubRead(pos,&dirStruct->dir_id,sizeof(u16)); //read the dir_id
            dirStruct->namepos += next + sizeof(u16) + 1;                                 //now we points to next one plus dir_id size:D
        }
        else
        {
            if (st)
                st->st_mode = 0;
            nitroSubRead(pos, filename, next);
            dirStruct->namepos += next + 1; //now we points to next one :D
            //read file info to get filesize (and for fileopen)
            nitroSubSeek(pos, fatOffset + (dirStruct->entry_id * sizeof(struct ROM_FAT)), SEEK_SET);
            nitroSubRead(pos, &dirStruct->romfat, sizeof(dirStruct->romfat)); //retrieve romfat entry (contains filestart and end positions)
            dirStruct->entry_id++;                                            //advance ROM_FNTStrFile ptr
            if (st)
                st->st_size = dirStruct->romfat.bottom - dirStruct->romfat.top; //calculate filesize
        }
        filename[(int)next] = 0; //zero last char
        return (0);
    }
    else
    {
        r->_errno = EIO;
        return (-1);
    }
}

//fs functs
int nitroFSOpen(struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fileStruct;
    struct nitroDIRStruct dirStruct;
    DIR_ITER dirState;
    dirState.dirStruct = &dirStruct; //create a temp dirstruct
    struct _reent dre;
    struct stat st;                     //all these are just used for reading the dir ~_~
    char dirfilename[NITROMAXPATHLEN];  // to hold a full path (i tried to avoid using so much stack but blah :/)
    char *filename;                     // to hold filename
    char *cptr;                         //used to string searching and manipulation
    cptr = (char *)path + strlen(path); //find the end...
    filename = NULL;
    do
    {
        if ((*cptr == '/') || (*cptr == ':'))
        { // split at either / or : (whichever comes first form the end!)
            cptr++;
            strncpy(dirfilename, path, cptr - path); //copy string up till and including/ or : zero rest
            dirfilename[cptr - path] = 0;            //it seems strncpy doesnt always zero?!
            filename = cptr;                         //filename = now remainder of string
            break;
        }
    } while (cptr-- != path); //search till start
    if (!filename)
    {                            //we didnt find a / or : ? shouldnt realyl happen but if it does...
        filename = (char *)path; //filename = complete path
        dirfilename[0] = 0;      //make directory path ""
    }
    if (nitroFSDirOpen(&dre, &dirState, dirfilename))
    {
        fatStruct->start = 0;
        while (nitroFSDirNext(&dre, &dirState, dirfilename, &st) == 0)
        {
            if (!(st.st_mode & S_IFDIR) && (strcmp(dirfilename, filename) == 0))
            { //Found the *file* youre looking for!!
                fatStruct->start = dirStruct.romfat.top;
                fatStruct->end = dirStruct.romfat.bottom;
                if (hasLoader)
                {
                    fatStruct->start += LOADEROFFSET;
                    fatStruct->end += LOADEROFFSET;
                }
                break;
            }
        }
        if (fatStruct->start)
        {
            nitroSubSeek(&fatStruct->pos, fatStruct->start, SEEK_SET); //seek to start of file
            return (0);                                                //woot!
        }
        nitroFSDirClose(&dre, &dirState);
    }
    if (r->_errno == 0)
    {
        r->_errno = ENOENT;
    }
    return (-1); //teh fail
}

int nitroFSClose(struct _reent *r, void* fd)
{
    return (0);
}

ssize_t nitroFSRead(struct _reent *r, void* fd, char *ptr, size_t len)
{
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fd;
    off_t *npos = &fatStruct->pos;
    if (*npos + len > fatStruct->end)
        len = fatStruct->end - *npos; //dont let us read past the end plz!
    if (*npos > fatStruct->end)
        return (0); //hit eof
    return (nitroSubRead(npos, ptr, len));
}

off_t nitroFSSeek(struct _reent *r, void* fd, off_t pos, int dir)
{
    //need check for eof here...
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fd;
    off_t *npos = &fatStruct->pos;
    if (dir == SEEK_SET)
        pos += fatStruct->start; //add start from .nds file offset
    else if (dir == SEEK_END)
        pos += fatStruct->end; //set start to end of file (useless?)
    if (pos > fatStruct->end)
        return (-1); //dont let us read past the end plz!
    nitroSubSeek(npos, pos, dir);
    return (*npos - fatStruct->start);
}

int nitroFSFstat(struct _reent *r, void* fd, struct stat *st)
{
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fd;
    st->st_size = fatStruct->end - fatStruct->start;
    return (0);
}

int nitroFSstat(struct _reent *r, const char *file, struct stat *st)
{
    struct nitroFSStruct fatStruct;
    struct nitroDIRStruct dirStruct;
    DIR_ITER dirState;

    if (nitroFSOpen(NULL, &fatStruct, file, 0, 0) >= 0)
    {
        st->st_mode = S_IFREG;
        st->st_size = fatStruct.end - fatStruct.start;
        return (0);
    }

    dirState.dirStruct = &dirStruct;
    if ((nitroFSDirOpen(r, &dirState, file) != NULL))
    {

        st->st_mode = S_IFDIR;
        nitroFSDirClose(r, &dirState);
        return (0);
    }
    r->_errno = ENOENT;
    return (-1);
}

int nitroFSChdir(struct _reent *r, const char *name)
{
    struct nitroDIRStruct dirStruct;
    DIR_ITER dirState;
    dirState.dirStruct = &dirStruct;
    if ((name != NULL) && (nitroFSDirOpen(r, &dirState, name) != NULL))
    {
        chdirpathid = dirStruct.cur_dir_id;
        nitroFSDirClose(r, &dirState);
        return (0);
    }
    else
    {
        r->_errno = ENOENT;
        return (-1);
    }
}
