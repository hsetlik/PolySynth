/*
 * PatchFile.cpp
 *
 *  Created on: Sep 11, 2024
 *      Author: hayden
 */
#include "PatchFile.h"
constexpr std::string defaultPatchHeader() {
	std::string str = "SYNTHPATCH V0.1\n"; // 16 chars
	str += "NAME:****************\n"; // 21 chars
	str += "ATHR:****************\n"; // 21 chars
	str += "TYPE:****************\n"; // 21 chars
	return str;
}

constexpr uint16_t patchFileSize() {
	return PATCH_SIZE_BYTES + defaultPatchHeader().length();
}

void writeHeaderFor(char *buf, const std::string &name,
		const std::string &author, uint8_t category) {
	std::string str = defaultPatchHeader();
	// 1. update the name
	uint16_t offset = 16 + 5;
	for (uint8_t c = 0; c < name.length(); c++) {
		str[offset + c] = name[c];
	}
	//2. update the author
	offset += 22;
	for (uint8_t i = 0; i < author.length(); i++) {
		str[offset + i] = author[i];
	}
	// 3. stringify the category
	std::string categStr = "";
	if (category == 0)
		categStr = "BASS";
	else if (category == 1)
		categStr = "LEAD";
	else if (category == 2)
		categStr = "KEYS";
	else
		categStr = "PADS";
	// 4. set the category
	offset += 22;
	for (uint8_t i = 0; i < categStr.length(); i++) {
		str[offset + i] = categStr[i];
	}

	//5. strcpy
	strcpy(buf, (char*) str.c_str());
}

SynthConfig decodeSynthConfig(char *buf, size_t size) {
	SynthConfig conf;
	std::string configStr(buf, size);
	// to figure out the offset of the first byte
	constexpr std::string leaderStr = "SYNTHCONFIG V0.1\nDefaultPatch:\n";
	uint16_t idx = (uint16_t)leaderStr.length();
	conf.defaultPatchPath = "";
	while(configStr[idx] != '*'){
		conf.defaultPatchPath += configStr[idx];
		++idx;
	}
	constexpr std::string lastHeaderStr = "LastUsedPatch:";
	idx += lastHeaderStr.length();
	conf.lastUsedPatchPath = "";
	while(configStr[idx] != '*'){
		conf.lastUsedPatchPath += configStr[idx];
		++idx;
	}
	constexpr std::string authHeaderStr = "AuthorName:";
	idx += authHeaderStr.length();
	conf.patchAuthorName = "";
	while(configStr[idx] != '*'){
		conf.patchAuthorName += configStr[idx];
		++idx;
	}
	return conf;
}

constexpr std::string emptySynthConfig(){
	std::string str = "SYNTHCONFIG V0.1\n";
	str += "DefaultPatch:*************************************\n";
	str += "LastUsedPatch:************************************\n";
	str += "AuthorName:****************\n";
	return str;
}

std::string encodeSynthConfig(SynthConfig conf){
	std::string str = emptySynthConfig();
	constexpr std::string defaultHeaderStr = "DefaultPatch:";
	size_t idx = str.find(defaultHeaderStr) + defaultHeaderStr.length();
	for(auto& c : conf.defaultPatchPath){
		str[idx] = c;
		++idx;
	}
	constexpr std::string lastHeaderStr = "LastUsedPatch:";
	idx = str.find(lastHeaderStr) + lastHeaderStr.length();
	for(auto& c : conf.lastUsedPatchPath){
		str[idx] = c;
		++idx;
	}

	constexpr std::string authHeaderStr = "AuthorName:";
	idx = str.find(authHeaderStr) + authHeaderStr.length();
	for(auto& c : conf.patchAuthorName){
		str[idx] = c;
		++idx;
	}
	return str;

}

//================================

PatchBrowser::PatchBroswer() {

}

bool PatchBrowser::init() {
	return f_mount(&fileSys, "", 1) == FR_OK;
}

