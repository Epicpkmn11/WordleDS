#include "howto.hpp"
#include "gfx.hpp"
#include "sprite.hpp"

#include "bgBottom.h"
#include "bgTop.h"
#include "howtoBottom.h"
#include "howtoTop.h"

#include <array>
#include <nds.h>
#include <string>

void howto() {
	// Change to howto menu background and hide letterSprites
	dmaCopy(howtoTopTiles, bgGetGfxPtr(BG(0)), howtoTopTilesLen);
	dmaCopy(howtoTopPal, BG_PALETTE, howtoTopPalLen);
	dmaCopy(howtoTopMap, bgGetMapPtr(BG(0)), howtoTopMapLen);

	dmaCopy(howtoBottomTiles, bgGetGfxPtr(BG_SUB(0)), howtoBottomTilesLen);
	dmaCopy(howtoBottomPal, BG_PALETTE_SUB, howtoBottomPalLen);
	dmaCopy(howtoBottomMap, bgGetMapPtr(BG_SUB(0)), howtoBottomMapLen);

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
	dmaCopy(bgTopTiles, bgGetGfxPtr(BG(0)), bgTopTilesLen);
	dmaCopy(bgTopPal, BG_PALETTE, bgTopPalLen);
	dmaCopy(bgTopMap, bgGetMapPtr(BG(0)), bgTopMapLen);

	dmaCopy(bgBottomTiles, bgGetGfxPtr(BG_SUB(0)), bgBottomTilesLen);
	dmaCopy(bgBottomPal, BG_PALETTE_SUB, bgBottomPalLen);
	dmaCopy(bgBottomMap, bgGetMapPtr(BG_SUB(0)), 32 * 24 * sizeof(u16));

	for(Sprite &sprite : letterSprites)
		sprite.visible(true);
	Sprite::update(true);
}
