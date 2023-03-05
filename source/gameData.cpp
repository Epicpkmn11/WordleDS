#include "gameData.hpp"
#include "settings.hpp"
#include "tonccpy.h"
#include "version.hpp"
#include "words.hpp"

#include "backspaceKey_grf.h"
#include "bgBottom_grf.h"
#include "btnHowto_grf.h"
#include "btnSettings_grf.h"
#include "btnStats_grf.h"
#include "btnUpdate_grf.h"
#include "popupBox_grf.h"
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
#include "gameSettings_grf.h"
#include "shareMsgSettings_grf.h"
#include "statsBottom_grf.h"
#include "toggleOff_grf.h"
#include "toggleOn_grf.h"

#include <algorithm>
#include <string.h>
#include <unistd.h>

// sassert but it fixes the brightness
#undef sassert
#define sassert(e,...) ((e) ? (void)0 : (setBrightness(2, 0), __sassert(__FILE__, __LINE__, #e, __VA_ARGS__)))

std::vector<u16> GameData::getPalette(const Json &json, int size) {
	std::vector<u16> output;

	for(const Json &palette : json) {
		if(palette.isArray()) {
			for(const Json &color : palette) {
				if(color.isString()) {
					output.push_back(strtol(color.get()->valuestring, nullptr, 0));
				} else if(color.isNumber()) {
					output.push_back(color.get()->valueint);
				}
			}
			sassert(palette.size() == size, "Invalid palette (%d colors,\nshould be %d)", output.size(), size);
		}
	}

	return output;
}

