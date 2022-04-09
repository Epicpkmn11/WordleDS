#include "kbd.hpp"
#include "defines.hpp"

#include "backspaceKey.h"
#include "enterKey.h"
#include "kbdKeys.h"
#include "tonccpy.h"

#include <nds.h>

Kbd::Kbd() {
	constexpr int tileSize = 32 * 32 / 2;
	for(int i = 0; i < kbdKeysTilesLen / tileSize; i++) {
		_gfx.push_back(oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_16Color));
		tonccpy(_gfx.back(), kbdKeysTiles + (i * tileSize), tileSize);
	}

	_backspaceGfx = oamAllocateGfx(&oamSub, SpriteSize_64x32, SpriteColorFormat_16Color);
	tonccpy(_backspaceGfx, backspaceKeyTiles, backspaceKeyTilesLen);
	_enterGfx = oamAllocateGfx(&oamSub, SpriteSize_64x32, SpriteColorFormat_16Color);
	tonccpy(_enterGfx, enterKeyTiles, enterKeyTilesLen);

	for(Key &key : _keys) {
		_sprites.emplace_back(false, (key.c == u'\b' || key.c == u'\n') ? SpriteSize_64x32 : SpriteSize_32x32, SpriteColorFormat_16Color);
		_sprites.back().move(key.x, key.y).gfx(key.c == u'\b' ? _backspaceGfx : (key.c == u'\n' ? _enterGfx : _gfx[letterIndex(key.c)])).visible(false);
	}
	Sprite::update(false);
}

Kbd::~Kbd() {
	for(const u16 *gfx : _gfx) {
		oamFreeGfx(&oamSub, gfx);
	}
	oamFreeGfx(&oamSub, _backspaceGfx);
	oamFreeGfx(&oamSub, _enterGfx);
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
