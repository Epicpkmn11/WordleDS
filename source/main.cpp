#include "defines.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "howto.hpp"

#include <fat.h>
#include <nds.h>
#include <stdio.h>

Game *game;

int main() {
	bool fatInited = fatInitDefault();

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
