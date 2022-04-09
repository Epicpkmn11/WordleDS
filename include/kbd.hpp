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
		{4,  114,  u'Q'}, {29, 114, u'W'}, {54, 114, u'E'}, {79, 114, u'R'}, {104, 114, u'T'}, {129, 114, u'Y'}, {154, 114, u'U'}, {179, 114, u'I'}, {204, 114,  u'O'}, {229, 114, u'P'},
		{16, 140,  u'A'}, {41, 140, u'S'}, {66, 140, u'D'}, {91, 140, u'F'}, {116, 140, u'G'}, {141, 140, u'H'}, {166, 140, u'J'}, {191, 140, u'K'}, {216, 140,  u'L'},
		{4,  166, u'\n'}, {41, 166, u'Z'}, {66, 166, u'X'}, {91, 166, u'C'}, {116, 166, u'V'}, {141, 166, u'B'}, {166, 166, u'N'}, {191, 166, u'M'}, {216, 166, u'\b'}
	};

public:
	enum SpecialKey {
		NOKEY = 0xFFFF,
		ENTER = u'\n',
		BACKSPACE = u'\b'
	};

	Kbd(void);
	~Kbd(void);

	char16_t get(void);

	bool visible(void) { return _visible; }

	Kbd &show(void) { _visible = true; for(Sprite &sprite : _sprites) sprite.visible(true); Sprite::update(false); return *this; }
	Kbd &hide(void) { _visible = false; for(Sprite &sprite : _sprites) sprite.visible(false); Sprite::update(false); return *this; }

	TilePalette palette(char16_t c);
	Kbd &palette(char16_t c, TilePalette pal);
};

#endif // KBD_HPP
