#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <string>

class Music {
public:
	Music(const std::string &folder);
	~Music(void);

	void start(void) const;
	void stop(void) const;
};

#endif // MUSIC_HP
