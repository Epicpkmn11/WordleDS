#include "gameData.hpp"
#include "settings.hpp"
#include "tonccpy.h"
#include "words.hpp"

#include "backspaceKey_grf.h"
#include "bgBottom_grf.h"
#include "bgBottomBox_grf.h"
#include "bgTop_grf.h"
#include "enterKey_grf.h"
#include "howtoBottom_grf.h"
#include "howtoTop_grf.h"
#include "kbdKeys_grf.h"
#include "letterTiles_grf.h"
#include "main_nftr.h"
#include "modsBottom_grf.h"
#include "numbersLarge_nftr.h"
#include "numbersSmall_nftr.h"
#include "refreshButton_grf.h"
#include "settingsBottom_grf.h"
#include "statsBottom_grf.h"
#include "toggleOff_grf.h"
#include "toggleOn_grf.h"

#include <algorithm>

std::vector<u16> GameData::getPalette(const nlohmann::json &json, size_t size) {
	std::vector<u16> output;

	for(const auto &palette : json) {
		if(palette.is_array()) {
			for(const auto &color : palette) {
				if(color.is_string()) {
					output.push_back(strtol(color.get_ref<const std::string &>().c_str(), nullptr, 0));
				} else if(color.is_number()) {
					output.push_back(color);
				}
			}
			sassert(palette.size() == size, "Invalid palette (%d colors,\nshould be %d)", output.size(), size);
		}
	}

	return output;
}

