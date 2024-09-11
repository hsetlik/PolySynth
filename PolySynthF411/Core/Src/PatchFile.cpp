/*
 * PatchFile.cpp
 *
 *  Created on: Sep 11, 2024
 *      Author: hayden
 */
#include "PatchFile.h"

using namespace PatchFile;

const std::string headerBlank = "POLYSYNTHPATCH_****************_"; // note: the first asterisk is at index 15

uint16_t patchFileSizeBytes(){
	return (uint16_t)headerBlank.length() + PATCH_SIZE_BYTES;
}


void writePatchToBuf(patch_t* patch, const std::string& name, char* buf){
	// 1. set up the header and the file name
	std::string header = headerBlank;
	// now copy the name into the first however many chars of the header
	const uint8_t nameStartIdx = 15;
	for(uint8_t c = 0; c < name.length(); c++){
		header[nameStartIdx + c] = name[c];
	}
	// this guy will be important
	char* ptr = buf;
	// copy the header into the buffer
	for(uint8_t i = 0; i < header.length(); i++){
		*ptr = header[i];
		++ptr;
	}
	// now we memcpy each parameter in order

	memcpy(ptr, &patch->cutoffBase, sizeof(uint16_t));
	ptr += sizeof(uint16_t);
	memcpy(ptr, &patch->resBase, sizeof(uint16_t));
	ptr += sizeof(uint16_t);
	memcpy(ptr, &patch->foldBase, sizeof(uint16_t));
	ptr += sizeof(uint16_t);
	memcpy(ptr, &patch->foldFirst, 1);
	++ptr;
	memcpy(ptr, &patch->highPassMode, 1);
	++ptr;

	// copy the mod data
	for(uint8_t m = 0; m < MAX_MODULATIONS; m++){
		memcpy(ptr, &patch->modMatrix.mods[m], sizeof(mod_t));
		ptr += sizeof(mod_t);
	}

	// copy the osc data
	for(uint8_t o = 0; o < 2; o++){
		memcpy(ptr, &patch->oscs[o].coarseTune, 1);
		++ptr;
		memcpy(ptr, &patch->oscs[o].fineTune, 1);
		++ptr;
		memcpy(ptr, &patch->oscs[o].pulseWidth, sizeof(uint16_t));
		ptr += sizeof(uint16_t);
		memcpy(ptr, &patch->oscs[o].pulseLevel, 1);
		++ptr;
		memcpy(ptr, &patch->oscs[o].sawLevel, 1);
		++ptr;
		memcpy(ptr, &patch->oscs[o].triLevel, 1);
		++ptr;
		memcpy(ptr, &patch->oscs[o].oscLevel, 1);
		++ptr;
	}

	// copy the env data
	for(uint8_t e = 0; e < 2; e++){
		memcpy(ptr, &patch->envs[e].attack, sizeof(float));
		ptr += sizeof(float);
		memcpy(ptr, &patch->envs[e].decay, sizeof(float));
		ptr += sizeof(float);
		memcpy(ptr, &patch->envs[e].sustain, sizeof(float));
		ptr += sizeof(float);
		memcpy(ptr, &patch->envs[e].release, sizeof(float));
		ptr += sizeof(float);
	}

	// copy the LFO data
	for(uint8_t l = 0; l < 3; l++){
		memcpy(ptr, &patch->lfos[l].freq, sizeof(float));
		ptr += sizeof(float);
		memcpy(ptr, &patch->lfos[l].lfoType, 1);
		++ptr;
	}

	// and finally the sustain pedal setting
	memcpy(ptr, &patch->useSustainPedal, 1);
}



// this basically just undoes the above
void loadPatchFromBuf(patch_t*  patch, char* buf){

	// pointer to the first byte after the header
	char* ptr = buf + headerBlank.length();

	// filter/folder parameters
	memcpy(&patch->cutoffBase, ptr, sizeof(uint16_t));
	ptr += sizeof(uint16_t);
	memcpy(&patch->resBase, ptr, sizeof(uint16_t));
	ptr += sizeof(uint16_t);
	memcpy(&patch->foldBase, ptr, sizeof(uint16_t));
	ptr += sizeof(uint16_t);
	memcpy(&patch->foldFirst, ptr, 1);
	++ptr;
	memcpy(&patch->highPassMode, ptr, 1);
	++ptr;

	// copy the mod data
	for(uint8_t m = 0; m < MAX_MODULATIONS; m++){
		memcpy(&patch->modMatrix.mods[m], ptr, sizeof(mod_t));
		ptr += sizeof(mod_t);
	}

	// copy the osc data
	for(uint8_t o = 0; o < 2; o++){
		memcpy(&patch->oscs[o].coarseTune, ptr, 1);
		++ptr;
		memcpy(&patch->oscs[o].fineTune, ptr, 1);
		++ptr;
		memcpy(&patch->oscs[o].pulseWidth, ptr, sizeof(uint16_t));
		ptr += sizeof(uint16_t);
		memcpy(&patch->oscs[o].pulseLevel, ptr, 1);
		++ptr;
		memcpy(&patch->oscs[o].sawLevel, ptr, 1);
		++ptr;
		memcpy(&patch->oscs[o].triLevel, ptr, 1);
		++ptr;
		memcpy(&patch->oscs[o].oscLevel, ptr, 1);
		++ptr;
	}

	// copy the env data
	for(uint8_t e = 0; e < 2; e++){
		memcpy(&patch->envs[e].attack, ptr, sizeof(float));
		ptr += sizeof(float);
		memcpy(&patch->envs[e].decay, ptr, sizeof(float));
		ptr += sizeof(float);
		memcpy(&patch->envs[e].sustain, ptr, sizeof(float));
		ptr += sizeof(float);
		memcpy(&patch->envs[e].release, ptr, sizeof(float));
		ptr += sizeof(float);
	}

	// copy the LFO data
	for(uint8_t l = 0; l < 3; l++){
		memcpy(&patch->lfos[l].freq, ptr, sizeof(float));
		ptr += sizeof(float);
		memcpy(&patch->lfos[l].lfoType, ptr, 1);
		++ptr;
	}

	// and finally the sustain pedal setting
	memcpy(&patch->useSustainPedal, ptr, 1);
}


bool isValidPatch(char* buf){
	std::string headerPrefix(buf, 15);
	if(headerPrefix != headerBlank.substr(0, 15)){
		return false;
	}
	return buf[headerBlank.length() - 1] == '_';
}

std::string getPatchName(char* buf){
	std::string name = "";
	char* ptr = buf + 15;
	while(*ptr != "*" && name.length() < 16){
		name += *ptr;
		ptr++;
	}
	return name;
}

