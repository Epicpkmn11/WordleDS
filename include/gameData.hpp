#ifndef GAME_DATA_HPP
#define GAME_DATA_HPP

#include "button.hpp"
#include "image.hpp"
#include "json.hpp"
#include "kbd.hpp"
#include "sprite.hpp"

#define SETTINGS_JSON_OLD "WordleDS.json"

#define DEFAULT_MOD "Wordle DS"
#define DATA_PATH "/_nds/WordleDS/"
#define SETTINGS_JSON DATA_PATH "settings.json"
#define STATS_JSON "/stats.json"
#define MOD_JSON "/mod.json"

#include <map>
#include <string>
#include <vector>

class GameData {
	int _maxGuesses = 6,
		_firstDay = 18797; // June 19th 2021


	std::string
		_shareName = "Wordle DS",
		_lossMessage = "Better luck tomorrow...\nThe answer was:",
		_tooShortMessage = "Not enough letters",
		_notWordMessage = "Not in word list",
		_creditStr = "Wordle DS\nby Pk11",
		_nthMustBeX = "%d%s letter must be %s",
		_guessMustContainX = "Guess must contain %s",
		_shareTime = " %02d:%02d",
		_shareTimeHour = " %02d:%02d:%02d",
		_shareStreak = " 📈%d",
		_shareStreakLoss = " 📉%d",
		_emojiGreen = "🟩",
		_emojiGreenAlt = "🟧",
		_emojiYellow = "🟨",
		_emojiYellowAlt = "🟦",
		_emojiWhite = "⬜";

	Button _hardModeToggle = {224, 33, 21, 13};
	Button _infiniteModeToggle = { 224, 55, 21, 13 };
	Button _highContrastToggle = {224, 68, 21, 13};
	Button _musicToggle = {224, 92, 21, 13};
	Button _shareMsgBtn = {232, 108, 17, 17};
	Button _modBtn = {232, 131, 17, 17};

	Button _shareTimerToggle = {224, 37, 21, 13};
	Button _shareStreakToggle = {224, 82, 21, 13};
	Button _shareUrlToggle = {224, 128, 21, 13};

	bool _oldStatsMenu = false;

	std::vector<std::u16string> _howtoWords = {
		u"WEARY",
		u"PILLS",
		u"VAGUE"
	};

	std::vector<TilePalette> _howtoColors = {
		TilePalette::green, TilePalette::whiteDark, TilePalette::whiteDark, TilePalette::whiteDark, TilePalette::whiteDark,
		TilePalette::whiteDark, TilePalette::yellow, TilePalette::whiteDark, TilePalette::whiteDark, TilePalette::whiteDark,
		TilePalette::whiteDark, TilePalette::whiteDark, TilePalette::whiteDark, TilePalette::gray, TilePalette::whiteDark
	};

	std::vector<std::string> _victoryMessages = {
		"Genius",
		"Magnificent",
		"Impressive",
		"Splendid",
		"Great",
		"Phew"
	};

	std::map<int, std::string> _numberSuffixes = {
		{1, "st"},
		{2, "nd"},
		{3, "rd"},
		{0xFFFF, "th"}
	};

	std::vector<char16_t> _letters = {
		u'A',
		u'B',
		u'C',
		u'D',
		u'E',
		u'F',
		u'G',
		u'H',
		u'I',
		u'J',
		u'K',
		u'L',
		u'M',
		u'N',
		u'O',
		u'P',
		u'Q',
		u'R',
		u'S',
		u'T',
		u'U',
		u'V',
		u'W',
		u'X',
		u'Y',
		u'Z'
	};

	std::vector<Key> _keyboard = {
		{4,  114,  u'Q'}, {29, 114, u'W'}, {54, 114, u'E'}, {79, 114, u'R'}, {104, 114, u'T'}, {129, 114, u'Y'}, {154, 114, u'U'}, {179, 114, u'I'}, {204, 114,  u'O'}, {229, 114, u'P'},
		{16, 140,  u'A'}, {41, 140, u'S'}, {66, 140, u'D'}, {91, 140, u'F'}, {116, 140, u'G'}, {141, 140, u'H'}, {166, 140, u'J'}, {191, 140, u'K'}, {216, 140,  u'L'},
		{4,  166, u'\n'}, {41, 166, u'Z'}, {66, 166, u'X'}, {91, 166, u'C'}, {116, 166, u'V'}, {141, 166, u'B'}, {166, 166, u'N'}, {191, 166, u'M'}, {216, 166, u'\b'}
	};

