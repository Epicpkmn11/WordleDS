#include "settings.hpp"
#include "font.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "music.hpp"
#include "sprite.hpp"
#include "stats.hpp"
#include "tonccpy.h"
#include "version.hpp"

#include <algorithm>
#include <array>
#include <dirent.h>
#include <maxmod9.h>
#include <nds.h>

Settings *settings;

Settings::Settings(const std::string &path) : _path(path) {
	Json json(_path.c_str());
	if(!json.get())
		return;

	if(json.contains("hardMode") && json["hardMode"].isBool())
		_hardMode = json["hardMode"].isTrue();

	if(json.contains("infiniteMode") && json["infiniteMode"].isBool()) {
		_infiniteMode = json["infiniteMode"].isTrue();
	}

	if(json.contains("altPalette") && json["altPalette"].isBool())
		_altPalette = json["altPalette"].isTrue();

	if(json.contains("music") && json["music"].isBool())
		_music = json["music"].isTrue();

	if(json.contains("mod") && json["mod"].isString())
		_mod = json["mod"].get()->valuestring;

	if(access((DATA_PATH + _mod).c_str(), F_OK) != 0)
		_mod = DEFAULT_MOD;

	if(json.contains("shareMsg") && json["shareMsg"].isObject()) {
		if(json["shareMsg"].contains("timer") && json["shareMsg"]["timer"].isBool())
			_shareTimer = json["shareMsg"]["timer"].isTrue();

		if(json["shareMsg"].contains("streak") && json["shareMsg"]["streak"].isBool())
			_shareStreak = json["shareMsg"]["streak"].isTrue();

		if(json["shareMsg"].contains("url") && json["shareMsg"]["url"].isBool())
			_shareUrl = json["shareMsg"]["url"].isTrue();
	} else if(json.contains("timer") && json["timer"].isBool()) {
		_shareTimer = json["timer"].isTrue();
	}
}

bool Settings::save() {
	Json json;
	json.set(_hardMode, "hardMode");
	json.set(_infiniteMode, "infiniteMode");
	json.set(_altPalette, "altPalette");
	json.set(_music, "music");
	json.set(_mod.c_str(), "mod");

	Json shareMsg = json.create(true, "shareMsg");
	shareMsg.set(_shareTimer, "timer");
	shareMsg.set(_shareStreak, "streak");
	shareMsg.set(_shareUrl, "url");

	FILE *file = fopen(_path.c_str(), "w");
	if(file) {
		std::string dump = json.dump();
		size_t bytesWritten = fwrite(dump.c_str(), 1, dump.size(), file);
		fclose(file);

		return bytesWritten == dump.size();
	}

	return false;
}

void Settings::legacyImport(const std::string &path) {
	Json json(path.c_str());
	if(!json.get())
		return;

	bool good = true;

	// Settings
	if(json.contains("settings") && json["settings"].isObject()) {
		Settings settings(SETTINGS_JSON);

		if(json["settings"].contains("hardMode") && json["settings"]["hardMode"].isBool())
			settings.hardMode(json["settings"]["hardMode"].isTrue());
		if(json["settings"].contains("infiniteMode") && json["settings"]["infiniteMode"].isBool())
			settings.infiniteMode(json["settings"]["infiniteMode"].isTrue());
		if(json["settings"].contains("altPalette") && json["settings"]["altPalette"].isBool())
			settings.altPalette(json["settings"]["altPalette"].isTrue());
		if(json["settings"].contains("music") && json["settings"]["music"].isBool())
			settings.music(json["settings"]["music"].isTrue());

		good = settings.save();
	}

	// Stats
	if(json.contains("stats") && json["stats"].isObject()) {
		Stats stats(DATA_PATH DEFAULT_MOD STATS_JSON);

		if(json["stats"].contains("guessCounts") && json["stats"]["guessCounts"].isArray()) {
			for(const Json &item : json["stats"]["guessCounts"]) {
				if(item.isNumber())
					stats.guessCounts(item.get()->valueint);
			}
		}
		if(json["stats"].contains("boardState") && json["stats"]["boardState"].isArray()) {
			for(const Json &item : json["stats"]["boardState"]) {
				if(item.isString())
					stats.boardState(item.get()->valuestring);
			}
		}
		if(json["stats"].contains("streak") && json["stats"]["streak"].isNumber())
			stats.streak(json["stats"]["streak"].get()->valueint);
		if(json["stats"].contains("maxStreak") && json["stats"]["maxStreak"].isNumber())
			stats.maxStreak(json["stats"]["maxStreak"].get()->valueint);
		if(json["stats"].contains("gamesPlayed") && json["stats"]["gamesPlayed"].isNumber())
			stats.gamesPlayed(json["stats"]["gamesPlayed"].get()->valueint);
		if(json["stats"].contains("lastPlayed") && json["stats"]["lastPlayed"].isNumber())
			stats.lastPlayed(json["stats"]["lastPlayed"].get()->valueint);

		time_t today = time(NULL) / 24 / 60 / 60;
		if(today - stats.lastPlayed() > 1)
			stats.streak(0);
		if(stats.streak() > stats.maxStreak())
			stats.maxStreak(stats.streak());
		if(stats.lastPlayed() != today)
			stats.boardState(std::vector<std::string>());

		good = stats.save();
	}

	if(good) {
		remove(SETTINGS_JSON_OLD);
		rename("WordleDS.txt", DATA_PATH DEFAULT_MOD "/share.txt");
		rename("WordleDS.msl", DATA_PATH DEFAULT_MOD "/music.msl");
	}
}

