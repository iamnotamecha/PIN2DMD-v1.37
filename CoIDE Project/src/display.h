
/**
 * 	PIN2DMD display.h
 *
 *      (c) 2016 by Joerg Amann, Stefan Rinke
 *
 *	This work is licensed under a Creative
 *	Commons Attribution-NonCommercial-
 *	ShareAlike 4.0 International License.
 *
 *	http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdint.h>

#include "framebuffer.h"

void writeText(const char* text, uint8_t x, uint8_t y, int len, uint8_t color);

#endif
