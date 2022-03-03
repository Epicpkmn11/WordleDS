#include "settings.hpp"
#include "font.hpp"
#include "gfx.hpp"
#include "tonccpy.h"

#include "bgBottom.h"
#include "settingsBottom.h"

#include <array>
#include <algorithm>
#include <nds.h>

constexpr std::array<std::array<u16, 6>, 2> toggleMap = {{
	{ // Off
		0x00B4, 0x00B5, 0x00B6,
		0x08B4, 0x08B5, 0x08B6
	},
	{ // On
		0x003E, 0x003F, 0x0040,
		0x083E, 0x083F, 0x0840
	}
}};

constexpr std::array<std::array<u16, 3>, 2> togglePal = {{
	{ // Green
		0x32AD, 0x42F1, 0x6378
	},
	{ // Orange
		0x1DFD, 0x363D, 0x5B1E
	}
}};

void setToggle(int toggle, bool on) {
	int tile = 0;
	switch(toggle) {
		case 0:
			tile = 0x0BC;
			break;
		case 1:
			tile = 0x19C;
			break;
	}

	u16 *ptr = bgGetMapPtr(BG_SUB(0)) + tile;
	for(uint i = 0; i < toggleMap[on].size(); i++) {
		ptr[i / 3 * 32 + (i % 3)] = toggleMap[on][i];
	}
}

void settingsMenu(Config &config) {
	// Change to settings menu background
	swiWaitForVBlank();
	tonccpy(bgGetGfxPtr(BG_SUB(0)), settingsBottomTiles, settingsBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, settingsBottomPal, settingsBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), settingsBottomMap, settingsBottomMapLen);

	while(1) {
		tonccpy(BG_PALETTE_SUB + 0x1D, togglePal[config.altPalette()].data(), togglePal[config.altPalette()].size() * sizeof(u16));
		setSpritePalettes(config.altPalette());
		setToggle(0, config.hardMode());
		setToggle(1, config.altPalette());

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
				if(touch.py >= 41 && touch.py <= 54) {
					if(config.boardState().size() == 0) // Can't toggle mid-game
						config.hardMode(!config.hardMode());
				} else if(touch.py >= 97 && touch.py <= 110) {
					config.altPalette(!config.altPalette());
				}
			}
		}
	}

	config.save();

	// Restore normal BG and letterSprites
	swiWaitForVBlank();
	tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomTiles, bgBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, bgBottomPal, bgBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomMap, bgBottomMapLen);
}