void Settings::showMenu() {
	// Change to settings menu background
	game->data().settingsBottom()
		.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
		.decompressMap(bgGetMapPtr(BG_SUB(0)))
		.decompressPal(BG_PALETTE_SUB);

	game->data().mainFont()
		.palette(TEXT_GRAY)
		.print(4, 192 - 2 - game->data().mainFont().calcHeight(game->data().creditStr()), false, game->data().creditStr())
		.print(256 - 4, 192 - 2 - game->data().mainFont().height(), false, VER_NUMBER, Alignment::right);
	Font::update(false);

	Sprite hardToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	hardToggle.move(game->data().hardModeToggle());
	Sprite colorToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	colorToggle.move(game->data().highContrastToggle());
	Sprite musicToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	musicToggle.move(game->data().musicToggle());

	if(!game->data().oldSettingsMenu()) {
		hardToggle.visible(false);
		colorToggle.visible(false);
		musicToggle.visible(false);
	}

	Gfx::fadeIn(FADE_FAST, FADE_BOTTOM);

	while(1) {
		if(game->data().oldSettingsMenu()) {
			hardToggle
				.gfx(_hardMode ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
				.palette(_hardMode ? TilePalette::green : TilePalette::gray);
			colorToggle
				.gfx(_altPalette ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
				.palette(_altPalette ? TilePalette::green : TilePalette::gray);
			musicToggle
				.gfx(_music ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
				.palette(_music ? TilePalette::green : TilePalette::gray);
			Sprite::update(false);
		}

		u16 pressed;
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
		} while(!(pressed & (KEY_B | KEY_TOUCH)));

		if(pressed & KEY_B) {
			break;
		}

		if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);

			if(touch.px > 232 && touch.py < 24) { // X
				break;
			} else if(game->data().oldSettingsMenu()) {
				if(game->data().hardModeToggle().touching(touch)) {
					if(game->stats().boardState().size() == 0) // Can't toggle mid-game
						_hardMode = !_hardMode;
				} else if(game->data().highContrastToggle().touching(touch)) {
					_altPalette = !_altPalette;
					game->data().setPalettes(_altPalette);
				} else if(game->data().musicToggle().touching(touch)) {
					_music = !_music;
					if(_music)
						Music::music->start();
					else
						Music::music->stop();
				}
			}

			if(game->data().gameSettingsBtn().touching(touch)) {
				Gfx::fadeOut(FADE_FAST, FADE_BOTTOM);
				// Clear text
				Font::clear(false);
				Font::update(false);

				gameSettings();

				// Restore background
				game->data().settingsBottom()
					.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
					.decompressMap(bgGetMapPtr(BG_SUB(0)))
					.decompressPal(BG_PALETTE_SUB);

				game->data().mainFont()
					.palette(TEXT_GRAY)
					.print(4, 192 - 2 - game->data().mainFont().calcHeight(game->data().creditStr()), false, game->data().creditStr())
					.print(256 - 4, 192 - 2 - game->data().mainFont().height(), false, VER_NUMBER, Alignment::right);
				Font::update(false);
				Gfx::fadeIn(FADE_FAST, FADE_BOTTOM);
			} else if(game->data().shareMsgBtn().touching(touch)) {
				Gfx::fadeOut(FADE_FAST, FADE_BOTTOM);
				// Clear text, hide sprites
				Font::clear(false);
				Font::update(false);

				if(game->data().oldSettingsMenu()) {
					hardToggle.visible(false);
					colorToggle.visible(false);
					musicToggle.visible(false);
					Sprite::update(false);
				}

				shareMsgSettings();

				// Restore background and sprites
				swiWaitForVBlank();
				game->data().settingsBottom()
					.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
					.decompressMap(bgGetMapPtr(BG_SUB(0)))
					.decompressPal(BG_PALETTE_SUB);

				if(game->data().oldSettingsMenu()) {
					hardToggle.visible(true);
					colorToggle.visible(true);
					musicToggle.visible(true);
					Sprite::update(false);
				}

				game->data().mainFont()
					.palette(TEXT_GRAY)
					.print(4, 192 - 2 - game->data().mainFont().calcHeight(game->data().creditStr()), false, game->data().creditStr())
					.print(256 - 4, 192 - 2 - game->data().mainFont().height(), false, VER_NUMBER, Alignment::right);
				Font::update(false);
				Gfx::fadeIn(FADE_FAST, FADE_BOTTOM);
			} else if(game->data().modBtn().touching(touch)) {
				Gfx::fadeOut(FADE_FAST, FADE_BOTTOM);
				// Clear text, hide sprites
				Font::clear(false);
				Font::update(false);

				if(game->data().oldSettingsMenu()) {
					hardToggle.visible(false);
					colorToggle.visible(false);
					musicToggle.visible(false);
					Sprite::update(false);
				}

				selectMod();

				// Restore background and sprites
				swiWaitForVBlank();
				game->data().settingsBottom()
					.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
					.decompressMap(bgGetMapPtr(BG_SUB(0)))
					.decompressPal(BG_PALETTE_SUB);

				if(game->data().oldSettingsMenu()) {
					hardToggle.visible(true);
					colorToggle.visible(true);
					musicToggle.visible(true);
					Sprite::update(false);
				}

				game->data().mainFont()
					.palette(TEXT_GRAY)
					.print(4, 192 - 2 - game->data().mainFont().calcHeight(game->data().creditStr()), false, game->data().creditStr())
					.print(256 - 4, 192 - 2 - game->data().mainFont().height(), false, VER_NUMBER, Alignment::right);
				Font::update(false);
				Gfx::fadeIn(FADE_FAST, FADE_BOTTOM);
			}
		}
	}

	save();
}

