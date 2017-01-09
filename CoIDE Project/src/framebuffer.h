/**
 * 	PIN2DMD framebuffer.
 *
 *      (c) 2015 by Joerg Amann, Stefan Rinke
 *
 *	This work is licensed under a Creative 
 *	Commons Attribution-NonCommercial-
 *	ShareAlike 4.0 International License.
 *
 *	http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 */

#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include <stdint.h>

#define subframeSizeInByte 512
#define targetSubframeSize 4096
#define displaybufSize 2048*3 // for 12 Bit color depth
#define targetSubframes 4
#define MD5DigestLen 16
#define panelWidth 128
#define panelHeight 32

#define numberOfColorsPerPalette 16
#define numberOfPalettes 8
#define PAL_NOT_FOUND NULL

typedef struct  pinMap {
    uint8_t digest[MD5DigestLen];
    int     palIndex;
    int     numberOfFrames;
    uint32_t offset;
    struct pinMap *next;
} PalMapping;

typedef struct palStruct {
	uint8_t index;
	uint8_t rgbData[numberOfColorsPerPalette*3];
	struct palStruct* next;
} Palette;

void createRGBDisplayBuffer_4bit(uint8_t* src, uint8_t* target, int activePalette);
void createRGBDisplayBuffer_2bit(uint8_t* src, uint8_t* target, int activePalette);
void createRGBDisplayBuffer_12bit(uint8_t* src, uint8_t* target, int nSrcSubframes, int nTargetSubframes, int activePalette);


void addPalette(uint8_t* palData, uint8_t index );
Palette* getPalette(uint8_t index);
void initPalettes();
void deleteAllPalettes();

extern uint8_t rgbbuf[targetSubframeSize*targetSubframes];
extern uint8_t displaybuf[displaybufSize];
// extern uint8_t palettes[numberOfPalettes][numberOfColorsPerPalette*3];

#endif /* FRAMEBUFFER_H_ */
