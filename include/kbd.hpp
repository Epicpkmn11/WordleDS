#ifndef KBD_HPP
#define KBD_HPP

#include "sprite.hpp"
#include "gfx.hpp"

#include <vector>

struct Key {
	int x, y;
	char c;

	Key(int x, int y, char c) : x(x), y(y), c(c) {}
};

class Kbd {
	std::vector<Sprite> _sprites;
	std::vector<u16 *> _gfx;
	bool _visible = false;

	std::vector<Key> _keys = {
		{30, 132, 'q'}, {50, 132, 'w'}, {70, 132, 'e'}, {90, 132, 'r'}, {110, 132, 't'}, {130, 132, 'y'}, {150, 132, 'u'}, {170, 132, 'i'}, {190, 132, 'o'}, {210, 132, 'p'},
		{40, 152, 'a'}, {60, 152, 's'}, {80, 152, 'd'}, {100, 152, 'f'}, {120, 152, 'g'}, {140, 152, 'h'}, {160, 152, 'j'}, {180, 152, 'k'}, {200, 152, 'l'},
		{35, 172, '\n'}, {60, 172, 'z'}, {80, 172, 'x'}, {100, 172, 'c'}, {120, 172, 'v'}, {140, 172, 'b'}, {160, 172, 'n'}, {180, 172, 'm'}, {200, 172, '\b'}
	};

public:
	enum SpecialKey {
		NOKEY = -1,
		ENTER = '\n',
		BACKSPACE = '\b'
	};

	Kbd(void);

	char get(void);

	bool visible(void) { return _visible; }

	Kbd &show(void) { _visible = true; for(Sprite &sprite : _sprites) sprite.visible(true); Sprite::update(false); return *this; }
	Kbd &hide(void) { _visible = false; for(Sprite &sprite : _sprites) sprite.visible(false); Sprite::update(false); return *this; }

	TilePalette palette(char c);
	Kbd &palette(char c, TilePalette pal);
};

#endif // KBD_HPP