void Settings::gameSettings() {
	// Change to game settings background
	game->data().gameSettings()
		.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
		.decompressMap(bgGetMapPtr(BG_SUB(0)))
		.decompressPal(BG_PALETTE_SUB);

	Sprite hardModeToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	hardModeToggle.move(game->data().hardModeToggle());
	Sprite infiniteModeToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	infiniteModeToggle.move(game->data().infiniteModeToggle());
	Sprite highContrastToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	highContrastToggle.move(game->data().highContrastToggle());
	Sprite musicToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	musicToggle.move(game->data().musicToggle());

	while(1) {
		hardModeToggle
			.gfx(_hardMode ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
			.palette(_hardMode ? TilePalette::green : TilePalette::gray);
		infiniteModeToggle
			.gfx(_infiniteMode ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
			.palette(_infiniteMode ? TilePalette::green : TilePalette::gray);
		highContrastToggle
			.gfx(_altPalette ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
			.palette(_altPalette ? TilePalette::green : TilePalette::gray);
		musicToggle
			.gfx(_music ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
			.palette(_music ? TilePalette::green : TilePalette::gray);
		Sprite::update(false);
		Gfx::fadeIn(FADE_FAST, FADE_BOTTOM);

		u16 pressed;
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
		} while(!(pressed & (KEY_B | KEY_TOUCH)));

		if(pressed & KEY_B) {
			break;
		}

		if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);

			if(touch.px > 232 && touch.py < 24) { // X
				break;
			} else if(game->data().hardModeToggle().touching(touch)) {
				if(game->stats().boardState().size() == 0) // Can't toggle mid-game
					_hardMode = !_hardMode;
			} else if(game->data().infiniteModeToggle().touching(touch)) {
				_infiniteMode = !_infiniteMode;
			} else if(game->data().highContrastToggle().touching(touch)) {
				_altPalette = !_altPalette;
				game->data().setPalettes(_altPalette);
			} else if(game->data().musicToggle().touching(touch)) {
				_music = !_music;
				if(_music)
					Music::music->start();
				else
					Music::music->stop();
			}
		}
	}

	Gfx::fadeOut(FADE_FAST, FADE_BOTTOM);
}

void Settings::shareMsgSettings() {
	// Change to share message settings background
	game->data().shareMsgSettings()
		.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
		.decompressMap(bgGetMapPtr(BG_SUB(0)))
		.decompressPal(BG_PALETTE_SUB);

	Sprite timerToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	timerToggle.move(game->data().shareTimerToggle());
	Sprite streakToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	streakToggle.move(game->data().shareStreakToggle());
	Sprite urlToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	urlToggle.move(game->data().shareUrlToggle());

	while(1) {
		timerToggle
			.gfx(_shareTimer ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
			.palette(_shareTimer ? TilePalette::green : TilePalette::gray);
		streakToggle
			.gfx(_shareStreak ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
			.palette(_shareStreak ? TilePalette::green : TilePalette::gray);
		urlToggle
			.gfx(_shareUrl ? game->data().toggleOnGfx() : game->data().toggleOffGfx())
			.palette(_shareUrl ? TilePalette::green : TilePalette::gray);
		Sprite::update(false);
		Gfx::fadeIn(FADE_FAST, FADE_BOTTOM);

		u16 pressed;
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
		} while(!(pressed & (KEY_B | KEY_TOUCH)));

		if(pressed & KEY_B) {
			break;
		}

		if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);

			if(touch.px > 232 && touch.py < 24) { // X
				break;
			} else if(game->data().shareTimerToggle().touching(touch)) {
				_shareTimer = !_shareTimer;
			} else if(game->data().shareStreakToggle().touching(touch)) {
				_shareStreak = !_shareStreak;
			} else if(game->data().shareUrlToggle().touching(touch)) {
				_shareUrl = !_shareUrl;
			}
		}
	}

	Gfx::fadeOut(FADE_FAST, FADE_BOTTOM);
}

