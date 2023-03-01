#include "howto.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "sprite.hpp"

#include "tonccpy.h"

#include <array>
#include <nds.h>
#include <string>

void howtoMenu(bool slowFade) {
	// Change to howto menu background and hide letterSprites
	game->data().howtoTop().decompressAll(bgGetGfxPtr(BG(0)), bgGetMapPtr(BG(0)), BG_PALETTE);
	game->data().howtoBottom().decompressAll(bgGetGfxPtr(BG_SUB(0)), bgGetMapPtr(BG_SUB(0)), BG_PALETTE_SUB);

	for(Sprite &sprite : game->letterSprites())
		sprite.visible(false);
	Sprite::update(true);

	std::vector<Sprite> howtoSprites;
	for(size_t i = 0; i < game->data().howtoWords().size(); i++) {
		for(size_t j = 0; j < game->data().howtoWords(i).size(); j++) {
			howtoSprites.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
			howtoSprites.back()
				.move(1 + j * 26, 22 + i * 60)
				.gfx(game->data().letterGfxSub(game->kbd().letterIndex(game->data().howtoWords(i)[j]) + 1))
				.palette(TilePalette::whiteDark);
		}
	}

	Sprite::update(false);
	Gfx::fadeIn(slowFade ? FADE_SLOW : FADE_FAST, FADE_TOP | FADE_BOTTOM);

	std::vector<Sprite> toFlip;
	std::vector<TilePalette> flipColors;
	for(size_t i = 0; i < game->data().howtoColors().size(); i++) {
		if(game->data().howtoColors(i) != TilePalette::whiteDark) {
			toFlip.push_back(howtoSprites[i]);
			flipColors.push_back(game->data().howtoColors(i));
		}
	}
	Gfx::flipSprites(toFlip.data(), toFlip.size(), flipColors);

	u16 pressed;
	touchPosition touch;
	do {
		swiWaitForVBlank();
		scanKeys();
		pressed = keysDown();
		touchRead(&touch);
	} while(!((pressed & (KEY_A | KEY_B)) || ((pressed & KEY_TOUCH) && (touch.px > 232 && touch.py < 24))));

	Gfx::fadeOut(FADE_FAST, FADE_TOP | FADE_BOTTOM);

	// Restore letterSprites
	for(Sprite &sprite : game->letterSprites())
		sprite.visible(true);
	Sprite::update(true);
}
