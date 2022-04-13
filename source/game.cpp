#include "game.hpp"
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

#include <algorithm>
#include <map>
#include <nds.h>
#include <time.h>

void Game::drawBgBottom(std::string_view msg, int timeout = -1) {
	swiWaitForVBlank();

	mainFont.clear(false);

	if(msg.size() == 0) {
		tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomTiles, bgBottomTilesLen);
		tonccpy(BG_PALETTE_SUB, bgBottomPal, bgBottomPalLen);
		tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomMap, bgBottomMapLen);
	} else {
		tonccpy(bgGetGfxPtr(BG_SUB(0)), bgBottomBoxTiles, bgBottomBoxTilesLen);
		tonccpy(BG_PALETTE_SUB, bgBottomBoxPal, bgBottomBoxPalLen);
		tonccpy(bgGetMapPtr(BG_SUB(0)), bgBottomBoxMap, bgBottomBoxMapLen);

		mainFont.palette(TEXT_WHITE).print(0, 56 - mainFont.calcHeight(msg) / 2, false, msg, Alignment::center);
	}

	mainFont.update(false);

	if(timeout != -1)
		_popupTimeout = timeout;
}

std::vector<TilePalette> Game::check(const std::u16string &_guess) {
	std::vector<TilePalette> res;
	res.resize(std::min(_guess.size(), _answer.size()));

	// Get map of letters for wrong location
	std::map<char16_t, int> letters;
	for(char16_t letter : _answer) {
		letters[letter]++;
	}

	// First check for exact matches
	for(uint i = 0; i < res.size(); i++) {
		if(_guess[i] == _answer[i]) {
			res[i] = TilePalette::green;
			_kbd.palette(_guess[i], TilePalette::green);
			letters[_guess[i]]--;
		}
	}
	// Then check for matches in the wrong location
	for(uint i = 0; i < res.size(); i++) {
		if(res[i] == TilePalette::green) {
			continue;
		} else if(letters[_guess[i]] > 0) {
			res[i] = TilePalette::yellow;
			if(_kbd.palette(_guess[i]) != TilePalette::green)
				_kbd.palette(_guess[i], TilePalette::yellow);
			letters[_guess[i]]--;
		} else {
			res[i] = TilePalette::gray;
			if(_kbd.palette(_guess[i]) != TilePalette::yellow && _kbd.palette(_guess[i]) != TilePalette::green)
				_kbd.palette(_guess[i], TilePalette::gray);
		}
	}

	return res;
}

std::string Game::shareMessage() {
	char str[256];

	sprintf(str, APP_NAME " %lld %c/%d%s\n\n",
		_config.lastPlayed() - FIRST_DAY,
		_config.guessCounts().back() > MAX_GUESSES ? 'X' : '0' + _config.guessCounts().back(),
		MAX_GUESSES,
		_config.hardMode() ? "*" : "");

	const char *green = _config.altPalette() ? "🟧" : "🟩";
	const char *yellow = _config.altPalette() ? "🟦" : "🟨";

	for(const std::string &_guess : _config.boardState()) {
		std::vector<TilePalette> colors = check(Font::utf8to16(_guess));
		for(uint i = 0; i < colors.size(); i++)
			strcat(str, colors[i] == TilePalette::green ? green : (colors[i] == TilePalette::yellow ? yellow : "⬜"));

		strcat(str, "\n");
	}

	return str;
}

