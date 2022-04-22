#include "music.hpp"
#include "gameData.hpp"

#include "soundbank.h"
#include "soundbank_bin.h"

#include <maxmod9.h>
#include <unistd.h>

Music *Music::music;

Music::Music(const std::string &folder) {
	std::string musicPath = DATA_PATH + folder + "/music.msl";
	if(access(musicPath.c_str(), F_OK) == 0)
		mmInitDefault(musicPath.data());
	else
		mmInitDefaultMem((mm_addr)soundbank_bin);
	mmLoad(MOD_MUSIC);
	mmSetModuleVolume(800);
}

Music::~Music(void) {
	stop();
	mmUnload(MOD_MUSIC);
}

void Music::start(void) const {
	mmStart(MOD_MUSIC, MM_PLAY_LOOP);
}

void Music::stop(void) const {
	mmStop();
	mmStop(); // yes, this *is* needed
}
