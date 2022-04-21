#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <memory>
#include <nds/arm9/sprite.h>

#define FLOAT_TO_FIXED(x) (x == 0.0f ? 0 : int((1.0f / x) * 256.0f))

class OamGfx {
	OamState *_oam;
	std::shared_ptr<u16> _gfx = nullptr;

public:
	OamGfx(bool top, SpriteSize size, SpriteColorFormat colorFormat)
			: _oam(top ? &oamMain : &oamSub), _gfx(std::shared_ptr<u16>(oamAllocateGfx(_oam, size, colorFormat), [this](u16 *p) { oamFreeGfx(_oam, p); })) {}
	OamGfx(void) {}

	u16 *get(void) { return _gfx.get(); }
};

class Sprite {
	static s8 activeIds[2][256];

	bool _top;
	OamState *_oam;
	int _id = -1;
	SpriteSize _size;
	SpriteColorFormat _format;

	OamGfx _gfx;
	int _affineIndex = -1;
	int _paletteAlpha = 0;
	int _priority = 0;
	int _x = 0, _y = 0;
	bool _hFlip = false, _vFlip = false;
	bool _sizeDouble = false;
	bool _visible = true;

public:
	Sprite(bool top, SpriteSize size, SpriteColorFormat format);
	Sprite(const Sprite &sprite);
	~Sprite(void);

	Sprite &operator=(const Sprite &sprite);

	static void update(bool top) { oamUpdate(top ? &oamMain : &oamSub); }
	Sprite &update(void) { oamUpdate(_oam); return *this; }

	int id(void) const { return _id; }

	OamGfx gfx(void) const { return _gfx; }
	int affineIndex(void) const { return _affineIndex; }
	int paletteAlpha(void) const { return _paletteAlpha; }
	int priority(void) const { return _priority; }
	int x(void) const { return _x; }
	int y(void) const { return _y; }
	bool hFlip(void) const { return _hFlip; }
	bool vFlip(void) const { return _vFlip; }
	bool sizeDouble(void) const { return _sizeDouble; }
	bool visible(void) const { return _visible; }

	Sprite &affineIndex(int affineIndex, bool sizeDouble) { _affineIndex = affineIndex, _sizeDouble = sizeDouble; oamSetAffineIndex(_oam, _id, _affineIndex, _sizeDouble); return *this; }
	Sprite &affineTransform(float hdx, float hdy, float vdx, float vdy) { if(_affineIndex >= 0 && _affineIndex <= 31) oamAffineTransformation(_oam, _affineIndex, FLOAT_TO_FIXED(hdx), FLOAT_TO_FIXED(hdy), FLOAT_TO_FIXED(vdx), FLOAT_TO_FIXED(vdy)); return *this; }
	Sprite &alpha(int alpha) { _paletteAlpha =  alpha; oamSetAlpha(_oam, _id, _paletteAlpha); return *this; }
	Sprite &flip(bool hFlip, bool vFlip) { _hFlip = hFlip, _vFlip = vFlip; oamSetFlip(_oam, _id, _hFlip, _vFlip); return *this; }
	Sprite &gfx(const OamGfx &gfx) { _gfx = gfx; oamSetGfx(_oam, _id, _size, _format, _gfx.get()); return *this; }
	Sprite &move(int x, int y) { _x = x, _y = y; oamSetXY(_oam, _id, _x, _y); return *this; }
	Sprite &palette(int palette) { _paletteAlpha = palette; oamSetPalette(_oam, _id, _paletteAlpha); return *this; }
	Sprite &priority(int priority) { _priority = priority; oamSetPriority(_oam, _id, _priority); return *this; }
	Sprite &rotateScale(int angle, float sx, float sy) { if(_affineIndex >= 0 && _affineIndex <= 31) oamRotateScale(_oam, _affineIndex, angle, FLOAT_TO_FIXED(sx), FLOAT_TO_FIXED(sy)); return *this; }
	Sprite &visible(bool visible) { _visible = visible; oamSetHidden(_oam, _id, !visible); return *this; }
};

#endif // SPRITE_HPP