Game::Game() : _config(CONFIG_PATH) {
	// Get random word based on date
	_today = time(NULL) / 24 / 60 / 60;
	_answer = choices[(_today - FIRST_DAY) % choices.size()];

	for(int i = 0; i < WORD_LEN; i++)
		_knownPositions += u' ';

	setPalettes(_config.altPalette());

	// Check if bootstub exists
	extern char *fake_heap_end;
	__bootstub *bootstub = (struct __bootstub *)fake_heap_end;
	_bootstubExists = bootstub->bootsig == BOOTSIG;

	// Reload game state from _config
	if(_config.boardState().size() > 0) {
		std::vector<TilePalette> palettes;
		for(const std::string &guess8 : _config.boardState()) {
			std::u16string _guess = Font::utf8to16(guess8);

			Sprite *sprite = &letterSprites[_currentGuess * WORD_LEN];
			for(char letter : _guess) {
				(sprite++)->palette(TilePalette::whiteDark).gfx(letterGfx[letterIndex(letter) + 1]);
			}
			std::vector<TilePalette> newPalettes = check(_guess);
			palettes.reserve(palettes.size() + newPalettes.size());
			palettes.insert(palettes.end(), newPalettes.begin(), newPalettes.end());
			_won = std::find_if_not(newPalettes.begin(), newPalettes.end(), [](TilePalette a) { return a == TilePalette::green; }) == newPalettes.end();
			Sprite::update(false);

			// Save info needed for hard mode
			if(_config.hardMode()) {
				_knownLetters = u"";
				for(uint i = 0; i < _guess.size(); i++) {
					if(newPalettes[i] != TilePalette::gray)
						_knownLetters += _guess[i];
					if(newPalettes[i] == TilePalette::green)
						_knownPositions[i] = _guess[i];
				}
			}

			_currentGuess++;
			if(_currentGuess >= MAX_GUESSES)
				break;
		}
		flipSprites(letterSprites.data(), _currentGuess * WORD_LEN, palettes);
	}

	if(_currentGuess == MAX_GUESSES && !_won)
		_currentGuess++;

	if(_won || _currentGuess > MAX_GUESSES)
		_statsSaved = true; // an already completed game was loaded, don't re-save
}

Game::~Game() {
	for(int i = 0; i < WORD_LEN * MAX_GUESSES; i++)
		letterSprites[i].palette(TilePalette::white).gfx(letterGfx[0]);
	Sprite::update(true);

	refreshSprite->visible(false).update();

	_config.save();
}

