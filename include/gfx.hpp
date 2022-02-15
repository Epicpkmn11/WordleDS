#ifndef GFX_HPP
#define GFX_HPP

#include "sprite.hpp"

#include <vector>

#define WORD_LEN 5
#define MAX_GUESSES 6

#define SCREEN_SIZE (256 * 192)
#define SCREEN_SIZE_TILES (32 * 24 * sizeof(u16))

static_assert(WORD_LEN > 0 && WORD_LEN <= 9, "WORD_LEN must be 1-9");
static_assert(MAX_GUESSES > 0 && MAX_GUESSES <= 6, "WORD_LEN must be 1-6");

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

void flipSprites(Sprite *letterSprites, int count, std::vector<TilePalette> newPalettes, FlipOptions option = FlipOptions::none);

#endif // GFX_HPP