	std::vector<std::vector<u16>> _letterPalettes = {
		{
			0x0000, 0x6B5A, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C,
			0x0000, 0x4631, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C,
			0x0000, 0x39CE, 0x39CE, 0x7FFF, 0x77DD, 0x739C, 0x6F7B, 0x6B5A, 0x6739, 0x6318, 0x5AD6, 0x56B5, 0x4E94, 0x4A52, 0x4631, 0x4210,
			0x0000, 0x2ED8, 0x2ED8, 0x7FFF, 0x77DE, 0x73BD, 0x6B9C, 0x677C, 0x5F5B, 0x5B5B, 0x573A, 0x4F3A, 0x4B1A, 0x42F9, 0x3AF9, 0x32D8,
			0x0000, 0x32AD, 0x32AD, 0x7FFF, 0x77DD, 0x73BC, 0x6F9B, 0x6B7A, 0x6359, 0x5F57, 0x5B36, 0x5314, 0x4AF3, 0x42D1, 0x3AAE, 0x36AD
		},
		{
			0x0000, 0x6B5A, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C,
			0x0000, 0x4631, 0x7FFF, 0x0000, 0x0C63, 0x14A5, 0x1CE7, 0x2529, 0x2D6B, 0x35AD, 0x4210, 0x4E94, 0x56B5, 0x6318, 0x6B5A, 0x739C,
			0x0000, 0x39CE, 0x39CE, 0x7FFF, 0x77DD, 0x739C, 0x6F7B, 0x6B5A, 0x6739, 0x6318, 0x5AD6, 0x56B5, 0x4E94, 0x4A52, 0x4631, 0x4210,
			0x0000, 0x7AF0, 0x7AF0, 0x7FFF, 0x7FDE, 0x7BDD, 0x7BBC, 0x7BBB, 0x7B9A, 0x7B99, 0x7B78, 0x7B57, 0x7B56, 0x7B35, 0x7B13, 0x7B12,
			0x0000, 0x1DFD, 0x1DFD, 0x7FFF, 0x7BDF, 0x739E, 0x6B7E, 0x635E, 0x5B1E, 0x56DE, 0x4EBD, 0x469D, 0x427D, 0x363D, 0x2A1D, 0x21FD
		}
	};

	std::vector<std::vector<u16>> _fontPalettes = {
		{
			0xFFFF, 0xDEF7, 0xC631, 0x8000,
			0x0000, 0xEF7B, 0xD6B5, 0xC631,
			0xFFFF, 0xDF57, 0xC2D1, 0x32AD,
			0x39CE, 0xC631, 0xF39C, 0xFFFF,
			0x32AD, 0xC2D1, 0xDF57, 0xFFFF
		},
		{
			0xFFFF, 0xDEF7, 0xC631, 0x8000,
			0x0000, 0xEF7B, 0xD6B5, 0xC631,
			0xFFFF, 0xD6DE, 0xB63D, 0x1DFD,
			0x39CE, 0xC631, 0xF39C, 0xFFFF,
			0x1DFD, 0xB63D, 0xD6DE, 0xFFFF
		}
	};

	std::vector<std::u16string> _choices, _guesses;

	Image
		_bgBottom,
		_bgBottomBox,
		_bgTop,
		_howtoBottom,
		_howtoTop,
		_modsBottom,
		_settingsBottom,
		_shareMsgSettings,
		_statsBottom;

	OamGfx
		_backspaceKeyGfx,
		_enterKeyGfx,
		_refreshGfx,
		_toggleOffGfx,
		_toggleOnGfx;

	std::vector<OamGfx> _letterGfx, _letterGfxSub, _kbdGfx;

	Sprite _refreshSprite = Sprite(false, SpriteSize_64x64, SpriteColorFormat_16Color);

	Font _mainFont, _numbersLarge, _numbersSmall;

	// size is used for sanity checking
	static std::vector<u16> getPalette(const Json &json, int size);

public:
	GameData(const std::string &folder);

	int maxGuesses(void) const { return _maxGuesses; }
	int firstDay(void) const { return _firstDay; }

	const std::vector<std::u16string> &howtoWords(void) const { return _howtoWords; }
	const std::u16string &howtoWords(size_t i) const { return _howtoWords[i]; }
	const std::vector<TilePalette> &howtoColors(void) const { return _howtoColors; }
	const TilePalette &howtoColors(size_t i) const { return _howtoColors[i]; }

