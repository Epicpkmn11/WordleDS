#ifndef GAME_HPP
#define GAME_HPP

#include "gameData.hpp"
#include "gfx.hpp"
#include "kbd.hpp"
#include "stats.hpp"

#include <string>
#include <vector>

class Game {
	GameData _data;
	Stats _stats;
	Kbd _kbd;

	time_t _today;
	std::u16string _answer;
	std::u16string _guess = u"";
	int _currentGuess = 0;
	bool _won = false;
	bool _statsSaved = false;
	bool _showRefresh = settings->infiniteMode();
	bool _timerStarted = false;
	bool _bootstubExists;
	std::vector<Sprite> _letterSprites;
	std::u16string _knownLetters, _knownPositions; // for hard mode

	static void timerHandler(void);

	void fadeOut(int frames, int screen);
	void fadeIn(int frames, int screen);

public:
	Game(void);

	~Game(void);

	std::vector<TilePalette> check(const std::u16string &guess);
	std::string shareMessage(void);
	bool run(void);

	Stats &stats(void) { return _stats; }
	GameData &data(void) { return _data; }
	Kbd &kbd(void) { return _kbd; }

	time_t today(void) const { return _today; }
	const std::u16string &answer(void) const { return _answer; }
	bool won(void) const { return _won; }
	int currentGuess(void) const { return _currentGuess; }

	std::vector<Sprite> &letterSprites(void) { return _letterSprites; }
	Sprite &letterSprites(size_t i) { return _letterSprites[i]; }
};

extern Game *game;

#endif // GAME_HPP
