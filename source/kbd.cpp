#include "kbd.hpp"

#include "backspaceKey.h"
#include "enterKey.h"
#include "kbdKeys.h"
#include "tonccpy.h"

#include <nds.h>

Kbd::Kbd() {
	constexpr int tileSize = 16 * 16 / 2;
	for(int i = 0; i < kbdKeysTilesLen / tileSize; i++) {
		_gfx.push_back(oamAllocateGfx(&oamSub, SpriteSize_16x16, SpriteColorFormat_16Color));
		tonccpy(_gfx.back(), kbdKeysTiles + (i * tileSize), tileSize);
	}
	_gfx.push_back(oamAllocateGfx(&oamSub, SpriteSize_32x16, SpriteColorFormat_16Color));
	tonccpy(_gfx.back(), backspaceKeyTiles, backspaceKeyTilesLen);
	_gfx.push_back(oamAllocateGfx(&oamSub, SpriteSize_32x16, SpriteColorFormat_16Color));
	tonccpy(_gfx.back(), enterKeyTiles, enterKeyTilesLen);
	for(Key &key : _keys) {
		_sprites.emplace_back(false, (key.c == '\b' || key.c == '\n') ? SpriteSize_32x16 : SpriteSize_16x16, SpriteColorFormat_16Color);
		_sprites.back().xy(key.x, key.y).gfx(_gfx[key.c == '\b' ? 26 : (key.c == '\n' ? 27 : key.c - 'a')]).visible(false);
	}
	Sprite::update(false);
}

char Kbd::get() {
	if(!_visible)
		return SpecialKey::NOKEY;

	touchPosition touch;
	touchRead(&touch);

	for(const Key &key : _keys) {
		if(touch.px >= key.x && touch.px < key.x + 16 && touch.py >= key.y && touch.py < key.y + 16)
			return key.c;
	}

	return SpecialKey::NOKEY;
}

Kbd &Kbd::palette(char c, TilePalette pal) {
	for(uint i = 0; i < _keys.size(); i++) {
		if(_keys[i].c == c) {
			_sprites[i].palette(pal);
		}
	}

	return *this;
}
