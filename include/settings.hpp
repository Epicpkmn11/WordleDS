#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "gameData.hpp"

#include <string>

class Settings {
	std::string _path;

	bool _hardMode = false;
	bool _altPalette = false;
	bool _music = true;
	std::string _mod = DEFAULT_MOD;

public:
	Settings(const std::string &path);

	bool save(void);

	static void legacyImport(const std::string &path);

	void showMenu(void);

	bool hardMode(void) const { return _hardMode; }
	Settings &hardMode(bool hardMode) { _hardMode = hardMode; return *this; }
	bool altPalette(void) const { return _altPalette; }
	Settings &altPalette(bool altPalette) { _altPalette = altPalette; return *this; }
	bool music(void) const { return _music; }
	Settings &music(bool music) { _music = music; return *this; }
	const std::string &mod(void) const { return _mod; }
	Settings &mod(const std::string &mod) { _mod = mod; return *this; }
};

extern Settings *settings;

#endif // SETTINGS_HPP
