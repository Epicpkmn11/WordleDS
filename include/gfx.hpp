#ifndef GFX_HPP
#define GFX_HPP

#include "sprite.hpp"

#include <vector>

#define BG(x) (0 + x)
#define BG_SUB(x) (4 + x)

enum TilePalette : int {
	white = 0,
	whiteDark = 1,
	gray = 2,
	yellow = 3,
	green = 4
};

enum class FlipOptions {
	none,
	hide, // hide after the first half of the animation
	show // unhide after the first half of the animation
};

extern std::vector<Sprite> letterSprites;
extern std::vector<u16 *> letterGfx, letterGfxSub;

void initGraphics(bool altPalette);

void setSpritePalettes(bool altPalette);

void flipSprites(Sprite *letterSprites, int count, std::vector<TilePalette> newPalettes, FlipOptions option = FlipOptions::none);

#endif // GFX_HPP
