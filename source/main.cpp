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

	mmInitDefaultMem((mm_addr)soundbank_bin);
	mmLoad(MOD_MUSIC);
	mmSetModuleVolume(800);
	mmStart(MOD_MUSIC, MM_PLAY_LOOP);

	initGraphics();
	game = new Game();

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
