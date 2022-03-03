#include "config.hpp"
#include "defines.hpp"
#include "font.hpp"
#include "gfx.hpp"
#include "howto.hpp"
#include "kbd.hpp"
#include "settings.hpp"
#include "stats.hpp"
#include "tonccpy.h"
#include "words.hpp"

#include "bgBottom.h"
#include "bgBottomBox.h"
#include "main_nftr.h"

#include <algorithm>
#include <fat.h>
#include <map>
#include <nds.h>
#include <stdio.h>
#include <string>
#include <time.h>

extern char *fake_heap_end;
__bootstub *bootstub = (struct __bootstub *)fake_heap_end;

constexpr u16 fontPal[] = {
	0x39CE, 0xC631, 0xF39C, 0xFFFF
};

std::vector<TilePalette> check(const std::u16string &guess, std::u16string_view answer, Kbd *kbd) {
	std::vector<TilePalette> res;
	res.resize(std::min(guess.size(), answer.size()));

	// Get map of letters for wrong location
	std::map<char16_t, int> letters;
	for(char16_t letter : answer) {
		letters[letter]++;
	}

	// First check for exact matches
	for(uint i = 0; i < res.size(); i++) {
		if(guess[i] == answer[i]) {
			res[i] = TilePalette::green;
			if(kbd)
				kbd->palette(guess[i], TilePalette::green);
			letters[guess[i]]--;
		}
	}
	// Then check for matches in the wrong location
	for(uint i = 0; i < res.size(); i++) {
		if(res[i] == TilePalette::green) {
			continue;
		} else if(letters[guess[i]] > 0) {
			res[i] = TilePalette::yellow;
			if(kbd && kbd->palette(guess[i]) != TilePalette::green)
				kbd->palette(guess[i], TilePalette::yellow);
			letters[guess[i]]--;
		} else {
			res[i] = TilePalette::gray;
			if(kbd && (kbd->palette(guess[i]) != TilePalette::yellow && kbd->palette(guess[i]) != TilePalette::green))
				kbd->palette(guess[i], TilePalette::gray);
		}
	}

	return res;
}

void makeTxt(Config &config, std::u16string_view answer) {
	FILE *file = fopen("WordleDS.txt", "w");

	if(file) {
		char str[64];
		sprintf(str, "Wordle DS %lld %c/%d%s\n\n",
			config.lastPlayed() - FIRST_DAY,
			config.guessCounts().back() > MAX_GUESSES ? 'X' : '0' + config.guessCounts().back(),
			MAX_GUESSES,
			config.hardMode() ? "*" : "");

		fwrite(str, 1, strlen(str), file);
		for(const std::string &guess : config.boardState()) {
			toncset(str, 0, 64);

			const char *green = config.altPalette() ? "🟧" : "🟩";
			const char *yellow = config.altPalette() ? "🟦" : "🟨";

			std::vector<TilePalette> colors = check(Font::utf8to16(guess), answer, nullptr);
			for(uint i = 0; i < colors.size(); i++)
				strcat(str, colors[i] == TilePalette::green ? green : (colors[i] == TilePalette::yellow ? yellow : "⬜"));

			strcat(str, "\n");

			fwrite(str, 1, strlen(str), file);
		}

		fclose(file);
	}
}

void drawBgBottom(Font &font, std::string_view msg) {
	font.clear(false);

	if(msg.size() == 0) {
		tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomTiles, bgBottomTilesLen);
		tonccpy(BG_PALETTE_SUB, bgBottomPal, bgBottomPalLen);
		tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomMap, bgBottomMapLen);
	} else {
		tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomBoxTiles, bgBottomBoxTilesLen);
		tonccpy(BG_PALETTE_SUB, bgBottomBoxPal, bgBottomBoxPalLen);
		tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomBoxMap, bgBottomBoxMapLen);

		font.print(0, 56 - font.calcHeight(msg) / 2, false, msg, Alignment::center);
	}

	font.update(false);
}

