#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <nds/arm9/sprite.h>

#define FLOAT_TO_FIXED(x) (x == 0.0f ? 0 : int((1.0f / x) * 256.0f))

class Sprite {
	static s8 activeIds[2][256];

	bool _top;
	OamState *_oam;
	int _id = -1;
	int _rotId = -1;
	bool _visible = true;
	SpriteSize _size;
	SpriteColorFormat _format;

public:
	Sprite(bool top, SpriteSize size, SpriteColorFormat format);
	Sprite(const Sprite &sprite);
	~Sprite(void);

	Sprite &operator=(const Sprite &sprite);

	static void update(bool top) { oamUpdate(top ? &oamMain : &oamSub); }
	Sprite &update(void) { oamUpdate(_oam); return *this; }

	int id(void) const { return _id; }
	bool visible(void) const { return _visible; }

	Sprite &affineIndex(int affineIndex, bool sizeDouble) { _rotId = affineIndex; oamSetAffineIndex(_oam, _id, _rotId, sizeDouble); return *this; }
	Sprite &affineTransform(float hdx, float hdy, float vdx, float vdy) { if(_rotId >= 0 && _rotId <= 31) oamAffineTransformation(_oam, _rotId, FLOAT_TO_FIXED(hdx), FLOAT_TO_FIXED(hdy), FLOAT_TO_FIXED(vdx), FLOAT_TO_FIXED(vdy)); return *this; }
	Sprite &alpha(int alpha) { oamSetAlpha(_oam, _id, alpha); return *this; }
	Sprite &flip(bool hflip, bool vflip) { oamSetFlip(_oam, _id, hflip, vflip); return *this; }
	Sprite &gfx(u16 *gfx) { oamSetGfx(_oam, _id, _size, _format, gfx); return *this; }
	Sprite &palette(int palette) { oamSetPalette(_oam, _id, palette); return *this; }
	Sprite &priority(int priority) { oamSetPriority(_oam, _id, priority); return *this; }
	Sprite &rotateScale(int angle, float sx, float sy) { if(_rotId >= 0 && _rotId <= 31) oamRotateScale(_oam, _rotId, angle, FLOAT_TO_FIXED(sx), FLOAT_TO_FIXED(sy)); return *this; }
	Sprite &visible(bool visible) { _visible = visible; oamSetHidden(_oam, _id, !visible); return *this; }
	Sprite &xy(int x, int y) { oamSetXY(_oam, _id, x, y); return *this; }
};

#endif // SPRITE_HPP
