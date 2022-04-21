#include "game.hpp"
#include "gfx.hpp"
#include "howto.hpp"
#include "settings.hpp"

#include <dirent.h>
#include <fat.h>
#include <nds/arm9/background.h>
#include <maxmod9.h>
#include <stdio.h>
#include <unistd.h>

#include "soundbank.h"
#include "soundbank_bin.h"

int main() {
	bool fatInited = fatInitDefault();

	mkdir("/_nds", 0777);
	mkdir("/_nds/WordleDS", 0777);
	mkdir(DATA_PATH DEFAULT_MOD, 0777);

	// Import old settings to new location
	if(access(SETTINGS_JSON_OLD, F_OK) == 0)
		Settings::legacyImport(SETTINGS_JSON_OLD);

	Gfx::init();
	settings = new Settings(SETTINGS_JSON);

	// Load music
	std::string musicPath = DATA_PATH + settings->mod() + "/music.msl";
	if(access(musicPath.c_str(), F_OK) == 0)
		mmInitDefault(musicPath.data());
	else
		mmInitDefaultMem((mm_addr)soundbank_bin);
	mmLoad(MOD_MUSIC);
	mmSetModuleVolume(800);
	if(settings->music())
		mmStart(MOD_MUSIC, MM_PLAY_LOOP);

	game = new Game();

	// Show howto if first game
	if(game->stats().gamesPlayed() < 1)
		howtoMenu();

	game->drawBgBottom(fatInited ? "" : "FAT init failed\nStats cannot be saved", 240);
	game->data().bgTop()
		.decompressTiles(bgGetGfxPtr(BG(0)))
		.decompressMap(bgGetMapPtr(BG(0)))
		.decompressPal(BG_PALETTE);

	// Loop game until returns false
	while(game->run()) {
		delete game;
		game = new Game();
	}
}
