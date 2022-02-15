// Simplified version of TWiLight Menu++'s text rendering

#ifndef FONT_HPP
#define FONT_HPP

#include "gfx.hpp"

#include <nds/ndstypes.h>
#include <string>
#include <string_view>

enum class Alignment {
	left,
	center,
	right,
};

class Font {
private:
	static u8 textBuf[2][SCREEN_SIZE];

	u8 tileWidth = 0, tileHeight = 0;
	u16 tileSize = 0;
	int tileAmount = 0;
	u16 questionMark = 0;
	u8 paletteStart = 0;
	u8 *fontTiles = nullptr;
	u8 *fontWidths = nullptr;
	u16 *fontMap = nullptr;

	u16 getCharIndex(char16_t c);

public:
	static std::u16string utf8to16(std::string_view text);

	Font(const u8 *nftr, u32 nftrSize);

	~Font(void);

	u8 height(void) { return tileHeight; }

	int calcWidth(std::string_view text) { return calcWidth(utf8to16(text)); }
	int calcWidth(std::u16string_view text);

	Font &palette(int palette) { paletteStart = palette; return *this; }

	Font &print(int x, int y, bool top, int value, Alignment align = Alignment::left) { return print(x, y, top, std::to_string(value), align); }
	Font &print(int x, int y, bool top, std::string_view text, Alignment align = Alignment::left) { return print(x, y, top, utf8to16(text), align); }
	Font &print(int x, int y, bool top, std::u16string_view text, Alignment align = Alignment::left);

	Font &clear(bool top);
	Font &update(bool top, bool preserve = false);
};

#endif // FONT_HPP
