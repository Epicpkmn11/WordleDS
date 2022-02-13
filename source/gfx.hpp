#ifndef GFX_HPP
#define GFX_HPP

#include "sprite.hpp"

#include <vector>

#define WORD_LEN 5
#define MAX_GUESSES 6

static_assert(WORD_LEN > 0 && WORD_LEN <= 9, "WORD_LEN must be 1-9");
static_assert(MAX_GUESSES > 0 && MAX_GUESSES <= 6, "WORD_LEN must be 1-6");

enum TilePalette : int {
	white = 0,
	whiteDark = 1,
	gray = 2,
	yellow = 3,
	green = 4
};

extern std::vector<Sprite> sprites;
extern std::vector<u16 *> letterGfx;

void initGraphics(void);

#endif // GFX_HPP
