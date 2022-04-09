#include "defines.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "howto.hpp"

#include <fat.h>
#include <nds.h>
#include <stdio.h>

Game *game;

int main() {
	if(!fatInitDefault()) {
		consoleDemoInit();
		printf("FAT init failed.");
		while(1)
			swiWaitForVBlank();
	}

	initGraphics();
	game = new Game();

	// Show howto if first game
	if(game->config().gamesPlayed() < 1)
		howtoMenu();

	// Loop game until returns false
	while(game->run()) {
		delete game;
		game = new Game();
	}
}
