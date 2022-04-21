#include "settings.hpp"
#include "font.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "sprite.hpp"
#include "stats.hpp"
#include "tonccpy.h"
#include "version.hpp"

#include "soundbank.h"

#include <algorithm>
#include <array>
#include <maxmod9.h>
#include <nds.h>

Settings *settings;

Settings::Settings(const std::string &path) : _path(path) {
	FILE *file = fopen(_path.c_str(), "r");
	if(!file)
		return;

	nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
	fclose(file);

	if(json.contains("hardMode") && json["hardMode"].is_boolean())
		_hardMode = json["hardMode"];
	if(json.contains("altPalette") && json["altPalette"].is_boolean())
		_altPalette = json["altPalette"];
	if(json.contains("music") && json["music"].is_boolean())
		_music = json["music"];
	if(json.contains("mod") && json["mod"].is_string())
		_mod = json["mod"];

	if(access((DATA_PATH + _mod).c_str(), F_OK) != 0)
		_mod = DEFAULT_MOD;
}

bool Settings::save() {
	nlohmann::json json({
		{"hardMode", _hardMode},
		{"altPalette", _altPalette},
		{"music", _music},
		{"mod", _mod}
	});

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
	FILE *file = fopen(path.c_str(), "r");
	if(!file)
		return;

	nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
	fclose(file);

	bool good = true;

	// Settings
	if(json.contains("settings") && json["settings"].is_object()) {
		Settings settings(SETTINGS_JSON);

		if(json["settings"].contains("hardMode") && json["settings"]["hardMode"].is_boolean())
			settings.hardMode(json["settings"]["hardMode"]);
		if(json["settings"].contains("altPalette") && json["settings"]["altPalette"].is_boolean())
			settings.altPalette(json["settings"]["altPalette"]);
		if(json["settings"].contains("music") && json["settings"]["music"].is_boolean())
			settings.music(json["settings"]["music"]);

		good = settings.save();
	}

	// Stats
	if(json.contains("stats") && json["stats"].is_object()) {
		Stats stats(DATA_PATH DEFAULT_MOD STATS_JSON);

		if(json["stats"].contains("guessCounts") && json["stats"]["guessCounts"].is_array()) {
			for(const auto &item : json["stats"]["guessCounts"]) {
				if(item.is_number())
					stats.guessCounts(item.get<int>());
			}
		}
		if(json["stats"].contains("boardState") && json["stats"]["boardState"].is_array()) {
			for(const auto &item : json["stats"]["boardState"]) {
				if(item.is_string())
					stats.boardState(item.get_ref<const std::string &>());
			}
		}
		if(json["stats"].contains("streak") && json["stats"]["streak"].is_number())
			stats.streak(json["stats"]["streak"]);
		if(json["stats"].contains("maxStreak") && json["stats"]["maxStreak"].is_number())
			stats.maxStreak(json["stats"]["maxStreak"]);
		if(json["stats"].contains("gamesPlayed") && json["stats"]["gamesPlayed"].is_number())
			stats.gamesPlayed(json["stats"]["gamesPlayed"]);
		if(json["stats"].contains("lastPlayed") && json["stats"]["lastPlayed"].is_number())
			stats.lastPlayed(json["stats"]["lastPlayed"]);

		time_t today = time(NULL) / 24 / 60 / 60;
		if(today - stats.lastPlayed() > 1)
			stats.streak(0);
		if(stats.streak() > stats.maxStreak())
			stats.maxStreak(stats.streak());
		if(stats.lastPlayed() != today)
			stats.boardState(std::vector<std::string>());

		good = stats.save();
	}

	if(good)
		remove(SETTINGS_JSON_OLD);
}

void Settings::showMenu() {
	// Change to settings menu background
	game->data().settingsBottom()
		.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
		.decompressMap(bgGetMapPtr(BG_SUB(0)))
		.decompressPal(BG_PALETTE_SUB);

	game->data().mainFont().palette(TEXT_GRAY);
	game->data().mainFont().print(4, 192 - 2 - game->data().mainFont().calcHeight(game->data().creditStr()), false, game->data().creditStr());
	game->data().mainFont().print(256 - 4, 192 - 2 - game->data().mainFont().height(), false, VER_NUMBER, Alignment::right);
	Font::update(false);

	Sprite hardToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	hardToggle.move(224, 47);
	Sprite colorToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	colorToggle.move(224, 89);
	Sprite musicToggle(false, SpriteSize_32x16, SpriteColorFormat_16Color);
	musicToggle.move(224, 120);

	while(1) {
		game->data().setPalettes(_altPalette);
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

		u16 pressed;
		touchPosition touch;
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			touchRead(&touch);
		} while(!(pressed & (KEY_B | KEY_TOUCH)));

		if(pressed & KEY_B) {
			break;
		}

		if(pressed & KEY_TOUCH) {
			if(touch.px > 232 && touch.py < 24) { // X
				break;
			} else if(touch.px >= 224 && touch.px <= 245) { // Toggle
				if(touch.py >= 47 && touch.py <= (47 + 13)) {
					if(game->stats().boardState().size() == 0) // Can't toggle mid-game
						_hardMode = !_hardMode;
				} else if(touch.py >= 89 && touch.py <= (89 + 13)) {
					_altPalette = !_altPalette;
				} else if(touch.py >= 120 && touch.py <= (120 + 13)) {
					_music = !_music;
					if(_music) {
						mmStart(MOD_MUSIC, MM_PLAY_LOOP);
					} else {
						mmStop();
						mmStop(); // yes, this *is* needed
					}
				}
			}
		}
	}

	save();

	Font::clear(false);
	Font::update(false);
}