GameData::GameData(const std::string &folder) {
	const std::string modPath(DATA_PATH + folder);

	FILE *file = fopen((modPath + MOD_JSON).c_str(), "r");
	if(file) {
		nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
		fclose(file);

		if(json.contains("shareName") && json["shareName"].is_string())
			_shareName = json["shareName"];

		if(json.contains("wordleOffset") && json["wordleOffset"].is_number())
			_firstDay += json["wordleOffset"].get<int>();

		if(json.contains("maxGuesses") && json["maxGuesses"].is_number())
			_maxGuesses = json["maxGuesses"];
		sassert(_maxGuesses > 0 && _maxGuesses <= 6, "Max guesses must be between 0\nand 6 (inclusive)\n\n(currently %d)", _maxGuesses);

		if(json.contains("lossMessage") && json["lossMessage"].is_string())
			_lossMessage = json["lossMessage"];

		if(json.contains("tooShortMessage") && json["tooShortMessage"].is_string())
			_tooShortMessage = json["tooShortMessage"];

		if(json.contains("notWordMessage") && json["notWordMessage"].is_string())
			_notWordMessage = json["notWordMessage"];

		if(json.contains("creditStr") && json["creditStr"].is_string())
			_creditStr = json["creditStr"];

		if(json.contains("nthMustBeX") && json["nthMustBeX"].is_string())
			_nthMustBeX = json["nthMustBeX"];

		if(json.contains("guessMustContainX") && json["guessMustContainX"].is_string())
			_guessMustContainX = json["guessMustContainX"];

		if(json.contains("emoji") && json["emoji"].is_object()) {
			if(json["emoji"].contains("green") && json["emoji"]["green"].is_string())
				_emojiGreen = json["emoji"]["green"];

			if(json["emoji"].contains("greenAlt") && json["emoji"]["greenAlt"].is_string())
				_emojiGreenAlt = json["emoji"]["greenAlt"];

			if(json["emoji"].contains("yellow") && json["emoji"]["yellow"].is_string())
				_emojiYellow = json["emoji"]["yellow"];

			if(json["emoji"].contains("yellowAlt") && json["emoji"]["yellowAlt"].is_string())
				_emojiYellow = json["emoji"]["yellowAlt"];

			if(json["emoji"].contains("white") && json["emoji"]["white"].is_string())
				_emojiWhite = json["emoji"]["white"];
		}

		if(json.contains("howto") && json["howto"].is_object()) {
			size_t howtoWordsSize = 15;
			if(json["howto"].contains("words") && json["howto"]["words"].is_array() && json["howto"]["words"].size() > 0 && json["howto"]["words"].size() <= 3) {
				_howtoWords.clear();
				howtoWordsSize = 0;
				for(const auto &howtoWord : json["howto"]["words"]) {
					if(howtoWord.is_string()) {
						std::u16string u16word = Font::utf8to16(howtoWord.get_ref<const std::string &>());
						sassert(u16word.size() > 0 && u16word.size() <= 9, "How to words must be between 1\nand 9 (inclusive) characters\n\n%s is %d", howtoWord.get_ref<const std::string &>().c_str(), u16word.size());
						_howtoWords.push_back(u16word);
						howtoWordsSize += u16word.size();
					}
				}
			}

			if(json["howto"].contains("colors") && json["howto"]["colors"].is_array()) {
				sassert(json["howto"]["colors"].size() == howtoWordsSize, "Length of how to colors and\nwords must match\n\n(words is %d, colors is %d)", howtoWordsSize, json["howto"]["colors"].size());
				_howtoColors.clear();
				for(const auto &howtoColor : json["howto"]["colors"]) {
					if(howtoColor == "white") {
						_howtoColors.push_back(TilePalette::whiteDark);
					} else if(howtoColor == "green") {
						_howtoColors.push_back(TilePalette::green);
					} else if(howtoColor == "yellow") {
						_howtoColors.push_back(TilePalette::yellow);
					} else if(howtoColor == "gray") {
						_howtoColors.push_back(TilePalette::gray);
					} else {
						sassert(false, "Invalid how to color\n(%s)", howtoColor.get_ref<const std::string &>().c_str());
					}
				}
			}
		}

		if(json.contains("victoryMessages") && json["victoryMessages"].is_array() && json["victoryMessages"].size() > 0) {
			_victoryMessages.clear();
			for(const auto &victoryMessage : json["victoryMessages"]) {
				if(victoryMessage.is_string())
					_victoryMessages.push_back(victoryMessage);
			}
		}

		if(json.contains("numberSuffixes") && json["numberSuffixes"].is_object()) {
			_numberSuffixes.clear();
			for(const auto &numberSuffix : json["numberSuffixes"].items()) {
				if(numberSuffix.value().is_number()) {
					int key;
					if(numberSuffix.key() == "default")
						key = 0xFFFF;
					else
						key = atoi(numberSuffix.key().c_str());

					_numberSuffixes[key] = numberSuffix.value();
				}
			}
			_numberSuffixes[0xFFFF]; // Ensure default exists
		}

		if(json.contains("letters") && json["letters"].is_array() && json["letters"].size() > 0) {
			_letters.clear();
			for(const auto &letter : json["letters"]) {
				std::u16string u16letter = Font::utf8to16(letter.get_ref<const std::string &>());
				sassert(u16letter.size() == 1, "Invalid letter (%s), must be\n1 UTF-16 character", letter.get_ref<const std::string &>().c_str());
				_letters.push_back(u16letter[0]);
			}
		}

		if(json.contains("keyboard") && json["keyboard"].is_array()) {
			_keyboard.clear();
			for(const auto &key : json["keyboard"]) {
				if(key.is_array() && key.size() == 3 && key[0].is_number() && key[1].is_number() && key[2].is_string()) {
					std::u16string letter = Font::utf8to16(key[2].get_ref<const std::string &>());
					sassert(letter.size() == 1, "Invalid key (%s), must be\n1 UTF-16 character", key[2].get_ref<const std::string &>().c_str());
					_keyboard.emplace_back(key[0], key[1], letter[0]);
				}
			}
		}

		if(json.contains("palettes") && json["palettes"].is_object()) {
			if(json["palettes"].contains("letter") && json["palettes"]["letter"].is_object()) {
				if(json["palettes"]["letter"].contains("regular") && json["palettes"]["letter"]["regular"].is_array() && json["palettes"]["letter"]["regular"].size() == 5)
					_letterPalettes[0] = getPalette(json["palettes"]["letter"]["regular"], 16);

				if(json["palettes"]["letter"].contains("highContrast") && json["palettes"]["letter"]["highContrast"].is_array() && json["palettes"]["letter"]["highContrast"].size() == 5)
					_letterPalettes[1] = getPalette(json["palettes"]["letter"]["highContrast"], 16);
			}

			if(json["palettes"].contains("font") && json["palettes"]["font"].is_object()) {
				if(json["palettes"]["font"].contains("regular") && json["palettes"]["font"]["regular"].is_array() && json["palettes"]["font"]["regular"].size() == 5)
					_fontPalettes[0] = getPalette(json["palettes"]["font"]["regular"], 4);

				if(json["palettes"]["font"].contains("highContrast") && json["palettes"]["font"]["highContrast"].is_array() && json["palettes"]["font"]["highContrast"].size() == 5)
					_fontPalettes[1] = getPalette(json["palettes"]["font"]["highContrast"], 4);
			}
		}

		if(json.contains("words") && json["words"].is_object()) {
			if(json["words"].contains("choices") && json["words"]["choices"].is_array() && json["words"]["choices"].size() > 0) {
				for(const auto &word : json["words"]["choices"]) {
					std::u16string u16word = Font::utf8to16(word.get_ref<const std::string &>());
					sassert(u16word.size() > 0 && u16word.size() <= 9, "Words must be between 1 and 9\n(inclusive) characters\n\n%s is %d", word.get_ref<const std::string &>().c_str(), u16word.size());
					_choices.push_back(u16word);
				}
			} else {
				_choices = Words::choices;
			}

			if(json["words"].contains("guesses") && json["words"]["guesses"].is_array()) {
				for(const auto &word : json["words"]["guesses"]) {
					std::u16string u16word = Font::utf8to16(word.get_ref<const std::string &>());
					if(u16word.size() > 0 && u16word.size() <= 9) {
						_guesses.push_back(u16word);
					}
				}
			}
		} else {
			_choices = Words::choices;
			_guesses = Words::guesses;
		}
	} else {
		_choices = Words::choices;
		_guesses = Words::guesses;
	}

	// Load images
	_bgBottom = Image((modPath + "/bgBottom.grf").c_str(), 256, 192, bgBottom_grf);
	_bgBottomBox = Image((modPath + "/bgBottomBox.grf").c_str(), 256, 192, bgBottomBox_grf);
	_bgTop = Image((modPath + "/bgTop.grf").c_str(), 256, 192, bgTop_grf);
	_howtoBottom = Image((modPath + "/howtoBottom.grf").c_str(), 256, 192, howtoBottom_grf);
	_howtoTop = Image((modPath + "/howtoTop.grf").c_str(), 256, 192, howtoTop_grf);
	_modsBottom = Image((modPath + "/modsBottom.grf").c_str(), 256, 192, modsBottom_grf);
	_settingsBottom = Image((modPath + "/settingsBottom.grf").c_str(), 256, 192, settingsBottom_grf);
	_statsBottom = Image((modPath + "/statsBottom.grf").c_str(), 256, 192, statsBottom_grf);

	Image backspaceKey = Image((modPath + "/backspaceKey.grf").c_str(), 64, 32, backspaceKey_grf);
	_backspaceKeyGfx = OamGfx(false, SpriteSize_64x32, SpriteColorFormat_16Color);
	backspaceKey.decompressTiles(_backspaceKeyGfx.get());

	Image enterKey = Image((modPath + "/enterKey.grf").c_str(), 64, 32, enterKey_grf);
	_enterKeyGfx = OamGfx(false, SpriteSize_64x32, SpriteColorFormat_16Color);
	enterKey.decompressTiles(_enterKeyGfx.get());

	Image toggleOff = Image((modPath + "/toggleOff.grf").c_str(), 32, 16, toggleOff_grf);
	_toggleOffGfx = OamGfx(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	toggleOff.decompressTiles(_toggleOffGfx.get());
	
	Image toggleOn = Image((modPath + "/toggleOn.grf").c_str(), 32, 16, toggleOn_grf);
	_toggleOnGfx = OamGfx(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	toggleOn.decompressTiles(_toggleOnGfx.get());

	Image refreshButton = Image((modPath + "/refreshButton.grf").c_str(), 64, 64, refreshButton_grf);
	_refreshGfx = OamGfx(false, SpriteSize_64x64, SpriteColorFormat_16Color);
	refreshButton.decompressTiles(_refreshGfx.get());

	constexpr int tileSize = 32 * 32 / 2;

	Image kbdKeys = Image((modPath + "/kbdKeys.grf").c_str(), 32, 832, kbdKeys_grf, false);
	u8 *kbdKeysBuffer = new u8[kbdKeys.tilesLen()];
	kbdKeys.decompressTiles(kbdKeysBuffer, false);
	for(size_t i = 0; i < kbdKeys.tilesLen(); i += tileSize) {
		_kbdGfx.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
		tonccpy(_kbdGfx.back().get(), kbdKeysBuffer + i, tileSize);
	}
	delete[] kbdKeysBuffer;

	Image letterTiles((modPath + "/letterTiles.grf").c_str(), 32, 864, letterTiles_grf, false);
	u8 *letterTilesBuffer = new u8[letterTiles.tilesLen()];
	letterTiles.decompressTiles(letterTilesBuffer, false);
	for(size_t i = 0; i < letterTiles.tilesLen(); i += tileSize) {
		_letterGfx.emplace_back(true, SpriteSize_32x32, SpriteColorFormat_16Color);
		_letterGfxSub.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
		tonccpy(_letterGfx.back().get(), letterTilesBuffer + i, tileSize);
		tonccpy(_letterGfxSub.back().get(), letterTilesBuffer + i, tileSize);
	}
	delete[] letterTilesBuffer;

	_refreshSprite.move(96, 36).visible(false).gfx(_refreshGfx);

	// Load fonts
	_mainFont = std::move(Font((modPath + "/main.nftr").c_str(), main_nftr));
	_numbersLarge = std::move(Font((modPath + "/numbersLarge.nftr").c_str(), numbersLarge_nftr));
	_numbersSmall = std::move(Font((modPath + "/numbersSmall.nftr").c_str(), numbersSmall_nftr));
	_numbersLarge.palette(TEXT_BLACK);
	_numbersSmall.palette(TEXT_BLACK);
}

const std::string &GameData::emoji(TilePalette color) const {
	switch(color) {
		case TilePalette::green:
			return settings->altPalette() ? _emojiGreenAlt : _emojiGreen;

		case TilePalette::yellow:
			return settings->altPalette() ? _emojiYellowAlt : _emojiYellow;

		default:
			return _emojiWhite;
	}
}

const std::string &GameData::victoryMessage(size_t i) const {
	if(i < _victoryMessages.size())
		return _victoryMessages[i];
	else
		return _victoryMessages.back();
}

const std::string &GameData::numberSuffix(int i) const {
	const auto it = _numberSuffixes.find(i);
	if(it != _numberSuffixes.end())
		return it->second;
	else
		return _numberSuffixes.at(0xFFFF);
}

void GameData::setPalettes(bool altPalette) const {
	// Sprites
	tonccpy(SPRITE_PALETTE, _letterPalettes[altPalette].data(), _letterPalettes[altPalette].size() * sizeof(u16));
	tonccpy(SPRITE_PALETTE_SUB, _letterPalettes[altPalette].data(), _letterPalettes[altPalette].size() * sizeof(u16));

	// Fonts
	tonccpy(BG_PALETTE_SUB + TEXT_BLACK, _fontPalettes[altPalette].data(), _fontPalettes[altPalette].size() * sizeof(u16));
}