std::vector<std::string> Settings::getMods() {
	std::vector<std::string> dirContents;

	DIR *pdir = opendir(DATA_PATH);
	if(pdir == nullptr) {
		game->data().mainFont().print(0, 0, false, u"Unable to open directory").update(false);
	} else {
		while(true) {
			dirent *pent = readdir(pdir);
			if(pent == nullptr)
				break;

			if(pent->d_type & DT_DIR && pent->d_name[0] != '.') {
				dirContents.push_back(pent->d_name);
			}
		}
		closedir(pdir);
	}

	std::sort(dirContents.begin(), dirContents.end(), [](const std::string &a, const std::string &b) { return strcasecmp(a.c_str(), b.c_str()) < 0; });

	return dirContents;
}

void Settings::selectMod() {
	// Change to mods menu background
	swiWaitForVBlank();
	game->data().modsBottom()
		.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
		.decompressMap(bgGetMapPtr(BG_SUB(0)))
		.decompressPal(BG_PALETTE_SUB);

	std::vector<std::string> mods = getMods();
	Font &font = game->data().mainFont();
	int modsPerScreen = (192 - 24) / font.height();

	int cursorPos = 0, scrollPos = 0;
	// Find current mod for initial cursor position
	for(size_t i = 0; i < mods.size(); i++) {
		if(mods[i] == _mod) {
			cursorPos = i;
			break;
		}
	}
	while(1) {
		// Scroll if needed
		if(cursorPos < scrollPos)
			scrollPos = cursorPos;
		else if(cursorPos >= scrollPos + modsPerScreen)
			scrollPos = std::max(0, cursorPos - modsPerScreen + 1);

		Font::clear(false);
		for(int i = 0; i < modsPerScreen && i < (int)mods.size(); i++)
			font.palette((scrollPos + i) == cursorPos ? TEXT_GREEN : TEXT_GRAY)
			.print(5, 24 + i * font.height(), false, mods[scrollPos + i]);
		Font::update(false);
		Gfx::fadeIn(FADE_FAST, FADE_BOTTOM);

		u16 pressed, held;
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!(held & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT) || pressed & (KEY_A | KEY_B | KEY_TOUCH)));

		if(held & KEY_UP) {
			if(cursorPos > 0)
				cursorPos--;
			else
				cursorPos = mods.size() - 1;
		} else if(held & KEY_DOWN) {
			if(cursorPos < (int)mods.size() - 1)
				cursorPos++;
			else
				cursorPos = 0;
		} else if(held & KEY_LEFT) {
			cursorPos -= modsPerScreen;
			if(cursorPos < 0)
				cursorPos = 0;
		} else if(held & KEY_RIGHT) {
			cursorPos += modsPerScreen;
			if(cursorPos > (int)mods.size() - 1)
				cursorPos = mods.size() - 1;
		} else if(pressed & KEY_A) {
			_mod = mods[cursorPos];
			break;
		} else if(pressed & KEY_B) {
			break;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);

			if(touch.px > 232 && touch.py < 24) { // X
				break;
			} else if(touch.py >= 24) {
				size_t touched = (touch.py - 24) / font.height();
				if(touched < mods.size()) {
					_mod = mods[touched];
					break;
				}
			}
		}
	}

	Gfx::fadeOut(FADE_FAST, FADE_BOTTOM);
	Font::clear(false);
	Font::update(false);

	return;
}
