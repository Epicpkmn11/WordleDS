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

	if(fatInited) {
		mkdir("/_nds", 0777);
		mkdir("/_nds/WordleDS", 0777);
		mkdir(DATA_PATH DEFAULT_MOD, 0777);
	}

	// Import old settings to new location
	if(access(SETTINGS_JSON_OLD, F_OK) == 0)
		Settings::legacyImport(SETTINGS_JSON_OLD);

	srand(time(NULL));

	Gfx::init();
	settings = new Settings(SETTINGS_JSON, fatInited);
	game = new Game();
	Music::music = new Music(settings->mod());
	if(settings->music())
		Music::music->start();

	// Show howto if first game
	if(game->stats().firstPlay())
		howtoMenu(true);

	if(!fatInited)
		Gfx::showPopup("FAT init failed\nStats cannot be saved", 240);
	game->data().bgBottom().decompressAll(BG_SUB(0));
	game->data().bgTop().decompressAll(BG(0));
	Gfx::fadeIn(game->stats().firstPlay() ? FADE_FAST : FADE_SLOW, FADE_TOP | FADE_BOTTOM);

	// Loop game until returns false
	while(game->run()) {
		Gfx::fadeOut(FADE_SLOW, FADE_TOP | FADE_BOTTOM);
		Font::clear(false);
		Font::update(false);
		delete game;
		game = new Game();
	}

	Gfx::fadeOut(FADE_SLOW, FADE_TOP | FADE_BOTTOM);
}