int main(void) {
	if(!fatInitDefault()) {
		consoleDemoInit();
		printf("FAT init failed.");
		while(1)
			swiWaitForVBlank();
	}

	Config config("WordleDS.json");

	initGraphics(config.altPalette());

	Font font(main_nftr, main_nftr_size);
	font.palette(0xFC);
	tonccpy(BG_PALETTE_SUB + 0xFC, fontPal, sizeof(fontPal));

	// Show howto if first game
	if(config.gamesPlayed() < 1)
		howtoMenu();

	// Get random word based on date
	time_t today = time(NULL) / 24 / 60 / 60;
	std::u16string_view answer = choices[(today - FIRST_DAY) % choices.size()];
	config.lastPlayed(today);

	Kbd kbd;

	u16 pressed, held;
	char16_t key = NOKEY;
	touchPosition touch;
	std::u16string guess = u"";
	int currentGuess = 0;
	int popupTimeout = -1;
	bool won = false, statsSaved = false;
	std::u16string knownLetters, knownPositions; // for hard mode
	for(int i = 0; i < WORD_LEN; i++)
		knownPositions += u' ';

	if(config.boardState().size() > 0) {
		std::vector<TilePalette> palettes;
		for(const std::string &guess8 : config.boardState()) {
			std::u16string guess = Font::utf8to16(guess8);

			Sprite *sprite = &letterSprites[currentGuess * WORD_LEN];
			for(char letter : guess) {
				(sprite++)->palette(TilePalette::whiteDark).gfx(letterGfx[letterIndex(letter) + 1]);
			}
			std::vector<TilePalette> newPalettes = check(guess, answer, &kbd);
			palettes.reserve(palettes.size() + newPalettes.size());
			palettes.insert(palettes.end(), newPalettes.begin(), newPalettes.end());
			won = std::find_if_not(newPalettes.begin(), newPalettes.end(), [](TilePalette a) { return a == TilePalette::green; }) == newPalettes.end();
			Sprite::update(false);
			currentGuess++;
			if(currentGuess >= MAX_GUESSES)
				break;
		}
		flipSprites(letterSprites.data(), currentGuess * WORD_LEN, palettes);
	}

	if(currentGuess == MAX_GUESSES && !won)
		currentGuess++;

	if(!won && currentGuess <= MAX_GUESSES)
		kbd.show();
	else
		statsSaved = true; // an already completed game was loaded, don't re-save

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
			touchRead(&touch);
			if(held & KEY_TOUCH)
				key = kbd.get();
			else
				key = Kbd::NOKEY;

			if(popupTimeout == 0) {
				// Reset to no message box
				drawBgBottom(font, "");
			}
			if(popupTimeout >= 0)
				popupTimeout--;
		} while(!pressed && key == Kbd::NOKEY);

		// Process keyboard
		switch(key) {
			case Kbd::NOKEY:
				break;
			case Kbd::ENTER:
				// Ensure guess is a choice or valid guess
				if(std::find(choices.begin(), choices.end(), guess) != choices.end()
				|| std::binary_search(guesses.begin(), guesses.end(), guess)) {
					// check if meets hard mode requirements
					if(config.hardMode()) {
						char invalidMessage[64] = {0};
						for(char16_t letter : knownLetters) {
							if(std::count(knownLetters.begin(), knownLetters.end(), letter) != std::count(guess.begin(), guess.end(), letter)) {
								sprintf(invalidMessage, guessMustContainX, Font::utf16to8(letter).c_str());
								break;
							}
						}
						for(uint i = 0; i < knownPositions.size(); i++) {
							if(knownPositions[i] != u' ' && guess[i] != knownPositions[i]) {
								sprintf(invalidMessage, nthMustBeX, i + 1, numberSuffix(i + 1), Font::utf16to8(knownPositions[i]).c_str());
								break;
							}
						}

						if(strlen(invalidMessage) > 0) {
							drawBgBottom(font, invalidMessage);
							popupTimeout = 120;
							break;
						}
					}

					// Find status of the letters
					std::vector<TilePalette> newPalettes = check(guess, answer, &kbd);
					won = std::find_if_not(newPalettes.begin(), newPalettes.end(), [](TilePalette a) { return a == TilePalette::green; }) == newPalettes.end();
					flipSprites(&letterSprites[currentGuess * WORD_LEN], WORD_LEN, newPalettes);
					Sprite::update(false);

					// Save info needed for hard mode
					if(config.hardMode()) {
						knownLetters = u"";
						for(uint i = 0; i < guess.size(); i++) {
							if(newPalettes[i] == TilePalette::yellow)
								knownLetters += guess[i];
							else if(newPalettes[i] == TilePalette::green)
								knownPositions[i] = guess[i];
						}
					}

					config.boardState(Font::utf16to8(guess));

					guess = u"";
					currentGuess++;
				} else {
					drawBgBottom(font, guess.length() < WORD_LEN ? tooShortMessage : notWordMessage);
					popupTimeout = 120;
				}
				break;
			case Kbd::BACKSPACE:
				if(guess.length() > 0) {
					guess.pop_back();
					letterSprites[currentGuess * WORD_LEN + guess.length()].palette(TilePalette::white).gfx(letterGfx[0]).update();
				}
				break;
			default: // Letter
				if(guess.length() < WORD_LEN) {
					Sprite sprite = letterSprites[currentGuess * WORD_LEN + guess.length()];
					sprite.palette(TilePalette::whiteDark).gfx(letterGfx[letterIndex(key) + 1]).affineIndex(0, false);
					for(int i = 0; i < 6; i++) {
						swiWaitForVBlank();
						sprite.rotateScale(0, 1.1f - .1f / (6 - i), 1.1f - .1f / (6 - i)).update();
					}
					sprite.affineIndex(-1, false).update();
					guess += key;
				}
				break;
		}

		if(pressed & KEY_START && (bootstub->bootsig == BOOTSIG)) {
			config.save();
			return 0;
		}

		if(pressed & KEY_TOUCH) {
			if(touch.py < 24) { // One of the icons at the top
				bool showKeyboard = kbd.visible();
				if(showKeyboard)
					kbd.hide();

				if(touch.px < 24)
					howtoMenu();
				else if(touch.px > 116 && touch.px < 140)
					statsMenu(config, won);
				else if(touch.px > 232)
					settingsMenu(config);

				if(showKeyboard)
					kbd.show();
			}
		}

		if(!statsSaved && (won || currentGuess >= MAX_GUESSES)) {
			statsSaved = true;

			if(currentGuess == MAX_GUESSES && !won)
				currentGuess++;

			kbd.hide();

			// Update stats
			config.guessCounts(currentGuess);
			config.gamesPlayed(config.gamesPlayed() + 1);
			config.streak(config.streak() + 1);
			config.save();

			// Generate sharable txt
			makeTxt(config, answer);

			if(won) {
				drawBgBottom(font, victoryMessages[currentGuess - 1]);
				for(int i = 0; i < 180; i++)
					swiWaitForVBlank();
			} else {
				drawBgBottom(font, lossMessage);

				std::vector<Sprite> answerSprites;
				for(uint i = 0; i < answer.length(); i++) {
					answerSprites.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
					answerSprites.back()
						.move((((256 - (WORD_LEN * 24 + (WORD_LEN - 1) * 2)) / 2) - 4) + (i % WORD_LEN) * 26, 96)
						.palette(TilePalette::white)
						.gfx(letterGfxSub[letterIndex(answer[i]) + 1])
						.visible(false);
				}

				flipSprites(answerSprites.data(), answerSprites.size(), {}, FlipOptions::show);

				for(int i = 0; i < 180; i++)
					swiWaitForVBlank();

				flipSprites(answerSprites.data(), answerSprites.size(), {}, FlipOptions::hide);
			}
			font.clear(false).update(false);

			// Show stats
			statsMenu(config, won);
		}
	}
}
