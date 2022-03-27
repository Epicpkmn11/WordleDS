#include "config.hpp"
#include "json.hpp"

#include <unistd.h>

Config::Config(const std::string &path) : _path(path) {
	FILE *file = fopen(_path.c_str(), "r");
	if(!file)
		return;

	nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
	fclose(file);

	// Stats
	if(json.contains("stats") && json["stats"].is_object()) {
		if(json["stats"].contains("guessCounts") && json["stats"]["guessCounts"].is_array()) {
			for(const auto &item : json["stats"]["guessCounts"]) {
				if(item.is_number())
					_guessCounts.push_back(item);
			}
		}
		if(json["stats"].contains("boardState") && json["stats"]["boardState"].is_array()) {
			for(const auto &item : json["stats"]["boardState"]) {
				if(item.is_string())
					_boardState.push_back(item);
			}
		}
		if(json["stats"].contains("streak") && json["stats"]["streak"].is_number())
			_streak = json["stats"]["streak"];
		if(json["stats"].contains("maxStreak") && json["stats"]["maxStreak"].is_number())
			_maxStreak = json["stats"]["maxStreak"];
		if(json["stats"].contains("gamesPlayed") && json["stats"]["gamesPlayed"].is_number())
			_gamesPlayed = json["stats"]["gamesPlayed"];
		if(json["stats"].contains("lastPlayed") && json["stats"]["lastPlayed"].is_number())
			_lastPlayed = json["stats"]["lastPlayed"];

		time_t today = time(NULL) / 24 / 60 / 60;
		if(today - _lastPlayed > 1)
			_streak = 0;
		if(_streak > _maxStreak)
			_maxStreak = _streak;
		if(_lastPlayed != today)
			_boardState = {};
	}

	// Settings
	if(json.contains("settings") && json["settings"].is_object()) {
		if(json["settings"].contains("hardMode") && json["settings"]["hardMode"].is_boolean())
			_hardMode = json["settings"]["hardMode"];
		if(json["settings"].contains("altPalette") && json["settings"]["altPalette"].is_boolean())
			_altPalette = json["settings"]["altPalette"];
	}
}

void Config::save() {
	nlohmann::json json({
		{"stats", {
			{"guessCounts", _guessCounts},
			{"boardState", _boardState},
			{"streak", _streak},
			{"maxStreak", _maxStreak},
			{"gamesPlayed", _gamesPlayed},
			{"lastPlayed", _lastPlayed}
		}},
		{"settings", {
			{"hardMode", _hardMode},
			{"altPalette", _altPalette}
		}}
	});

	FILE *file = fopen(_path.c_str(), "w");
	if(file) {
		std::string dump = json.dump();
		fwrite(dump.c_str(), 1, dump.size(), file);
		fclose(file);
	}
}