GameData::GameData(const std::string &folder) : _modPath(DATA_PATH + folder) {
	Json json((_modPath + MOD_JSON).c_str(), true);
	if(json.get() != nullptr) {
		const char *minVer = nullptr;
		if(json.contains("minVersion") && json["minVersion"].isString()) {
			minVer = json["minVersion"].get()->valuestring;
			sassert(strcmp(minVer, VER_NUMBER) <= 0, "This mod requires Wordle DS\nversion %s or newer, please update.\n\n(You have " VER_NUMBER ")", minVer);
		}

		// Version backwards compatibility, when there is no minimum version defined by the mod, assume the lowest version.
		// v2.0.0, The base version uses the old stats menu, no share settings, and also doesn't have the new settings layout
		// v2.1.0, The sharing update, it features the new statistics screen and the share settings
		// v2.2.0, Has all the features
		if(!minVer || strcmp(minVer, "v2.1.0") < 0) {
			// If the mod provided a settings screen, place buttons at old locations
			if(access((_modPath + "/settingsBottom.grf").c_str(), F_OK) == 0) {
				_hardModeToggle = {224, 37, 21, 13};
				_highContrastToggle = {224, 76, 21, 13};
				_musicToggle = {224, 102, 21, 13};
				_modBtn = {224, 127, 21, 13};

				// Not supported in this version
				_infiniteModeToggle = { -1, -1, 0, 0 };
				_gameSettingsBtn = { -1, -1, 0,0 };
				_shareMsgBtn = { -1, -1, 0, 0 };

				_oldSettingsMenu = true;
			}

			// If the mod provided a custom stats screen, use the old statmenu
			if(access((_modPath + "/statsBottom.grf").c_str(), F_OK) == 0) {
				_oldStatsMenu = true;
			}
		} else if(strcmp(minVer, "v2.2.0") < 0) {
			// If the mod provided a settings screen, place buttons on the v2.1.0 location
			if(access((_modPath + "/settingsBottom.grf").c_str(), F_OK) == 0) {
				_hardModeToggle = { 224, 33, 21, 13 };
				_highContrastToggle = { 224, 68, 21, 13 };
				_musicToggle = { 224, 92, 21, 13 };
				_shareMsgBtn = { 232, 108, 17, 17 };
				_modBtn = { 232, 131, 17, 17 };

				// Not supported in this version
				_infiniteModeToggle = { -1, -1, 0, 0 };
				_gameSettingsBtn = { -1, -1, 0,0 };

				_oldSettingsMenu = true;
			}
		}

		if(json.contains("shareName") && json["shareName"].isString())
			_shareName = json["shareName"].get()->valuestring;

		if(json.contains("wordleOffset") && json["wordleOffset"].isNumber())
			_firstDay += json["wordleOffset"].get()->valueint;

		if(json.contains("maxGuesses") && json["maxGuesses"].isNumber())
			_maxGuesses = json["maxGuesses"].get()->valueint;
		sassert(_maxGuesses > 0 && _maxGuesses <= 6, "Max guesses must be between 0\nand 6 (inclusive)\n\n(currently %d)", _maxGuesses);

		if(json.contains("lossMessage") && json["lossMessage"].isString())
			_lossMessage = json["lossMessage"].get()->valuestring;
		if(json.contains("lossMessageInfinite") && json["lossMessageInfinite"].isString())
			_lossMessageInfinite = json["lossMessageInfinite"].get()->valuestring;

		if(json.contains("tooShortMessage") && json["tooShortMessage"].isString())
			_tooShortMessage = json["tooShortMessage"].get()->valuestring;

		if(json.contains("notWordMessage") && json["notWordMessage"].isString())
			_notWordMessage = json["notWordMessage"].get()->valuestring;

		if(json.contains("creditStr") && json["creditStr"].isString())
			_creditStr = json["creditStr"].get()->valuestring;

		if(json.contains("nthMustBeX") && json["nthMustBeX"].isString())
			_nthMustBeX = json["nthMustBeX"].get()->valuestring;

		if(json.contains("guessMustContainX") && json["guessMustContainX"].isString())
			_guessMustContainX = json["guessMustContainX"].get()->valuestring;

		if(json.contains("hardModeOnlyAtStart") && json["hardModeOnlyAtStart"].isString())
			_hardModeOnlyAtStart = json["hardModeOnlyAtStart"].get()->valuestring;

		if(json.contains("shareMsg") && json["shareMsg"].isObject()) {
			if(json["shareMsg"].contains("time") && json["shareMsg"]["time"].isString())
				_shareTime = json["shareMsg"]["time"].get()->valuestring;

			if(json["shareMsg"].contains("timeHour") && json["shareMsg"]["timeHour"].isString())
				_shareTimeHour = json["shareMsg"]["timeHour"].get()->valuestring;

			if(json["shareMsg"].contains("streak") && json["shareMsg"]["streak"].isString())
				_shareStreak = json["shareMsg"]["streak"].get()->valuestring;

			if(json["shareMsg"].contains("streakLoss") && json["shareMsg"]["streakLoss"].isString())
				_shareStreakLoss = json["shareMsg"]["streakLoss"].get()->valuestring;
		}

		if(json.contains("emoji") && json["emoji"].isObject()) {
			if(json["emoji"].contains("green") && json["emoji"]["green"].isString())
				_emojiGreen = json["emoji"]["green"].get()->valuestring;

			if(json["emoji"].contains("greenAlt") && json["emoji"]["greenAlt"].isString())
				_emojiGreenAlt = json["emoji"]["greenAlt"].get()->valuestring;

			if(json["emoji"].contains("yellow") && json["emoji"]["yellow"].isString())
				_emojiYellow = json["emoji"]["yellow"].get()->valuestring;

			if(json["emoji"].contains("yellowAlt") && json["emoji"]["yellowAlt"].isString())
				_emojiYellowAlt = json["emoji"]["yellowAlt"].get()->valuestring;

			if(json["emoji"].contains("white") && json["emoji"]["white"].isString())
				_emojiWhite = json["emoji"]["white"].get()->valuestring;
		}

		if(json.contains("settings") && json["settings"].isObject()) {
			if(json["settings"].contains("buttons") && json["settings"]["buttons"].isObject()) {
				Json buttons = json["settings"]["buttons"];
				if(buttons.contains("hardMode") && buttons["hardMode"].isArray() && buttons["hardMode"].size() == 4)
					_hardModeToggle = Button(buttons["hardMode"]);

				if(buttons.contains("infiniteMode") && buttons["infiniteMode"].isArray() && buttons["infiniteMode"].size() == 4)
					_infiniteModeToggle = Button(buttons["infiniteMode"]);

				if(buttons.contains("highContrast") && buttons["highContrast"].isArray() && buttons["highContrast"].size() == 4)
					_highContrastToggle = Button(buttons["highContrast"]);

				if(buttons.contains("music") && buttons["music"].isArray() && buttons["music"].size() == 4)
					_musicToggle = Button(buttons["music"]);

				if(buttons.contains("shareMsg") && buttons["shareMsg"].isArray() && buttons["shareMsg"].size() == 4)
					_shareMsgBtn = Button(buttons["shareMsg"]);

				if(buttons.contains("mod") && buttons["mod"].isArray() && buttons["mod"].size() == 4)
					_modBtn = Button(buttons["mod"]);
			}

			if(json["settings"].contains("shareMsgButtons") && json["settings"]["shareMsgButtons"].isObject()) {
				Json buttons = json["settings"]["shareMsgButtons"];
				if(buttons.contains("timer") && buttons["timer"].isArray() && buttons["timer"].size() == 4)
					_shareTimerToggle = Button(buttons["timer"]);

				if(buttons.contains("streak") && buttons["streak"].isArray() && buttons["streak"].size() == 4)
					_shareStreakToggle = Button(buttons["streak"]);

				if(buttons.contains("url") && buttons["url"].isArray() && buttons["url"].size() == 4)
					_shareUrlToggle = Button(buttons["url"]);
			}
		}

		if(json.contains("howto") && json["howto"].isObject()) {
			int howtoWordsSize = 15;
			if(json["howto"].contains("words") && json["howto"]["words"].isArray() && json["howto"]["words"].size() > 0 && json["howto"]["words"].size() <= 3) {
				_howtoWords.clear();
				howtoWordsSize = 0;
				for(const Json &howtoWord : json["howto"]["words"]) {
					if(howtoWord.isString()) {
						std::u16string u16word = Font::utf8to16(howtoWord.get()->valuestring);
						sassert(u16word.size() > 0 && u16word.size() <= 9, "How to words must be between 1\nand 9 (inclusive) characters\n\n%s is %d", howtoWord.get()->valuestring, u16word.size());
						_howtoWords.push_back(u16word);
						howtoWordsSize += u16word.size();
					}
				}
			}

			if(json["howto"].contains("colors") && json["howto"]["colors"].isArray()) {
				sassert(json["howto"]["colors"].size() == howtoWordsSize, "Length of how to colors and\nwords must match\n\n(words is %d, colors is %d)", howtoWordsSize, json["howto"]["colors"].size());
				_howtoColors.clear();
				for(const Json &howtoColor : json["howto"]["colors"]) {
					if(strcmp(howtoColor.get()->valuestring, "white") == 0) {
						_howtoColors.push_back(TilePalette::whiteDark);
					} else if(strcmp(howtoColor.get()->valuestring, "green") == 0) {
						_howtoColors.push_back(TilePalette::green);
					} else if(strcmp(howtoColor.get()->valuestring, "yellow") == 0) {
						_howtoColors.push_back(TilePalette::yellow);
					} else if(strcmp(howtoColor.get()->valuestring, "gray") == 0) {
						_howtoColors.push_back(TilePalette::gray);
					} else {
						sassert(false, "Invalid how to color\n(%s)", howtoColor.get()->valuestring);
					}
				}
			}
		}

		if(json.contains("victoryMessages") && json["victoryMessages"].isArray() && json["victoryMessages"].size() > 0) {
			_victoryMessages.clear();
			for(const Json &victoryMessage : json["victoryMessages"]) {
				if(victoryMessage.isString())
					_victoryMessages.push_back(victoryMessage.get()->valuestring);
			}
		}

		if(json.contains("numberSuffixes") && json["numberSuffixes"].isObject()) {
			_numberSuffixes.clear();
			for(const Json &numberSuffix : json["numberSuffixes"]) {
				if(numberSuffix.isString()) {
					int key;
					if(strcmp(numberSuffix.get()->string, "default") == 0)
						key = 0xFFFF;
					else
						key = atoi(numberSuffix.get()->string);

					_numberSuffixes[key] = numberSuffix.get()->valuestring;
				}
			}
			_numberSuffixes[0xFFFF]; // Ensure default exists
		}

		if(json.contains("letters") && json["letters"].isArray() && json["letters"].size() > 0) {
			_letters.clear();
			for(const Json &letter : json["letters"]) {
				std::u16string u16letter = Font::utf8to16(letter.get()->valuestring);
				sassert(u16letter.size() == 1, "Invalid letter (%s), must be\n1 UTF-16 character", letter.get()->valuestring);
				_letters.push_back(u16letter[0]);
			}
		}

		if(json.contains("keyboard") && json["keyboard"].isArray()) {
			_keyboard.clear();
			for(const Json &key : json["keyboard"]) {
				if(key.isArray() && key.size() == 3 && key[0].isNumber() && key[1].isNumber() && key[2].isString()) {
					std::u16string letter = Font::utf8to16(key[2].get()->valuestring);
					sassert(letter.size() == 1, "Invalid key (%s), must be\n1 UTF-16 character", key[2].get()->valuestring);
					_keyboard.emplace_back(key[0].get()->valueint, key[1].get()->valueint, letter[0]);
				}
			}
		}

		if(json.contains("palettes") && json["palettes"].isObject()) {
			if(json["palettes"].contains("letter") && json["palettes"]["letter"].isObject()) {
				if(json["palettes"]["letter"].contains("regular") && json["palettes"]["letter"]["regular"].isArray() && json["palettes"]["letter"]["regular"].size() == 5)
					_letterPalettes[0] = getPalette(json["palettes"]["letter"]["regular"], 16);

				if(json["palettes"]["letter"].contains("highContrast") && json["palettes"]["letter"]["highContrast"].isArray() && json["palettes"]["letter"]["highContrast"].size() == 5)
					_letterPalettes[1] = getPalette(json["palettes"]["letter"]["highContrast"], 16);
			}

			if(json["palettes"].contains("font") && json["palettes"]["font"].isObject()) {
				if(json["palettes"]["font"].contains("regular") && json["palettes"]["font"]["regular"].isArray() && json["palettes"]["font"]["regular"].size() == 5)
					_fontPalettes[0] = getPalette(json["palettes"]["font"]["regular"], 4);

				if(json["palettes"]["font"].contains("highContrast") && json["palettes"]["font"]["highContrast"].isArray() && json["palettes"]["font"]["highContrast"].size() == 5)
					_fontPalettes[1] = getPalette(json["palettes"]["font"]["highContrast"], 4);
			}
		}

		if(json.contains("words") && json["words"].isObject()) {
			if(json["words"].contains("choices") && json["words"]["choices"].isArray() && json["words"]["choices"].size() > 0) {
				for(const Json &word : json["words"]["choices"]) {
					std::u16string u16word = Font::utf8to16(word.get()->valuestring);
					sassert(u16word.size() > 0 && u16word.size() <= 9, "Words must be between 1 and 9\n(inclusive) characters\n\n%s is %d", word.get()->valuestring, u16word.size());
					_choices.push_back(u16word);
				}
			} else {
				_choiceOrder = Words::order;
				_choices = Words::choices;
				if(!json["words"].contains("guesses") || !json["words"]["guesses"].isArray())
					_guesses = Words::guesses;
			}

			if(json["words"].contains("order") && json["words"]["order"].isArray()) {
				_choiceOrder.clear();
				for(const Json &word : json["words"]["order"]) {
					int i = word.get()->valueint;
					if(i >= 1 && i < (int)_choices.size()) {
						_choiceOrder.push_back(i);
					}
				}
			}

			if(json["words"].contains("orderUrl") && json["words"]["orderUrl"].isString()) {
				_choiceOrderUrl = json["words"]["orderUrl"].get()->valuestring;
			} else if(folder != DEFAULT_MOD) {
				// If not the default mod and no URL, don't show the update icon
				_choiceOrderUrl = "";
				_howtoBtn = {2, 2, 24, 24};
				_statsBtn = {116, 2, 24, 24};
				_settingsBtn = {230, 2, 24, 24};

				if((!minVer || strcmp(minVer, "v3.0.0") < 0) && access((_modPath + "/bottomBg.grf").c_str(), F_OK) == 0) {
					_mainMenuSprites = false;
				}
			}

			if(json["words"].contains("guesses") && json["words"]["guesses"].isArray()) {
				for(const Json &word : json["words"]["guesses"]) {
					std::u16string u16word = Font::utf8to16(word.get()->valuestring);
					if(u16word.size() > 0 && u16word.size() <= 9) {
						_guesses.push_back(u16word);
					}
				}
			}
		} else {
			_choiceOrder = Words::order;
			_choices = Words::choices;
			_guesses = Words::guesses;
		}

		if(json.contains("mainMenu") && json["mainMenu"].isObject()) {
			if(json["mainMenu"].contains("buttons") && json["mainMenu"]["buttons"].isObject()) {
				Json buttons = json["mainMenu"]["buttons"];

				if(buttons.contains("howto") && buttons["howto"].isArray() && buttons["howto"].size() == 4)
					_howtoBtn = Button(buttons["howto"]);

				if(buttons.contains("stats") && buttons["stats"].isArray() && buttons["stats"].size() == 4)
					_statsBtn = Button(buttons["stats"]);

				if(buttons.contains("update") && buttons["update"].isArray() && buttons["update"].size() == 4)
					_updateBtn = Button(buttons["update"]);

				if(buttons.contains("settings") && buttons["settings"].isArray() && buttons["settings"].size() == 4)
					_settingsBtn = Button(buttons["settings"]);
			}
		}
	} else {
		_choiceOrder = Words::order;
		_choices = Words::choices;
		_guesses = Words::guesses;
	}

	// Load images
	_bgBottom = Image((_modPath + "/bgBottom.grf").c_str(), 256, 192, bgBottom_grf);
	_popupBox = Image((_modPath + "/popupBox.grf").c_str(), 256, 192, popupBox_grf);
	_bgTop = Image((_modPath + "/bgTop.grf").c_str(), 256, 192, bgTop_grf);
	_howtoBottom = Image((_modPath + "/howtoBottom.grf").c_str(), 256, 192, howtoBottom_grf);
	_howtoTop = Image((_modPath + "/howtoTop.grf").c_str(), 256, 192, howtoTop_grf);
	_modsBottom = Image((_modPath + "/modsBottom.grf").c_str(), 256, 192, modsBottom_grf);
	_settingsBottom = Image((_modPath + "/settingsBottom.grf").c_str(), 256, 192, settingsBottom_grf);
	_gameSettings = Image((_modPath + "/gameSettings.grf").c_str(), 256, 192, gameSettings_grf);
	_shareMsgSettings = Image((_modPath + "/shareMsgSettings.grf").c_str(), 256, 192, shareMsgSettings_grf);
	_statsBottom = Image((_modPath + "/statsBottom.grf").c_str(), 256, 192, statsBottom_grf);

	// Keyboard
	_backspaceKeyGfx = OamGfx(false, SpriteSize_64x32, SpriteColorFormat_16Color);
	Image backspaceKey((_modPath + "/backspaceKey.grf").c_str(), 64, 32, backspaceKey_grf);
	backspaceKey.decompressTiles(_backspaceKeyGfx.get());

	_enterKeyGfx = OamGfx(false, SpriteSize_64x32, SpriteColorFormat_16Color);
	Image enterKey((_modPath + "/enterKey.grf").c_str(), 64, 32, enterKey_grf);
	enterKey.decompressTiles(_enterKeyGfx.get());

	// Toggles
	_toggleOffGfx = OamGfx(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	Image toggleOff((_modPath + "/toggleOff.grf").c_str(), 32, 16, toggleOff_grf);
	toggleOff.decompressTiles(_toggleOffGfx.get());
	
	_toggleOnGfx = OamGfx(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	Image toggleOn((_modPath + "/toggleOn.grf").c_str(), 32, 16, toggleOn_grf);
	toggleOn.decompressTiles(_toggleOnGfx.get());

	// Refresh
	_refreshGfx = OamGfx(false, SpriteSize_64x64, SpriteColorFormat_16Color);
	Image refreshButton((_modPath + "/refreshButton.grf").c_str(), 64, 64, refreshButton_grf);
	refreshButton.decompressTiles(_refreshGfx.get());

	// Main menu buttons
	_btnHowtoGfx = OamGfx(false, SpriteSize_32x32, SpriteColorFormat_16Color);
	Image btnHowto((_modPath + "/btnHowto.grf").c_str(), 32, 32, btnHowto_grf);
	btnHowto
		.decompressTiles(_btnHowtoGfx.get())
		.decompressPal(SPRITE_PALETTE_SUB + 0x50);

	_btnStatsGfx = OamGfx(false, SpriteSize_32x32, SpriteColorFormat_16Color);
	Image btnStats((_modPath + "/btnStats.grf").c_str(), 32, 32, btnStats_grf);
	btnStats
		.decompressTiles(_btnStatsGfx.get())
		.decompressPal(SPRITE_PALETTE_SUB + 0x60);

	_btnUpdateGfx = OamGfx(false, SpriteSize_32x32, SpriteColorFormat_16Color);
	Image btnUpdate((_modPath + "/btnUpdate.grf").c_str(), 32, 32, btnUpdate_grf);
	btnUpdate
		.decompressTiles(_btnUpdateGfx.get())
		.decompressPal(SPRITE_PALETTE_SUB + 0x70);

	_btnSettingsGfx = OamGfx(false, SpriteSize_32x32, SpriteColorFormat_16Color);
	Image btnSettings((_modPath + "/btnHowtobtnSettings").c_str(), 32, 32, btnSettings_grf);
	btnSettings
		.decompressTiles(_btnSettingsGfx.get())
		.decompressPal(SPRITE_PALETTE_SUB + 0x80);

	constexpr int tileSize = 32 * 32 / 2;

	Image kbdKeys((_modPath + "/kbdKeys.grf").c_str(), 32, 832, kbdKeys_grf, false);
	u8 *kbdKeysBuffer = new u8[kbdKeys.tilesLen()];
	kbdKeys.decompressTiles(kbdKeysBuffer, false);
	for(size_t i = 0; i < kbdKeys.tilesLen(); i += tileSize) {
		_kbdGfx.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
		tonccpy(_kbdGfx.back().get(), kbdKeysBuffer + i, tileSize);
	}
	delete[] kbdKeysBuffer;

	Image letterTiles((_modPath + "/letterTiles.grf").c_str(), 32, 864, letterTiles_grf, false);
	u8 *letterTilesBuffer = new u8[letterTiles.tilesLen()];
	letterTiles.decompressTiles(letterTilesBuffer, false);
	for(size_t i = 0; i < letterTiles.tilesLen(); i += tileSize) {
		_letterGfx.emplace_back(true, SpriteSize_32x32, SpriteColorFormat_16Color);
		_letterGfxSub.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
		tonccpy(_letterGfx.back().get(), letterTilesBuffer + i, tileSize);
		tonccpy(_letterGfxSub.back().get(), letterTilesBuffer + i, tileSize);
	}
	delete[] letterTilesBuffer;

	_btnHowtoSprite.move(_howtoBtn.x, _howtoBtn.y).visible(false).gfx(_btnHowtoGfx).palette(5);
	_btnStatsSprite.move(_statsBtn.x, _statsBtn.y).visible(false).gfx(_btnStatsGfx).palette(6);
	_btnUpdateSprite.move(_updateBtn.x, _updateBtn.y).visible(false).gfx(_btnUpdateGfx).palette(7);
	_btnSettingsSprite.move(_settingsBtn.x, _settingsBtn.y).visible(false).gfx(_btnSettingsGfx).palette(8);

	_refreshSprite.move(96, 36).visible(false).gfx(_refreshGfx);

	// Load fonts
	_mainFont = std::move(Font((_modPath + "/main.nftr").c_str(), main_nftr));
	_numbersLarge = std::move(Font((_modPath + "/numbersLarge.nftr").c_str(), numbersLarge_nftr));
	_numbersSmall = std::move(Font((_modPath + "/numbersSmall.nftr").c_str(), numbersSmall_nftr));
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

const std::u16string &GameData::getAnswer(time_t day) const {
	const static std::u16string emptyString;

	unsigned int index = (unsigned int)day - _firstDay;
	if(_choiceOrder.size() > 0 && !settings->infiniteMode()) {
		if(index < _choiceOrder.size())
			return _choices[_choiceOrder[index] - 1];
		else
			return emptyString;
	} else {
		return _choices[index % _choices.size()];
	}
}

void GameData::appendChoiceOrder(const std::vector<int> &ids) {
	_choiceOrder.reserve(_choiceOrder.size() + ids.size());
	_choiceOrder.insert(_choiceOrder.end(), ids.begin(), ids.end());

	Json json((_modPath + MOD_JSON).c_str(), true);
	if(json.get() == nullptr)
		json = Json();

	if(!json.contains("words"))
		json.create(true, "words");

	Json words = json["words"];
	words.set(_choiceOrder, "order");

	FILE *modJson = fopen((_modPath + MOD_JSON).c_str(), "w");
	if(modJson) {
		std::string str = json.dump();
		fwrite(str.c_str(), 1, str.size(), modJson);
		fclose(modJson);
	}
}
