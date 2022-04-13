#include "settings.hpp"
#include "defines.hpp"
#include "font.hpp"
#include "gfx.hpp"
#include "sprite.hpp"
#include "tonccpy.h"
#include "version.hpp"

#include "bgBottom.h"
#include "settingsBottom.h"
#include "soundbank.h"
#include "toggleOn.h"
#include "toggleOff.h"

#include <algorithm>
#include <array>
#include <maxmod9.h>
#include <nds.h>

void settingsMenu(Config &config) {
	// Change to settings menu background
	swiWaitForVBlank();
	tonccpy(bgGetGfxPtr(BG_SUB(0)), settingsBottomTiles, settingsBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, settingsBottomPal, settingsBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), settingsBottomMap, settingsBottomMapLen);

	mainFont.palette(TEXT_GRAY);
	mainFont.print(4, 192 - 2 - mainFont.calcHeight(creditStr), false, creditStr);
	mainFont.print(256 - 4, 192 - 2 - mainFont.height(), false, VER_NUMBER, Alignment::right);
	mainFont.update(false);

	u16 *toggleOnGfx = oamAllocateGfx(&oamSub, SpriteSize_32x16, SpriteColorFormat_16Color);
	tonccpy(toggleOnGfx, toggleOnTiles, toggleOnTilesLen);
	u16 *toggleOffGfx = oamAllocateGfx(&oamSub, SpriteSize_32x16, SpriteColorFormat_16Color);
	tonccpy(toggleOffGfx, toggleOffTiles, toggleOffTilesLen);

	Sprite hardToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	hardToggle.move(224, 47);
	Sprite colorToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	colorToggle.move(224, 89);
	Sprite musicToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	musicToggle.move(224, 120);

	while(1) {
		setPalettes(config.altPalette());
		hardToggle
			.gfx(config.hardMode() ? toggleOnGfx : toggleOffGfx)
			.palette(config.hardMode() ? TilePalette::green : TilePalette::gray);
		colorToggle
			.gfx(config.altPalette() ? toggleOnGfx : toggleOffGfx)
			.palette(config.altPalette() ? TilePalette::green : TilePalette::gray);
		musicToggle
			.gfx(config.music() ? toggleOnGfx : toggleOffGfx)
			.palette(config.music() ? TilePalette::green : TilePalette::gray);
		Sprite::update(false);

		u16 pressed;
		touchPosition touch;
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			touchRead(&touch);
		} while(!(pressed & (KEY_B | KEY_TOUCH)));

		if(pressed & KEY_B) {
			break;
		}

		if(pressed & KEY_TOUCH) {
			if(touch.px > 232 && touch.py < 24) { // X
				break;
			} else if(touch.px >= 224 && touch.px <= 245) { // Toggle
				if(touch.py >= 47 && touch.py <= (47 + 13)) {
					if(config.boardState().size() == 0) // Can't toggle mid-game
						config.hardMode(!config.hardMode());
				} else if(touch.py >= 89 && touch.py <= (89 + 13)) {
					config.altPalette(!config.altPalette());
				} else if(touch.py >= 120 && touch.py <= (120 + 13)) {
					config.music(!config.music());
					if(config.music()) {
						mmStart(MOD_MUSIC, MM_PLAY_LOOP);
					} else {
						mmStop();
						mmStop(); // yes, this *is* needed
					}
				}
			}
		}
	}

	config.save();

	mainFont.clear(false).update(false);

	oamFreeGfx(&oamSub, toggleOnGfx);
	oamFreeGfx(&oamSub, toggleOffGfx);

	// Restore normal BG and letterSprites
	swiWaitForVBlank();
	tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomTiles, bgBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, bgBottomPal, bgBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomMap, bgBottomMapLen);
}
