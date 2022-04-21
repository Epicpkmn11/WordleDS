#include "kbd.hpp"
#include "game.hpp"

#include "tonccpy.h"

#include <nds.h>

Kbd::Kbd(const std::vector<Key> &keys, const std::vector<char16_t> &letters, const std::vector<OamGfx> &gfx, const OamGfx &backspaceGfx, const OamGfx &enterGfx) : _keys(keys), _letters(letters) {
	for(const Key &key : _keys) {
		char str[64];
		sprintf(str, "key %c at %d:%d", key.c, key.x, key.y);
		_sprites.emplace_back(false, (key.c == u'\b' || key.c == u'\n') ? SpriteSize_64x32 : SpriteSize_32x32, SpriteColorFormat_16Color);
		_sprites.back()
			.move(key.x, key.y)
			.gfx(key.c == u'\b' ? backspaceGfx : (key.c == u'\n' ? enterGfx : gfx[letterIndex(key.c)]))
			.visible(false);
	}
	Sprite::update(false);
}

char16_t Kbd::get() {
	if(!_visible)
		return SpecialKey::NOKEY;

	touchPosition touch;
	touchRead(&touch);

	for(const Key &key : _keys) {
		int size = (key.c == u'\b' || key.c == u'\n') ? 35 : 23;
		if(touch.px >= key.x && touch.px < key.x + size && touch.py >= key.y && touch.py < key.y + 23)
			return key.c;
	}

	return SpecialKey::NOKEY;
}

int Kbd::letterIndex(char16_t c) const {
	const auto out = std::find(_letters.begin(), _letters.end(), c);

	return out != _letters.end() ? std::distance(_letters.begin(), out) : 0;
}

TilePalette Kbd::palette(char16_t c) {
	for(uint i = 0; i < _keys.size(); i++) {
		if(_keys[i].c == c) {
			return TilePalette(_sprites[i].paletteAlpha());
		}
	}

	return TilePalette::white;
}

Kbd &Kbd::palette(char16_t c, TilePalette pal) {
	for(uint i = 0; i < _keys.size(); i++) {
		if(_keys[i].c == c) {
			_sprites[i].palette(pal);
			break;
		}
	}

	return *this;
}
