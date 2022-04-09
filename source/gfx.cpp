#include "gfx.hpp"
#include "defines.hpp"
#include "tonccpy.h"

#include "bgBottom.h"
#include "bgTop.h"
#include "letterTiles.h"
#include "main_nftr.h"
#include "refreshButton.h"

#include <array>
#include <nds.h>
#include <stdio.h>

std::vector<Sprite> letterSprites;
std::vector<u16 *> letterGfx, letterGfxSub;
Font mainFont;
Sprite *refreshSprite = nullptr;

constexpr std::array<std::array<u16, 16 * 5>, 2> letterPalettes = {{
	{
		0x0000, 0x6B5A, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C, // White
		0x0000, 0x4631, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C, // White (darker border)
		0x0000, 0x39CE, 0x39CE, 0x7FFF, 0x77DD, 0x739C, 0x6F7B, 0x6B5A, 0x6739, 0x6318, 0x5AD6, 0x56B5, 0x4E94, 0x4A52, 0x4631, 0x4210, // Gray
		0x0000, 0x2ED8, 0x2ED8, 0x7FFF, 0x77DE, 0x73BD, 0x6B9C, 0x677C, 0x5F5B, 0x5B5B, 0x573A, 0x4F3A, 0x4B1A, 0x42F9, 0x3AF9, 0x32D8, // Yellow
		0x0000, 0x32AD, 0x32AD, 0x7FFF, 0x77DD, 0x73BC, 0x6F9B, 0x6B7A, 0x6359, 0x5F57, 0x5B36, 0x5314, 0x4AF3, 0x42D1, 0x3AAE, 0x36AD  // Green

	},
	{
		0x0000, 0x6B5A, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C, // White
		0x0000, 0x4631, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C, // White (darker border)
		0x0000, 0x39CE, 0x39CE, 0x7FFF, 0x77DD, 0x739C, 0x6F7B, 0x6B5A, 0x6739, 0x6318, 0x5AD6, 0x56B5, 0x4E94, 0x4A52, 0x4631, 0x4210, // Gray
		0x0000, 0x7AF0, 0x7AF0, 0x7FFF, 0x7FDE, 0x7BDD, 0x7BBC, 0x7BBB, 0x7B9A, 0x7B99, 0x7B78, 0x7B57, 0x7B56, 0x7B35, 0x7B13, 0x7B12, // Light blue (Yellow)
		0x0000, 0x1DFD, 0x1DFD, 0x7FFF, 0x7BDF, 0x739E, 0x6B7E, 0x635E, 0x5B1E, 0x56DE, 0x4EBD, 0x469D, 0x427D, 0x363D, 0x2A1D, 0x21FD  // Orange (Green)
	}
}};

constexpr std::array<std::array<u16, 16>, 2> fontPal = {{
	{
		0xFFFF, 0xDEF7, 0xC631, 0x8000, // Black
		0x0000, 0xEF7B, 0xD6B5, 0xC631, // Gray
		0x39CE, 0xC631, 0xF39C, 0xFFFF, // White on gray
		0x32AD, 0xC2D1, 0xDF57, 0xFFFF, // White on green
	},
	{
		0xFFFF, 0xDEF7, 0xC631, 0x8000, // Black
		0x0000, 0xEF7B, 0xD6B5, 0xC631, // Gray
		0x39CE, 0xC631, 0xF39C, 0xFFFF, // White on gray
		0x1DFD, 0xB63D, 0xD6DE, 0xFFFF, // White on orange
	}
}};

void initGraphics() {
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(BG(0), 3);
	bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(BG_SUB(0), 3);
	bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 5, 0);
	bgSetPriority(BG_SUB(2), 0);

	// Copy BG data
	tonccpy(bgGetGfxPtr(BG(0)), bgTopTiles, bgTopTilesLen);
	tonccpy(BG_PALETTE, bgTopPal, bgTopPalLen);
	tonccpy(bgGetMapPtr(BG(0)), bgTopMap, bgTopMapLen);
	tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomTiles, bgBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, bgBottomPal, bgBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomMap, bgBottomMapLen);

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);

	constexpr int tileSize = 32 * 32 / 2;
	for(int i = 0; i < letterTilesTilesLen / tileSize; i++) {
		letterGfx.push_back(oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color));
		letterGfxSub.push_back(oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_16Color));
		tonccpy(letterGfx.back(), letterTilesTiles + (i * tileSize), tileSize);
		tonccpy(letterGfxSub.back(), letterTilesTiles + (i * tileSize), tileSize);
	}

	for(int i = 0; i < WORD_LEN * MAX_GUESSES; i++) {
		letterSprites.emplace_back(true, SpriteSize_32x32, SpriteColorFormat_16Color);
		letterSprites.back().move((((256 - (WORD_LEN * 24 + (WORD_LEN - 1) * 2)) / 2) - 4) + (i % WORD_LEN) * 26, (25 + (167 - (MAX_GUESSES * 24 + (MAX_GUESSES - 1) * 2)) / 2 - 4) + (i / WORD_LEN) * 26).gfx(letterGfx[0]);
	}
	Sprite::update(true);

	if(refreshSprite)
		delete refreshSprite;
	refreshSprite = new Sprite(false, SpriteSize_64x64, SpriteColorFormat_16Color);
	u16 *refreshGfx = oamAllocateGfx(&oamSub, SpriteSize_64x64, SpriteColorFormat_16Color);
	tonccpy(refreshGfx, refreshButtonTiles, refreshButtonTilesLen);
	refreshSprite->move(96, 36).visible(false).gfx(refreshGfx);

	mainFont = std::move(Font(main_nftr, main_nftr_size));
}

void setPalettes(bool altPalette) {
	// Sprites
	tonccpy(SPRITE_PALETTE, letterPalettes[altPalette].data(), letterPalettes[altPalette].size() * sizeof(u16));
	tonccpy(SPRITE_PALETTE_SUB, letterPalettes[altPalette].data(), letterPalettes[altPalette].size() * sizeof(u16));

	// Fonts
	tonccpy(BG_PALETTE_SUB + 0xF0, fontPal[altPalette].data(), fontPal[altPalette].size() * sizeof(u16));
}

void flipSprites(Sprite *letterSprites, int count, std::vector<TilePalette> newPalettes, FlipOptions option) {
	for(int i = 0; i < count + 1; i++) {
		Sprite *sprite = i >= count ? nullptr : (letterSprites + i);
		Sprite *prevSprite = i <= 0 ? nullptr : (letterSprites + i - 1);

		if(option == FlipOptions::show && prevSprite)
			prevSprite->affineIndex(-1, false).visible(true);

		if(sprite && sprite->visible())
			sprite->affineIndex(0, false);
		if(prevSprite && prevSprite->visible())
			prevSprite->affineIndex(1, false);

		for(int j = 0; j < 15; j++) {
			if(sprite && sprite->visible())
				sprite->affineTransform(1.0f, 0.0f, 0.0f, 1.0f - (j * (1.0f / 15.0f)) + 0.0001f).update();
			if(prevSprite && prevSprite->visible())
				prevSprite->affineTransform(1.0f, 0.0f, 0.0f, 1.0f - ((15 - j) * (1.0f / 15.0f)) + 0.0001f).update();
			swiWaitForVBlank();
		}

		if(sprite && i < (int)newPalettes.size())
			sprite->palette(newPalettes[i]);
		if(prevSprite && prevSprite->visible())
			prevSprite->affineIndex(-1, false).update();

		if(option == FlipOptions::hide && sprite)
			sprite->affineIndex(-1, false).visible(false).update();
	}
}
