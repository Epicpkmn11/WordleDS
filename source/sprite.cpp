#include "sprite.hpp"

#include <nds/arm9/sprite.h>
#include <string>

s8 Sprite::activeIds[2][256] = {{0}, {0}};

Sprite::Sprite(bool top, SpriteSize size, SpriteColorFormat format) : _top(top), _oam(top ? &oamMain : &oamSub), _size(size), _format(format) {
	for(int i = 0; i < 256; i++) {
		if(!activeIds[_top][i]) {
			_id = i;
			activeIds[_top][_id]++;
			break;
		}
	}

	if(_id == -1)
		return;

	oamSet(_oam, _id, _x, _y, _priority, _paletteAlpha, _size, _format, nullptr, _affineIndex, _sizeDouble, !_visible, _hFlip, _vFlip, false);
}

Sprite::Sprite(const Sprite &sprite) :
	_top(sprite._top), _oam(sprite._oam), _id(sprite._id), _size(sprite._size), _format(sprite._format),
	_gfx(sprite._gfx), _affineIndex(sprite._affineIndex), _paletteAlpha(sprite._paletteAlpha),
	_priority(sprite._priority), _x(sprite._x), _y(sprite._y), _hFlip(sprite._hFlip), _vFlip(sprite._vFlip),
	_sizeDouble(sprite._sizeDouble), _visible(sprite._visible) {
	activeIds[_top][_id]++;
}

Sprite::~Sprite() {
	activeIds[_top][_id]--;
	if(activeIds[_top][_id] == 0) {
		oamClearSprite(_oam, _id);
		update();
	}
}

Sprite &Sprite::operator=(const Sprite &sprite) {
	_top    = sprite._top;
	_oam    = sprite._oam;
	_id     = sprite._id;
	_size   = sprite._size;
	_format = sprite._format;

	_gfx          = sprite._gfx;
	_affineIndex  = sprite._affineIndex;
	_paletteAlpha = sprite._paletteAlpha;
	_priority     = sprite._priority;
	_x            = sprite._x;
	_y            = sprite._y;
	_hFlip        = sprite._hFlip;
	_vFlip        = sprite._vFlip;
	_sizeDouble   = sprite._sizeDouble;
	_visible      = sprite._visible;

	activeIds[_top][_id]++;

	return *this;
}
