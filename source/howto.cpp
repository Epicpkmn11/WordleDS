#include "howto.hpp"
#include "gfx.hpp"
#include "sprite.hpp"

#include "bgBottom.h"
#include "bgTop.h"
#include "howtoBottom.h"
#include "howtoTop.h"
#include "tonccpy.h"

#include <array>
#include <nds.h>
#include <string>

void howtoMenu() {
	// Change to howto menu background and hide letterSprites
	tonccpy(bgGetGfxPtr(BG(0)), howtoTopTiles, howtoTopTilesLen);
	tonccpy(BG_PALETTE, howtoTopPal, howtoTopPalLen);
	tonccpy(bgGetMapPtr(BG(0)), howtoTopMap, howtoTopMapLen);

	tonccpy(bgGetGfxPtr(BG_SUB(0)), howtoBottomTiles, howtoBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, howtoBottomPal, howtoBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), howtoBottomMap, howtoBottomMapLen);

	for(Sprite &sprite : letterSprites)
		sprite.visible(false);
	Sprite::update(true);

	const std::string words = "weary" "pills" "vague";
	std::vector<Sprite> howtoSprites;
	for(uint i = 0; i < words.length(); i++) {
		howtoSprites.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
		howtoSprites.back().xy(1 + (i % 5) * 26, 22 + (i / 5 * 60)).gfx(letterGfxSub[words[i] - 'a' + 1]);
	}

	std::array<Sprite, 3> toFlip = {howtoSprites[0], howtoSprites[6], howtoSprites[13]};
	flipSprites(toFlip.data(), toFlip.size(), {TilePalette::green, TilePalette::yellow, TilePalette::gray});

	u16 pressed;
	touchPosition touch;
	do {
		swiWaitForVBlank();
		scanKeys();
		pressed = keysDown();
		touchRead(&touch);
	} while(!((pressed & (KEY_A | KEY_B)) || ((pressed & KEY_TOUCH) && (touch.px > 232 && touch.py < 24))));

	// Restore normal BG and letterSprites
	tonccpy(bgGetGfxPtr(BG(0)), bgTopTiles, bgTopTilesLen);
	tonccpy(BG_PALETTE, bgTopPal, bgTopPalLen);
	tonccpy(bgGetMapPtr(BG(0)), bgTopMap, bgTopMapLen);

	tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomTiles, bgBottomTilesLen);
	tonccpy(BG_PALETTE_SUB, bgBottomPal, bgBottomPalLen);
	tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomMap, SCREEN_SIZE_TILES);

	for(Sprite &sprite : letterSprites)
		sprite.visible(true);
	Sprite::update(true);
}
