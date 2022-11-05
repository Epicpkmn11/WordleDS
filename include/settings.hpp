#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "gameData.hpp"

#include <string>
#include <vector>

class Settings {
	std::string _path;

	bool _hardMode = false;
	bool _infiniteMode = false;
	bool _altPalette = false;
	bool _music = true;
	bool _shareTimer = false;
	bool _shareStreak = false;
	bool _shareUrl = false;
	std::string _mod = DEFAULT_MOD;

	void gameSettings(void);
	void shareMsgSettings(void);

	std::vector<std::string> getMods(void);
	bool selectMod(void);

public:
	Settings(const std::string &path);

	bool save(void);

	static void legacyImport(const std::string &path);

	void showMenu(void);

	bool hardMode(void) const { return _hardMode; }
	Settings &hardMode(bool hardMode) { _hardMode = hardMode; return *this; }

	bool infiniteMode(void) const { return _infiniteMode; }
	Settings &infiniteMode(bool infiniteMode) { _infiniteMode = infiniteMode; return *this; }

	bool altPalette(void) const { return _altPalette; }
	Settings &altPalette(bool altPalette) { _altPalette = altPalette; return *this; }

	bool music(void) const { return _music; }
	Settings &music(bool music) { _music = music; return *this; }

	bool shareTimer(void) const { return _shareTimer; }
	Settings &shareTimer(bool shareTimer) { _shareTimer = shareTimer; return *this; }

	bool shareStreak(void) const { return _shareStreak; }
	Settings &shareStreak(bool shareStreak) { _shareStreak = shareStreak; return *this; }

	bool shareUrl(void) const { return _shareUrl; }
	Settings &shareUrl(bool shareUrl) { _shareUrl = shareUrl; return *this; }

	const std::string &mod(void) const { return _mod; }
	Settings &mod(const std::string &mod) { _mod = mod; return *this; }
};

extern Settings *settings;

#endif // SETTINGS_HPP
