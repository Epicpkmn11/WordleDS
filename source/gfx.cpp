#include "gfx.hpp"
#include "game.hpp"
#include "tonccpy.h"

#include <array>
#include <nds.h>
#include <stdio.h>

void Gfx::init() {
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

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);
}

void Gfx::flipSprites(Sprite *letterSprites, int count, std::vector<TilePalette> newPalettes, FlipOptions option) {
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

void Gfx::fadeIn() {
	for(int i = 16; i >= 0; i--) {
		setBrightness(3, i);
		swiWaitForVBlank();
	}
}

void Gfx::fadeOut() {
	for(int i = 0; i <= 16; i++) {
		setBrightness(3, i);
		swiWaitForVBlank();
	}
}
