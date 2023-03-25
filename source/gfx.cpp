#include "gfx.hpp"
#include "game.hpp"
#include "tonccpy.h"

#include <array>
#include <nds.h>
#include <stdio.h>

static bool fadedOut = true;
static int popupTimeout = -1;

static void vblankHandler() {
	if(popupTimeout == 0)
		Gfx::hidePopup();
	if(popupTimeout >= 0)
		popupTimeout--;
}

void Gfx::init() {
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(BG(0), 3);

	// https://mtheall.com/vram.html#T0=1&NT0=512&MB0=30&TB0=0&S0=0&T1=2&NT1=64&MB1=31&TB1=4&S1=0&T2=5&MB2=5&S2=1
	bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 30, 0);
	bgSetPriority(BG_SUB(0), 3);
	bgInitSub(1, BgType_Text4bpp, BgSize_T_256x256, 31, 4);
	bgSetPriority(BG_SUB(1), 2);
	bgHide(BG_SUB(1));
	bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 5, 0);
	bgSetPriority(BG_SUB(2), 0);

	bgUpdate();

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);

	irqSet(IRQ_VBLANK, vblankHandler);
}

void Gfx::showPopup(const std::string &msg, int timeout) {
	if(game) {
		Font::clear(false);
		game->data().mainFont().palette(TEXT_WHITE).print(0, 56 - game->data().mainFont().calcHeight(msg) / 2, false, msg, Alignment::center);
		Font::update(false);
	}

	bgShow(BG_SUB(1));
	bgUpdate();

	popupTimeout = timeout;
}

void Gfx::hidePopup() {
	bgHide(BG_SUB(1));
	Font::clear(false);
	Font::update(false);
	popupTimeout = -1;
}

bool Gfx::popupVisible() {
	return popupTimeout != -1;
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

void Gfx::fadeIn(int frames, int screen) {
	if(!fadedOut)
		return;
	else
		fadedOut = false;

	for(int i = 0; i < frames; i++) {
		setBrightness(screen, 16 - (i * 16 / frames));
		swiWaitForVBlank();
	}
	setBrightness(screen, 0); // Ensure fully faded in
}

void Gfx::fadeOut(int frames, int screen) {
	if(fadedOut)
		return;
	else
		fadedOut = true;

	for(int i = 0; i < frames; i++) {
		setBrightness(screen, (i * 16 / frames));
		swiWaitForVBlank();
	}
	setBrightness(screen, 16); // Ensure fully faded out

	// Hide the popup if it was shown
	hidePopup();
}
