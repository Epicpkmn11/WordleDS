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
	bool _visible = false;
	const std::vector<Key> &_keys;
	const std::vector<char16_t> &_letters;

public:
	enum SpecialKey {
		NOKEY = 0xFFFF,
		ENTER = u'\n',
		BACKSPACE = u'\b'
	};

	Kbd(const std::vector<Key> &keys, const std::vector<char16_t> &letters, const std::vector<OamGfx> &gfx, const OamGfx &backspaceGfx, const OamGfx &enterGfx);

	char16_t get(void);

	int letterIndex(char16_t c) const;

	bool visible(void) { return _visible; }

	Kbd &show(void) { _visible = true; for(Sprite &sprite : _sprites) sprite.visible(true); Sprite::update(false); return *this; }
	Kbd &hide(void) { _visible = false; for(Sprite &sprite : _sprites) sprite.visible(false); Sprite::update(false); return *this; }

	TilePalette palette(char16_t c);
	Kbd &palette(char16_t c, TilePalette pal);
};

#endif // KBD_HPP
