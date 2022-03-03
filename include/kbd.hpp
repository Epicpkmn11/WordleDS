#ifndef KBD_HPP
#define KBD_HPP

#include "sprite.hpp"
#include "gfx.hpp"

#include <vector>

struct Key {
	int x, y;
	char16_t c;

	Key(int x, int y, char16_t c) : x(x), y(y), c(c) {}
};

class Kbd {
	std::vector<Sprite> _sprites;
	std::vector<u16 *> _gfx;
	u16 *_backspaceGfx;
	u16 *_enterGfx;
	bool _visible = false;

	std::vector<Key> _keys = {
		{30, 132, u'Q'}, {50, 132, u'W'}, {70, 132, u'E'}, {90, 132, u'R'}, {110, 132, u'T'}, {130, 132, u'Y'}, {150, 132, u'U'}, {170, 132, u'I'}, {190, 132, u'O'}, {210, 132, u'P'},
		{40, 152, u'A'}, {60, 152, u'S'}, {80, 152, u'D'}, {100, 152, u'F'}, {120, 152, u'G'}, {140, 152, u'H'}, {160, 152, u'J'}, {180, 152, u'K'}, {200, 152, u'L'},
		{4, 154, u'\n'}, {60, 172, u'Z'}, {80, 172, u'X'}, {100, 172, u'C'}, {120, 172, u'V'}, {140, 172, u'B'}, {160, 172, u'N'}, {180, 172, u'M'}, {220, 154, u'\b'}
	};

public:
	enum SpecialKey {
		NOKEY = 0xFFFF,
		ENTER = u'\n',
		BACKSPACE = u'\b'
	};

	Kbd(void);

	char16_t get(void);

	bool visible(void) { return _visible; }

	Kbd &show(void) { _visible = true; for(Sprite &sprite : _sprites) sprite.visible(true); Sprite::update(false); return *this; }
	Kbd &hide(void) { _visible = false; for(Sprite &sprite : _sprites) sprite.visible(false); Sprite::update(false); return *this; }

	TilePalette palette(char16_t c);
	Kbd &palette(char16_t c, TilePalette pal);
};

#endif // KBD_HPP