	const std::string &shareName(void) const { return _shareName; }
	const std::string &lossMessage(void) const { return _lossMessage; }
	const std::string &tooShortMessage(void) const { return _tooShortMessage; }
	const std::string &notWordMessage(void) const { return _notWordMessage; }
	const std::string &creditStr(void) const { return _creditStr; }
	const std::string &nthMustBeX(void) const { return _nthMustBeX; }
	const std::string &guessMustContainX(void) const { return _guessMustContainX; }
	const std::string &shareTime(void) const { return _shareTime; }
	const std::string &shareTimeHour(void) const { return _shareTimeHour; }
	const std::string &shareStreak(void) const { return _shareStreak; }
	const std::string &shareStreakLoss(void) const { return _shareStreakLoss; }

	const Button &hardModeToggle(void) const { return _hardModeToggle; }
	const Button &infiniteModeToggle(void) const { return _infiniteModeToggle; }
	const Button &highContrastToggle(void) const { return _highContrastToggle; }
	const Button &musicToggle(void) const { return _musicToggle; }
	const Button &shareMsgBtn(void) const { return _shareMsgBtn; }
	const Button &modBtn(void) const { return _modBtn; }

	const Button &shareTimerToggle(void) const { return _shareTimerToggle; }
	const Button &shareStreakToggle(void) const { return _shareStreakToggle; }
	const Button &shareUrlToggle(void) const { return _shareUrlToggle; }

	bool oldStatsMenu(void) const { return _oldStatsMenu; }

	const std::vector<char16_t> &letters(void) const { return _letters; }
	char16_t letters(size_t i) const { return _letters[i]; }

	const std::vector<Key> &keyboard(void) const { return _keyboard; }
	const Key &keyboard(size_t i) const { return _keyboard[i]; }

	const std::vector<std::vector<u16>> &letterPalettes(void) const { return _letterPalettes; }
	const std::vector<u16> &letterPalettes(size_t i) const { return _letterPalettes[i]; }
	const std::vector<std::vector<u16>> &fontPalettes(void) const { return _fontPalettes; }
	const std::vector<u16> &fontPalettes(size_t i) const { return _fontPalettes[i]; }

	const std::vector<std::u16string> &choices(void) const { return _choices; }
	const std::u16string &choices(size_t i) const { return _choices[i]; }
	const std::vector<std::u16string> &guesses(void) const { return _guesses; }
	const std::u16string &guesses(size_t i) const { return _guesses[i]; }

	const Image &bgBottom(void) const { return _bgBottom; };
	const Image &bgBottomBox(void) const { return _bgBottomBox; };
	const Image &bgTop(void) const { return _bgTop; };
	const Image &howtoBottom(void) const { return _howtoBottom; };
	const Image &howtoTop(void) const { return _howtoTop; };
	const Image &modsBottom(void) const { return _modsBottom; };
	const Image &settingsBottom(void) const { return _settingsBottom; };
	const Image &shareMsgSettings(void) const { return _shareMsgSettings; };
	const Image &statsBottom(void) const { return _statsBottom; };

	const OamGfx &backspaceKeyGfx(void) const { return _backspaceKeyGfx; }
	const OamGfx &enterKeyGfx(void) const { return _enterKeyGfx; }
	const OamGfx &refreshGfx(void) const { return _refreshGfx; }
	const OamGfx &toggleOffGfx(void) const { return _toggleOffGfx; }
	const OamGfx &toggleOnGfx(void) const { return _toggleOnGfx; }

	const std::vector<OamGfx> &letterGfx(void) const { return _letterGfx; };
	const OamGfx &letterGfx(size_t i) const { return _letterGfx[i]; };
	const std::vector<OamGfx> &letterGfxSub(void) const { return _letterGfxSub; };
	const OamGfx &letterGfxSub(size_t i) const { return _letterGfxSub[i]; };
	const std::vector<OamGfx> &kbdGfx(void) const { return _kbdGfx; };
	const OamGfx &kbdGfx(size_t i) const { return _kbdGfx[i]; };

	Font &mainFont(void) { return _mainFont; }
	Font &numbersLarge(void) { return _numbersLarge; }
	Font &numbersSmall(void) { return _numbersSmall; }

	Sprite &refreshSprite(void) { return _refreshSprite; }

	const std::string &emoji(TilePalette color) const;
	const std::string &victoryMessage(size_t i) const;
	const std::string &numberSuffix(int i) const;
	void setPalettes(bool altPalette) const;
};

#endif // GAME_DATA_HPP
