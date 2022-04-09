#ifndef GFX_HPP
#define GFX_HPP

#include "font.hpp"
#include "sprite.hpp"

#include <vector>

#define BG(x) (0 + x)
#define BG_SUB(x) (4 + x)

#define TEXT_BLACK 0xF0
#define TEXT_GRAY  0xF4
#define TEXT_WHITE 0xF8
#define TEXT_GREEN 0xFC

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
extern Font mainFont;

void initGraphics(void);

void setPalettes(bool altPalette);

void flipSprites(Sprite *letterSprites, int count, std::vector<TilePalette> newPalettes, FlipOptions option = FlipOptions::none);

#endif // GFX_HPP