bool PatchBrowser::validPatchAtPath(const std::string &path) {
	constexpr uint16_t headerBytes = (uint16_t) defaultPatchHeader().length();
	char buffer[headerBytes];

	FIL file;
	UINT readResult;

	if (f_open(&file, path.c_str(), FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		if (f_read(&file, buffer, headerBytes, &readResult) != FR_OK) {
			return false;
		}
		/*
		 *
		 std::string str = "SYNTHPATCH V0.1\n"; // 16 chars
		 str += "NAME:****************\n"; // 22 chars
		 str += "ATHR:****************\n"; // 22 chars
		 str += "TYPE:****************\n"; // 22 chars
		 */
		bool allValid = true;
		std::string header(buffer, headerBytes);
		std::string versionStr = header.substr(0, 12);
		if (versionStr.compare("SYNTHPATCH V"))
			allValid = false;
		std::string nameStr = header.substr(16, 5);
		if (nameStr.compare("NAME:"))
			allValid = false;
		std::string athrStr = header.substr(16 + 22, 5);
		if (athrStr.compare("ATHR:"))
			allValid = false;
		std::string typeStr = header.substr(16 + 22 + 22, 5);
		if (typeStr.compare("TYPE:"))
			allValid = false;
		f_close(&file);
		return allValid;
	} else {
		return false;
	}
}

PatchMetadata PatchBrowser::metadataForPatch(const std::string &path) {
	PatchMetadata md;
	md.path = path;
	md.name = "";
	md.author = "";
	constexpr uint16_t headerBytes = (uint16_t) defaultPatchHeader().length();
	char buffer[headerBytes];

	FIL file;
	UINT readResult;

	if (f_open(&file, path.c_str(), FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		if (f_read(&file, buffer, headerBytes, &readResult) != FR_OK) {
			Error_Handler();
		}
		std::string header(buffer, headerBytes);
		std::string nameFull = header.substr(16 + 5, 16);
		uint8_t idx = 0;
		while (nameFull[idx] != '*' && idx < 16) {
			md.name += nameFull[idx];
			++idx;
		}

		std::string authorFull = header.substr(16 + 5 + 22, 16);
		idx = 0;

		while (authorFull[idx] != '*' && idx < 16) {
			md.author += authorFull[idx];
			++idx;
		}
		std::string typeStr = header.substr(16 + 5 + 22 + 22, 4);
		if (typeStr == "BASS")
			md.category = 0;
		else if (typeStr == "LEAD")
			md.category = 1;
		else if (typeStr == "KEYS")
			md.category = 2;
		else
			md.category = 3;
		f_close(&file);
	}
	return md;
}

void PatchBrowser::loadAvailablePatches() {
	DIR patchDir;
	FILINFO info;
	FRESULT res = f_opendir(&patchDir, "patches");
	if (res == FR_OK) {
		res = f_readdir(&patchDir, &info);
		while (res == FR_OK && info.fname[0] != 0) {
			std::string path = "patches/" + std::string(info.fname);
			if (validPatchAtPath(path)) {
				availablePatches.push_back(metadataForPatch(path));
			}
			res = f_readdir(&patchDir, &info);
		}
	}

	f_closedir(&patchDir);
}

void PatchBrowser::initNewCard(patch_t* destPatch) {
	FIL file;
	UINT br;
	if(f_open(&file,".synthconfig", FA_CREATE_NEW | FA_WRITE | FA_READ) == FR_OK){

		// 1. create .synthconfig file
		SynthConfig conf;
		conf.defaultPatchPath = "patches/default";
		conf.lastUsedPatchPath = "patches/default";
		conf.patchAuthorName = "USER";

		std::string configStr = encodeSynthConfig(conf);
		char* buf = (char*)configStr.c_str();

		if(f_write(&file, buf, configStr.length(), &br) == FR_OK){
			f_close(&file);
		} else {
			Error_Handler();
		}

		// 2. create default patch file
		patch_t patch = getDefaultPatch();
		PatchMetadata md;
		md.author = conf.patchAuthorName;
		md.path = conf.defaultPatchPath;
		md.name = "Default";
		md.type = patch_category_t::pKeys;
		if(!attemptPatchWrite(md, &patch)){
			Error_Handler();
		}
		attemptPatchLoad(conf.defaultPatchPath, destPatch);
	} else {
		Error_Handler();
	}
}

void PatchBrowser::loadDefaultPatch(patch_t *dest) {
	FIL file;
	FILINFO info;
	if (f_stat(".synthconfig", &info) == FR_OK) { // we already have a config file
		if (f_open(&file, ".synthconfig", FA_READ | FA_WRITE) == FR_OK) {
			UINT bytesRead;
			char buf[info.fsize];
			f_read(&file, buf, info.fsize, &bytesRead);
			f_close(&file);
			SynthConfig conf = decodeSynthConfig(buf, info.fsize);
			if(!attemptPatchLoad(conf.defaultPatchPath, dest)){
				Error_Handler();
			}
		} else {
			Error_Handler();
		}

	} else { // we need create the defaults
		initNewCard(dest);
	}

}

uint16_t PatchBrowser::numPatchesAvailable() {
	return (uint16_t) availablePatches.size();
}

bool PatchBrowser::attemptPatchWrite(PatchMetadata md, patch_t *patch) {
	char fileBuf[patchFileSize()];
	char *head = fileBuf;
	writeHeaderFor(head, md.name, md.author, md.type);
	head = &fileBuf[defaultPatchHeader().length()];
	// no reason this shouldn't work right?
	memcpy(head, patch, PATCH_SIZE_BYTES);

	//check if the patch exists already
	bool patchExists = false;
	for (uint8_t i = 0; i < availablePatches.size(); i++) {
		if (availablePatches[i].name == md.name) {
			patchExists = true;
			break;
		}
	}

	// now file writing time
	FIL file;
	UINT bytesWritten = 0;
	FRESULT res =
			patchExists ?
					f_open(&file, md.path.c_str(), FA_READ | FA_WRITE) :
					f_open(&file, md.path.c_str(),
					FA_CREATE_NEW | FA_WRITE | FA_READ);
	if (res == FR_OK) {
		res = f_write(&file, fileBuf, patchFileSize(), &bytesWritten);
		f_close(&file);
		return res == FR_OK;
	}
	return false;
}

bool PatchBrowser::attemptPatchLoad(const std::string &path, patch_t *patch) {
	// assuming the path we've been passed is a valid patch file
	char fileBuf[patchFileSize()];
	FIL file;
	UINT bytesRead = 0;
	FRESULT res = f_open(&file, path.c_str(), FA_READ);
	if (res == FR_OK) {
		res = f_read(&file, fileBuf, sizeof(fileBuf), &bytesRead);
		f_close(&file);
		if (res != FR_OK)
			return false;
	} else {
		return false;
	}
	// now copy the patch data into place
	char *head = &fileBuf[defaultPatchHeader().length()];
	memcpy(patch, head, PATCH_SIZE_BYTES);
	return true;
}

