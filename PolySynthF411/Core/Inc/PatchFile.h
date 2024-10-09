/*
 * PatchFile.h
 *
 *  Created on: Sep 11, 2024
 *      Author: hayden
 */

#ifndef INC_PATCHFILE_H_
#define INC_PATCHFILE_H_
#include "Patch.h"

#define PATCH_NAME_MAX_LENGTH 16
#define AUTHOR_NAME_MAX_LENGTH 16

#ifdef __cplusplus

#include "fatfs.h"
#include <cstring>
#include <string>
#include <vector>

struct PatchMetadata {
	std::string path;
	std::string name;
	std::string author;
	uint8_t type;
};

struct SynthConfig {
	std::string defaultPatchPath;
	std::string lastUsedPatchPath;
	std::string patchAuthorName;
};

class PatchBrowser {
private:
	FATFS fileSys;
	std::vector<PatchMetadata> availablePatches;
	bool validPatchAtPath(const std::string& path);
	PatchMetadata metadataForPatch(const std::string& path);
// helper for setting up defaults on a new SD card
	void initNewCard();
public:
	PatchBrowser();
	// attempts to mount the attached SD card. returns success or failure.
	bool init();
	// if a card is mounted, load the paths of all the valid patch files
	void loadAvailablePatches();
	// set the default patch
	void loadDefaultPatch(patch_t* dest);
	uint16_t numPatchesAvailable();
	bool attemptPatchWrite(PatchMetadata md, patch_t* patch);
	bool attemptPatchLoad(const std::string& path, patch_t* patch);


};



#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_PATCHFILE_H_ */
