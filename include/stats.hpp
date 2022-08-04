#ifndef STATS_HPP
#define STATS_HPP

#include <string>
#include <time.h>
#include <vector>

class Stats {
	std::string _path;

	std::vector<int> _guessCounts = {};
	std::vector<std::string> _boardState = {};
	std::vector<int> _completionTimes = {};
	int _maxStreak = 0;
	int _streak = 0;
	int _gamesPlayed = 0;
	time_t _lastPlayed = 0;
	time_t _lastWon = 0;
	int _timeElapsed = 0;

	void showQr(void);

public:
	Stats(const std::string &path);

	bool save(void);

	void showMenu(void);

	std::string shareMessage(void);

	const std::vector<int> &guessCounts(void) const { return _guessCounts; }
	Stats &guessCounts(const std::vector<int> &guessCounts) { _guessCounts = guessCounts; return *this; }
	Stats &guessCounts(int guessCount) { _guessCounts.push_back(guessCount); return *this; }

	const std::vector<std::string> &boardState(void) const { return _boardState; }
	Stats &boardState(const std::vector<std::string> &boardState) { _boardState = boardState; return *this; }
	Stats &boardState(const std::string &guess) { _boardState.push_back(guess); return *this; }

	const std::vector<int> &completionTimes(void) const { return _completionTimes; }
	Stats &completionTimes(const std::vector<int> &completionTimes) { _completionTimes = completionTimes; return *this; }
	Stats &completionTimes(const int &completionTime) { _completionTimes.push_back(completionTime); return *this; }

	int streak(void) const { return _streak; }
	Stats &streak(int streak) { _streak = streak; _maxStreak = std::max(_maxStreak, _streak); return *this; }

	int maxStreak(void) const { return _maxStreak; }
	Stats &maxStreak(int maxStreak) { _maxStreak = maxStreak; return *this; }

	int gamesPlayed(void) const { return _gamesPlayed; }
	Stats &gamesPlayed(int gamesPlayed) { _gamesPlayed = gamesPlayed; return *this; }

	time_t lastPlayed(void) const { return _lastPlayed; }
	Stats &lastPlayed(time_t lastPlayed) { _lastPlayed = lastPlayed; return *this; }

	time_t lastWon(void) const { return _lastWon; }
	Stats &lastWon(time_t lastWon) { _lastWon = lastWon; return *this; }

	int timeElapsed(void) const { return _timeElapsed; }
	Stats &timeElapsed(int timeElapsed) { _timeElapsed = timeElapsed; return *this; }

	bool firstPlay(void) const { return _gamesPlayed == 0 && _boardState.size() == 0; }
};

#endif // STATS_HPP
