#include "gfx.hpp"

#include "bg.h"
#include "letterTiles.h"

#include <array>
#include <nds.h>
#include <stdio.h>

std::vector<Sprite> sprites;
std::vector<u16 *> letterGfx;

constexpr std::array<u16, 16 * 5> letterPalettes = {
	0x0000, 0x6B5A, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C, // White
	0x0000, 0x4631, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C, // White (darker border)
	0x0000, 0x39CE, 0x39CE, 0x7FFF, 0x77DD, 0x739C, 0x6F7B, 0x6B5A, 0x6739, 0x6318, 0x5AD6, 0x56B5, 0x4E94, 0x4A52, 0x4631, 0x4210, // Gray
	0x0000, 0x2ED8, 0x2ED8, 0x7FFF, 0x77DE, 0x73BD, 0x6B9C, 0x677C, 0x5F5B, 0x5B5B, 0x573A, 0x4F3A, 0x4B1A, 0x42F9, 0x3AF9, 0x32D8, // Yellow
	0x0000, 0x32AD, 0x32AD, 0x7FFF, 0x77DD, 0x73BC, 0x6F9B, 0x6B7A, 0x6359, 0x5F57, 0x5B36, 0x5314, 0x4AF3, 0x42D1, 0x3AAE, 0x36AD  // Green
};

void initGraphics() {
	consoleDemoInit();

	videoSetMode(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);

	int bg0 = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bgTiles, bgGetGfxPtr(bg0), bgTilesLen);
	dmaCopy(bgPal, BG_PALETTE, bgPalLen);
	dmaCopy(bgMap, bgGetMapPtr(bg0), bgMapLen);

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	dmaCopy(letterPalettes.data(), SPRITE_PALETTE, letterPalettes.size() * sizeof(u16));

	constexpr int tileSize = 32 * 32 / 2;
	for(int i = 0; i < letterTilesTilesLen / tileSize; i++) {
		letterGfx.push_back(oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color));
		dmaCopy(letterTilesTiles + (i * tileSize), letterGfx.back(), tileSize);
	}

	for(int i = 0; i < WORD_LEN * MAX_GUESSES; i++) {
		sprites.emplace_back(true, SpriteSize_32x32, SpriteColorFormat_16Color);
		sprites.back().xy((((256 - (WORD_LEN * 24 + (WORD_LEN - 1) * 2)) / 2) - 4) + (i % WORD_LEN) * 26, (25 + (167 - (MAX_GUESSES * 24 + (MAX_GUESSES - 1) * 2)) / 2 - 4) + (i / WORD_LEN) * 26).gfx(letterGfx[0]);
	}
	Sprite::update(true);
}
