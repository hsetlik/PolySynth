/*
 * Graphics.cpp
 *
 *  Created on: Aug 22, 2024
 *      Author: hayden
 */

#include "Graphics.h"


//======================================================

GraphicsProcessor::GraphicsProcessor() :dmaBusy(false) {
	// initialize the screen
	ILI9341_Init();

}
//======================================================

graphics_processor_t create_graphics_processor(){
	return new GraphicsProcessor();
}

void disp_dma_finished(graphics_processor_t proc){
	GraphicsProcessor* ptr = static_cast<GraphicsProcessor*>(proc);
	ptr->dmaFinished();
}




