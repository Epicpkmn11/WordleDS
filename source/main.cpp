#include "defines.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "howto.hpp"

#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

#include "soundbank.h"
#include "soundbank_bin.h"

Game *game;

int main() {
	bool fatInited = fatInitDefault();

	initGraphics();
	game = new Game();

	// Load music
	if(access(MUSIC_PATH, F_OK) == 0)
		mmInitDefault((char *)MUSIC_PATH);
	else
		mmInitDefaultMem((mm_addr)soundbank_bin);
	mmLoad(MOD_MUSIC);
	mmSetModuleVolume(800);
	if(game->config().music())
		mmStart(MOD_MUSIC, MM_PLAY_LOOP);

	// Show howto if first game
	if(game->config().gamesPlayed() < 1)
		howtoMenu();

	if(!fatInited)
		game->drawBgBottom("FAT init failed\nStats cannot be saved", 240);

	// Loop game until returns false
	while(game->run()) {
		delete game;
		game = new Game();
	}
}
