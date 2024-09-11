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

#ifdef __cplusplus
#include <string.h>
#include <string>

namespace PatchFile {
	uint16_t patchFileSizeBytes();
	void writePatchToBuf(patch_t* patch, const std::string& name, char* buf);
	void loadPatchFromBuf(patch_t*  patch, char* buf);
}

#endif

#ifdef __cplusplus
#define EXTERNC extern "c"
#else
#define EXTERNC
#endif

/* ----C-FRIENDLY BINDINGS GO HERE------*/

#undef EXTERNC

#endif /* INC_PATCHFILE_H_ */