bool Game::run() {
	if(!_won && _currentGuess <= MAX_GUESSES)
		_kbd.show();

	u16 pressed, held;
	char16_t key = NOKEY;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
			touchRead(&touch);
			if(held & KEY_TOUCH)
				key = _kbd.get();
			else
				key = Kbd::NOKEY;

			if(_popupTimeout == 0) { // Reset to no message box
				drawBgBottom("");
				if(_showRefresh)
					refreshSprite->visible(true).update();
			}
			if(_popupTimeout >= 0) {
				_popupTimeout--;
			}

			if(!_showRefresh && time(NULL) / 24 / 60 / 60 != _today) { // New day, show refresh button
				_showRefresh = true;
				refreshSprite->visible(true).update();
			}
		} while(!pressed && key == Kbd::NOKEY);

		// Process keyboard
		switch(key) {
			case Kbd::NOKEY:
				break;
			case Kbd::ENTER:
				// Ensure _guess is a choice or valid _guess
				if(std::find(choices.begin(), choices.end(), _guess) != choices.end()
				|| std::binary_search(guesses.begin(), guesses.end(), _guess)) {
					// check if meets hard mode requirements
					if(_config.hardMode()) {
						char invalidMessage[64] = {0};
						for(char16_t letter : _knownLetters) {
							if(std::count(_knownLetters.begin(), _knownLetters.end(), letter) > std::count(_guess.begin(), _guess.end(), letter)) {
								sprintf(invalidMessage, guessMustContainX, Font::utf16to8(letter).c_str());
								break;
							}
						}
						for(uint i = 0; i < _knownPositions.size(); i++) {
							if(_knownPositions[i] != u' ' && _guess[i] != _knownPositions[i]) {
								sprintf(invalidMessage, nthMustBeX, i + 1, numberSuffix(i + 1), Font::utf16to8(_knownPositions[i]).c_str());
								break;
							}
						}

						if(strlen(invalidMessage) > 0) {
							if(_showRefresh)
								refreshSprite->visible(false).update();
							drawBgBottom(invalidMessage, 120);
							break;
						}
					}

					// Find status of the letters
					std::vector<TilePalette> newPalettes = check(_guess);
					_won = std::find_if_not(newPalettes.begin(), newPalettes.end(), [](TilePalette a) { return a == TilePalette::green; }) == newPalettes.end();
					flipSprites(&letterSprites[_currentGuess * WORD_LEN], WORD_LEN, newPalettes);
					Sprite::update(false);

					// Save info needed for hard mode
					if(_config.hardMode()) {
						_knownLetters = u"";
						for(uint i = 0; i < _guess.size(); i++) {
							if(newPalettes[i] != TilePalette::gray)
								_knownLetters += _guess[i];
							if(newPalettes[i] == TilePalette::green)
								_knownPositions[i] = _guess[i];
						}
					}

					_config.boardState(Font::utf16to8(_guess));

					_guess = u"";
					_currentGuess++;
				} else {
					if(_showRefresh)
						refreshSprite->visible(false).update();
					drawBgBottom(_guess.length() < WORD_LEN ? tooShortMessage : notWordMessage, 120);
				}
				break;
			case Kbd::BACKSPACE:
				if(_guess.length() > 0) {
					_guess.pop_back();
					letterSprites[_currentGuess * WORD_LEN + _guess.length()].palette(TilePalette::white).gfx(letterGfx[0]).update();
				}
				break;
			default: // Letter
				if(_guess.length() < WORD_LEN) {
					Sprite sprite = letterSprites[_currentGuess * WORD_LEN + _guess.length()];
					sprite.palette(TilePalette::whiteDark).gfx(letterGfx[letterIndex(key) + 1]).affineIndex(0, false);
					for(int i = 0; i < 6; i++) {
						swiWaitForVBlank();
						sprite.rotateScale(0, 1.1f - .1f / (6 - i), 1.1f - .1f / (6 - i)).update();
					}
					sprite.affineIndex(-1, false).update();
					_guess += key;
				}
				break;
		}

		if(pressed & KEY_START && _bootstubExists)
			return false;

		if(pressed & KEY_TOUCH) {
			if(touch.py < 24) { // One of the icons at the top
				bool showKeyboard = _kbd.visible();
				if(showKeyboard)
					_kbd.hide();
				if(_showRefresh)
					refreshSprite->visible(false).update();
				mainFont.clear(false).update(false);

				if(touch.px < 24)
					howtoMenu();
				else if(touch.px > 116 && touch.px < 140)
					statsMenu(_config, _won);
				else if(touch.px > 232)
					settingsMenu(_config);

				if(showKeyboard)
					_kbd.show();
				if(_showRefresh)
					refreshSprite->visible(true).update();
			} else if(_showRefresh && _popupTimeout == -1 && (touch.py >= 36 && touch.py <= 36 + 64 && touch.px >= 96 && touch.px <= 96 + 64)) {
				// Refresh button
				return true;
			}
		}

		if(!_statsSaved && (_won || _currentGuess >= MAX_GUESSES)) {
			_statsSaved = true;

			if(_currentGuess == MAX_GUESSES && !_won)
				_currentGuess++;

			_kbd.hide();

			// Update stats
			_config.lastPlayed(_today);
			_config.guessCounts(_currentGuess);
			_config.gamesPlayed(_config.gamesPlayed() + 1);
			_config.streak(_won ? _config.streak() + 1 : 0);
			_config.save();

			// Generate sharable txt
			FILE *file = fopen(SCORE_PATH, "w");
			if(file) {
				std::string str = shareMessage();
				fwrite(str.c_str(), 1, str.size(), file);
				fclose(file);
			}

			if(_showRefresh)
				refreshSprite->visible(false).update();
			if(_won) {
				drawBgBottom(victoryMessages[_currentGuess - 1]);
				for(int i = 0; i < 180; i++)
					swiWaitForVBlank();
			} else {
				drawBgBottom(lossMessage);

				std::vector<Sprite> answerSprites;
				for(uint i = 0; i < _answer.length(); i++) {
					answerSprites.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
					answerSprites.back()
						.move((((256 - (WORD_LEN * 24 + (WORD_LEN - 1) * 2)) / 2) - 4) + (i % WORD_LEN) * 26, 96)
						.palette(TilePalette::white)
						.gfx(letterGfxSub[letterIndex(_answer[i]) + 1])
						.visible(false);
				}

				flipSprites(answerSprites.data(), answerSprites.size(), {}, FlipOptions::show);

				for(int i = 0; i < 180; i++)
					swiWaitForVBlank();

				flipSprites(answerSprites.data(), answerSprites.size(), {}, FlipOptions::hide);
			}
			if(_showRefresh)
				refreshSprite->visible(true).update();
			mainFont.clear(false).update(false);

			// Show stats
			statsMenu(_config, _won);
		}
	}

	return false;
}
