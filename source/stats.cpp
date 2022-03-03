#include "stats.hpp"
#include "defines.hpp"
#include "font.hpp"
#include "gfx.hpp"
#include "tonccpy.h"

#include "bgBottom.h"
#include "numbers_large_nftr.h"
#include "numbers_small_nftr.h"
#include "statsBottom.h"

#include <array>
#include <algorithm>
#include <nds.h>

constexpr std::array<std::array<u16, 12>, 2> fontPal = {{
	{
		0xFFFF, 0xDEF7, 0xC631, 0x8000, // Black
		0x39CE, 0xC631, 0xF39C, 0xFFFF, // White on gray
		0x32AD, 0xC2D1, 0xDF57, 0xFFFF, // White on green
	},
	{
		0xFFFF, 0xDEF7, 0xC631, 0x8000, // Black
		0x39CE, 0xC631, 0xF39C, 0xFFFF, // White on gray
		0x1DFD, 0xB63D, 0xD6DE, 0xFFFF, // White on orange
	}

}};

#define TEXT_BLACK 0xF0
#define TEXT_WHITE 0xF4
#define TEXT_GREEN 0xF8

void statsMenu(const Config &config, bool won) {
	// Change to stats menu background
	swiWaitForVBlank();
	tonccpy(bgGetGfxPtr(BG_SUB(0)), statsBottomTiles, statsBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, statsBottomPal, statsBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), statsBottomMap, statsBottomMapLen);

	// Loat fonts
	Font largeFont(numbers_large_nftr, numbers_large_nftr_size), smallFont(numbers_small_nftr, numbers_small_nftr_size);
	largeFont.palette(TEXT_BLACK);
	smallFont.palette(TEXT_BLACK);
	tonccpy(BG_PALETTE_SUB + 0xF0, fontPal[config.altPalette()].data(), fontPal[config.altPalette()].size() * sizeof(u16));

	// Print scores
	largeFont.print(-96, 32, false, config.gamesPlayed(), Alignment::center);
	largeFont.print(-32, 32, false, std::count_if(config.guessCounts().begin(), config.guessCounts().end(), [](int a) { return a <= MAX_GUESSES; }) * 100 / config.gamesPlayed(), Alignment::center);
	largeFont.print(32, 32, false, config.streak(), Alignment::center);
	largeFont.print(96, 32, false, config.maxStreak(), Alignment::center).update(false);

	// Draw guess percentage bars
	int highestCount = 0;
	for(int i = 1; i <= MAX_GUESSES; i++)
		highestCount = std::max(highestCount, std::count(config.guessCounts().begin(), config.guessCounts().end(), i));

	for(int i = 1; i <= MAX_GUESSES; i++) {
		int count = std::count(config.guessCounts().begin(), config.guessCounts().end(), i);
		int width = (10 + (216 * count / highestCount));
		u8 palette = (i == config.guessCounts().back() && won) ? TEXT_GREEN : TEXT_WHITE;
		u8 *dst = (u8 *)bgGetGfxPtr(BG_SUB(2)) + ((256 * (90 + (14 * i))) + 20);

		smallFont.print(8, 90 - 1 + i * 14, false, i);
		for(int j = 0; j < 12; j++) {
			int adjust = (j == 0 || j == 11) ? 1 : 0;
			toncset(dst + 256 * j + adjust, palette, width - adjust * 2);
		}

		smallFont.palette(palette).print(20 + width - 1, 90 - 1 + i * 14, false, count, Alignment::right).palette(TEXT_BLACK);
	}
	smallFont.update(false, true);

	u16 pressed;
	touchPosition touch;
	do {
		swiWaitForVBlank();
		scanKeys();
		pressed = keysDown();
		touchRead(&touch);
	} while(!((pressed & (KEY_A | KEY_B)) || ((pressed & KEY_TOUCH) && (touch.px > 232 && touch.py < 24))));

	largeFont.clear(false).update(false);

	// Restore normal BG and letterSprites
	swiWaitForVBlank();
	tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomTiles, bgBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, bgBottomPal, bgBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomMap, bgBottomMapLen);
}
