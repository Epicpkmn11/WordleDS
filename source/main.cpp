#include "game.hpp"
#include "gfx.hpp"
#include "howto.hpp"
#include "music.hpp"
#include "settings.hpp"

#include <dirent.h>
#include <fat.h>
#include <nds.h>

int main() {
	bool fatInited = fatInitDefault();
	keysSetRepeat(25, 5);
	setBrightness(3, 16);

	mkdir("/_nds", 0777);
	mkdir("/_nds/WordleDS", 0777);
	mkdir(DATA_PATH DEFAULT_MOD, 0777);

	// Import old settings to new location
	if(access(SETTINGS_JSON_OLD, F_OK) == 0)
		Settings::legacyImport(SETTINGS_JSON_OLD);

	Gfx::init();
	settings = new Settings(SETTINGS_JSON);
	game = new Game();
	Music::music = new Music(settings->mod());
	if(settings->music())
		Music::music->start();

	Gfx::fadeIn();

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
		Gfx::fadeOut();
		delete game;
		game = new Game();
		Gfx::fadeIn();
	}

	Gfx::fadeOut();
}
