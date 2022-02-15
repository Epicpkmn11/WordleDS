// Simplified version of TWiLight Menu++'s text rendering

#include "font.hpp"
#include "tonccpy.h"

#include <nds/arm9/background.h>
#include <string.h>

u8 Font::textBuf[2][SCREEN_SIZE];

Font::Font(const u8 *nftr, u32 nftrSize) {
	// Skip font info
	const u8 *ptr = nftr + 0x14 + nftr[0x14];

	// Load glyph info
	u32 chunkSize;
	tonccpy(&chunkSize, ptr, 4);
	ptr += 4;
	tileWidth = *(ptr++);
	tileHeight = *(ptr++);
	tonccpy(&tileSize, ptr, 2);
	ptr += 2;

	// Load character glyphs
	tileAmount = (chunkSize - 0x10) / tileSize;
	ptr += 4;
	fontTiles = new u8[tileSize * tileAmount];
	tonccpy(fontTiles, ptr, tileSize * tileAmount);

	// Load character widths
	ptr = nftr + 0x24;
	u32 locHDWC;
	tonccpy(&locHDWC, ptr, 4);
	ptr = nftr + locHDWC - 4;
	tonccpy(&chunkSize, ptr, 4);
	ptr += 4 + 8;
	fontWidths = new u8[3 * tileAmount];
	tonccpy(fontWidths, ptr, 3 * tileAmount);

	// Load character maps
	fontMap = new u16[tileAmount];

	ptr = nftr + 0x28;
	u32 locPAMC, mapType;
	tonccpy(&locPAMC, ptr, 4);

	while(locPAMC < nftrSize && locPAMC != 0) {
		u16 firstChar, lastChar;
		ptr = nftr + locPAMC;
		tonccpy(&firstChar, ptr, 2);
		ptr += 2;
		tonccpy(&lastChar, ptr, 2);
		ptr += 2;
		tonccpy(&mapType, ptr, 4);
		ptr += 4;
		tonccpy(&locPAMC, ptr, 4);
		ptr += 4;

		switch(mapType) {
			case 0: {
				u16 firstTile;
				tonccpy(&firstTile, ptr, 2);
				ptr += 2;
				for(unsigned i=firstChar;i<=lastChar;i++) {
					fontMap[firstTile+(i-firstChar)] = i;
				}
				break;
			} case 1: {
				for(int i=firstChar;i<=lastChar;i++) {
					u16 tile;
					tonccpy(&tile, ptr, 2);
					ptr += 2;
					fontMap[tile] = i;
				}
				break;
			} case 2: {
				u16 groupAmount;
				tonccpy(&groupAmount, ptr, 2);
				ptr += 2;
				for(int i=0;i<groupAmount;i++) {
					u16 charNo, tileNo;
					tonccpy(&charNo, ptr, 2);
					ptr += 2;
					tonccpy(&tileNo, ptr, 2);
					ptr += 2;
					fontMap[tileNo] = charNo;
				}
				break;
			}
		}
	}

	questionMark = getCharIndex(0xFFFD);
	if(questionMark == 0)
		questionMark = getCharIndex('?');
}

Font::~Font(void) {
	if(fontTiles)
		delete[] fontTiles;
	if(fontWidths)
		delete[] fontWidths;
	if(fontMap)
		delete[] fontMap;
}

u16 Font::getCharIndex(char16_t c) {
	// Try a binary search
	int left = 0;
	int right = tileAmount;

	while(left <= right) {
		int mid = left + ((right - left) / 2);
		if(fontMap[mid] == c) {
			return mid;
		}

		if(fontMap[mid] < c) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	return questionMark;
}

std::u16string Font::utf8to16(std::string_view text) {
	std::u16string out;
	for(uint i=0;i<text.size();) {
		char16_t c;
		if(!(text[i] & 0x80)) {
			c = text[i++];
		} else if((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if((text[i] & 0xF0) == 0xE0) {
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		} else {
			i++; // out of range or something (This only does up to 0xFFFF since it goes to a U16 anyways)
		}
		out += c;
	}
	return out;
}

int Font::calcWidth(std::u16string_view text) {
	uint x = 0;

	for(char16_t c : text) {
		u16 index = getCharIndex(c);
		x += fontWidths[(index * 3) + 2];
	}

	return x;
}

ITCM_CODE Font &Font::print(int x, int y, bool top, std::u16string_view text, Alignment align) {
	// Adjust x for alignment
	switch(align) {
		case Alignment::left: {
			break;
		} case Alignment::center: {
			size_t newline = text.find('\n');
			while(newline != text.npos) {
				print(x, y, top, text.substr(0, newline), align);
				text = text.substr(newline + 1);
				newline = text.find('\n');
				y += tileHeight;
			}

			x = ((256 - calcWidth(text)) / 2) + x;
			break;
		} case Alignment::right: {
			size_t newline = text.find('\n');
			while(newline != text.npos) {
				print(x - calcWidth(text.substr(0, newline)), y, top, text.substr(0, newline), Alignment::left);
				text = text.substr(newline + 1);
				newline = text.find('\n');
				y += tileHeight;
			}
			x = x - calcWidth(text);
			break;
		}
	}
	const int xStart = x;

	// Loop through string and print it
	for(auto it = text.begin(); it != text.end(); ++it) {
		if(*it == '\n') {
			x = xStart;
			y += tileHeight;
			continue;
		}

		u16 index = getCharIndex(*it);

		// Don't draw off screen chars
		if(x >= 0 && x + fontWidths[(index * 3) + 2] < 256 && y >= 0 && y + tileHeight < 192) {
			u8 *dst = textBuf[top] + x + fontWidths[(index * 3)];
			for(int i = 0; i < tileHeight; i++) {
				for(int j = 0; j < tileWidth; j++) {
					u8 px = fontTiles[(index * tileSize) + (i * tileWidth + j) / 4] >> ((3 - ((i * tileWidth + j) % 4)) * 2) & 3;
					if(px)
						dst[(y + i) * 256 + j] = paletteStart + px;
				}
			}
		}

		x += fontWidths[(index * 3) + 2];
	}

	return *this;
}

Font &Font::clear(bool top) {
	toncset(textBuf[top], 0, SCREEN_SIZE);
	return *this;
}

Font &Font::update(bool top, bool preserve) {
	u8 *dst = (u8 *)bgGetGfxPtr(top ? BG(2) : BG_SUB(2));

	if(preserve) {
		for(int i = 0; i < SCREEN_SIZE; i++)
			if(!textBuf[top][i])
				textBuf[top][i] = dst[i];
	}

	tonccpy(dst, textBuf[top], SCREEN_SIZE);
	return *this;
}
