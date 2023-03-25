// Simplified version of TWiLight Menu++'s text rendering

#ifndef FONT_HPP
#define FONT_HPP

#include <memory>
#include <nds/arm9/video.h>
#include <nds/ndstypes.h>
#include <string>

enum class Alignment {
	left,
	center,
	right,
};

class Font {
private:
	static u8 textBuf[2][SCREEN_WIDTH * SCREEN_HEIGHT];

	u8 tileWidth = 0, tileHeight = 0;
	u16 tileSize = 0;
	int tileAmount = 0;
	u16 questionMark = 0;
	u8 paletteStart = 0;
	const u8 *fontTiles = nullptr;
	const u8 *fontWidths = nullptr;
	std::unique_ptr<u8[]> fontTilesFile = nullptr;
	std::unique_ptr<u8[]> fontWidthsFile = nullptr;
	std::unique_ptr<u16[]> fontMap;

	u16 getCharIndex(char16_t c) const;

public:
	static std::u16string utf8to16(const std::string &text);
	static std::string utf16to8(const std::u16string &text);
	static std::string utf16to8(char16_t c);

	Font(const char *path, const u8 *fallback);
	Font(void) {}

	u8 height(void) const { return tileHeight; }

	int calcWidth(const std::string &text) const { return calcWidth(utf8to16(text)); }
	int calcWidth(const std::u16string &text) const;

	int calcHeight(const std::string &text) const { return calcHeight(utf8to16(text)); }
	int calcHeight(const std::u16string &text) const;

	Font &palette(int palette) { paletteStart = palette; return *this; }

	Font &print(int x, int y, bool top, int value, Alignment align = Alignment::left) { return print(x, y, top, std::to_string(value), align); }
	Font &print(int x, int y, bool top, const std::string &text, Alignment align = Alignment::left) { return print(x, y, top, utf8to16(text), align); }
	Font &print(int x, int y, bool top, std::u16string text, Alignment align = Alignment::left);

	static void clear(bool top);
	static void update(bool top, bool preserve = false);
};

#endif // FONT_HPP
