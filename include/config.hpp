#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <time.h>
#include <vector>

class Config {
	std::string _path;

	// Stats
	std::vector<int> _guessCounts = {};
	std::vector<std::string> _boardState = {};
	int _maxStreak = 0;
	int _streak = 0;
	int _gamesPlayed = 0;
	time_t _lastPlayed = time(NULL) / 24 / 60 / 60;

	// Settings
	bool _hardMode = false;
	bool _altPalette = false;

public:
	Config(const std::string &path);

	void save(void);

	// Stats
	const std::vector<int> &guessCounts(void) const { return _guessCounts; }
	Config &guessCounts(const std::vector<int> &guessCounts) { _guessCounts = guessCounts; return *this; }
	Config &guessCounts(int guessCount) { _guessCounts.push_back(guessCount); return *this; }
	const std::vector<std::string> &boardState(void) const { return _boardState; }
	Config &boardState(const std::vector<std::string> &boardState) { _boardState = boardState; return *this; }
	Config &boardState(const std::string &guess) { _boardState.push_back(guess); return *this; }
	int streak(void) const { return _streak; }
	Config &streak(int streak) { _streak = streak; _maxStreak = std::max(_maxStreak, _streak); return *this; }
	int maxStreak(void) const { return _maxStreak; }
	Config &maxStreak(int maxStreak) { _maxStreak = maxStreak; return *this; }
	int gamesPlayed(void) const { return _gamesPlayed; }
	Config &gamesPlayed(int gamesPlayed) { _gamesPlayed = gamesPlayed; return *this; }
	time_t lastPlayed(void) const { return _lastPlayed; }
	Config &lastPlayed(time_t lastPlayed) { _lastPlayed = lastPlayed; return *this; }

	// Settings
	bool hardMode(void) const { return _hardMode; }
	Config &hardMode(bool hardMode) { _hardMode = hardMode; return *this; }
	bool altPalette(void) const { return _altPalette; }
	Config &altPalette(bool altPalette) { _altPalette = altPalette; return *this; }
};

#endif // CONFIG_HPP
