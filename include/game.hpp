#ifndef GAME_HPP
#define GAME_HPP

#include "gameData.hpp"
#include "gfx.hpp"
#include "kbd.hpp"
#include "stats.hpp"

#include <string>
#include <vector>

class Game {
	Stats _stats;
	GameData _data;
	Kbd _kbd;

	time_t _today = time(NULL) / 24 / 60 / 60;
	std::u16string_view _answer;
	std::u16string _guess = u"";
	int _currentGuess = 0;
	int _popupTimeout = -1;
	bool _won = false;
	bool _statsSaved = false;
	bool _showRefresh = false;
	bool _bootstubExists;
	std::vector<Sprite> _letterSprites;
	std::u16string _knownLetters, _knownPositions; // for hard mode

public:
	Game(void);

	~Game(void);

	void drawBgBottom(std::string_view msg, int timeout);
	std::vector<TilePalette> check(const std::u16string &guess);
	std::string shareMessage(void);
	bool run(void);

	Stats &stats(void) { return _stats; }
	GameData &data(void) { return _data; }
	Kbd &kbd(void) { return _kbd; }

	std::u16string_view answer(void) const { return _answer; }
	bool won(void) const { return _won; }

	std::vector<Sprite> &letterSprites(void) { return _letterSprites; }
	Sprite &letterSprites(size_t i) { return _letterSprites[i]; }
};

extern Game *game;

#endif // GAME_HPP
