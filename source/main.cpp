#include "game.hpp"
#include "gfx.hpp"
#include "howto.hpp"
#include "settings.hpp"

#include <dirent.h>
#include <fat.h>
#include <nds.h>

#include "soundbank.h"
#include "soundbank_bin.h"

int main() {
	bool fatInited = fatInitDefault();
	keysSetRepeat(25, 5);

	mkdir("/_nds", 0777);
	mkdir("/_nds/WordleDS", 0777);
	mkdir(DATA_PATH DEFAULT_MOD, 0777);

	// Import old settings to new location
	if(access(SETTINGS_JSON_OLD, F_OK) == 0)
		Settings::legacyImport(SETTINGS_JSON_OLD);

	Gfx::init();
	settings = new Settings(SETTINGS_JSON);

	game = new Game();

	// Show howto if first game
	if(game->stats().firstPlay())
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
