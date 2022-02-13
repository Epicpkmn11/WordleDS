#include "sprite.hpp"

#include <nds.h>
#include <string>

s8 Sprite::activeIds[2][256];

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

	oamSet(_oam, _id, 0, 0, 0, 0, _size, _format, nullptr, _rotId, false, false, false, false, false);
}

Sprite::Sprite(const Sprite &sprite) : _top(sprite._top), _oam(sprite._oam), _id(sprite._id), _rotId(sprite._rotId), _size(sprite._size), _format(sprite._format) {
	activeIds[_top][_id]++;
}

Sprite::~Sprite() {
	activeIds[_top][_id]--;
}

Sprite &Sprite::operator=(const Sprite &sprite) {
	_top    = sprite._top;
	_oam    = sprite._oam;
	_id     = sprite._id;
	_rotId  = sprite._rotId;
	_size   = sprite._size;
	_format = sprite._format;

	activeIds[_top][_id]++;

	return *this;
}
