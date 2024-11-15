#include "game.hpp"
#include "font.hpp"
#include "gfx.hpp"
#include "howto.hpp"
#include "kbd.hpp"
#include "settings.hpp"
#include "tonccpy.h"
#include "wifi.hpp"

#include <algorithm>
#include <map>
#include <nds.h>
#include <time.h>

Game *game;

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

void Game::timerHandler() {
	if(game)
		game->stats().timeElapsedInc();
}

Game::Game() :
		_data(settings->mod()),
		_stats(DATA_PATH + settings->mod() + (settings->infiniteMode() ? STATS_JSON_INFINITE : STATS_JSON)),
		_kbd(_data.keyboard(), _data.letters(), _data.kbdGfx(), _data.backspaceKeyGfx(), _data.enterKeyGfx()) {
	// Get random word based on date
	_today = settings->infiniteMode() ? rand() : time(NULL) / 24 / 60 / 60;
	_answer = _data.getAnswer(_today);

	for(size_t i = 0; i < _answer.size(); i++)
		_knownPositions += u' ';

	// Create sprites
	for(size_t i = 0; i < _answer.size() * _data.maxGuesses(); i++) {
		_letterSprites.emplace_back(true, SpriteSize_32x32, SpriteColorFormat_16Color);
		_letterSprites.back().move(
			(((256 - (_answer.size() * 24 + (_answer.size() - 1) * 2)) / 2) - 4) + (i % _answer.size()) * 26,
			(25 + (167 - (_data.maxGuesses() * 24 + (_data.maxGuesses() - 1) * 2)) / 2 - 4) + (i / _answer.size()) * 26
		).gfx(_data.letterGfx(0));
	}
	Sprite::update(true);

	_data.setPalettes(settings->altPalette());

	_data.bgTop().decompressAll(BG(0));
	_data.bgBottom().decompressAll(BG_SUB(0));
	_data.popupBox().decompressAll(BG_SUB(1), BG_PALETTE_SUB + 0xD0);

	// Check if bootstub exists
	extern char *fake_heap_end;
	EnvNdsBootstubHeader *bootstub = (struct EnvNdsBootstubHeader *)fake_heap_end;
	_bootstubExists = bootstub->magic == ENV_NDS_BOOTSTUB_MAGIC;
}

Game::~Game() {
	_stats.save();
	timerStop(0);
}

bool Game::run() {
	// Reload game state from config
	if(_stats.boardState().size() > 0) {
		std::vector<TilePalette> palettes;
		for(const std::string &guess8 : _stats.boardState()) {
			std::u16string _guess = Font::utf8to16(guess8);

			Sprite *sprite = &_letterSprites[_currentGuess * _answer.size()];
			for(char16_t letter : _guess) {
				(sprite++)->palette(TilePalette::whiteDark).gfx(_data.letterGfx(_kbd.letterIndex(letter) + 1));
			}
			std::vector<TilePalette> newPalettes = check(_guess);
			palettes.reserve(palettes.size() + newPalettes.size());
			palettes.insert(palettes.end(), newPalettes.begin(), newPalettes.end());
			_won = std::find_if_not(newPalettes.begin(), newPalettes.end(), [](TilePalette a) { return a == TilePalette::green; }) == newPalettes.end();
			Sprite::update(false);

			// Save info needed for hard mode
			if(settings->hardMode()) {
				_knownLetters = u"";
				for(uint i = 0; i < _guess.size(); i++) {
					if(newPalettes[i] != TilePalette::gray)
						_knownLetters += _guess[i];
					if(newPalettes[i] == TilePalette::green)
						_knownPositions[i] = _guess[i];
				}
			}

			_currentGuess++;
			if(_currentGuess >= _data.maxGuesses())
				break;
		}
		Gfx::fadeIn(FADE_SLOW, FADE_TOP | FADE_BOTTOM);
		Gfx::flipSprites(_letterSprites.data(), _currentGuess * _answer.size(), palettes);
	}

	if(_currentGuess == _data.maxGuesses() && !_won)
		_currentGuess++;

	if(_won || _currentGuess > _data.maxGuesses())
		_statsSaved = true; // an already completed game was loaded, don't re-save

	fadeIn(FADE_SLOW, FADE_TOP | FADE_BOTTOM);

	u16 pressed, held;
	char16_t key = NOKEY;
	touchPosition touch;
	while(pmMainLoop()) {
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


			if(!_showRefresh && time(NULL) / 24 / 60 / 60 != _today) { // New day or infinite mode enabled, show refresh button
				_showRefresh = true;
			}

			if(!Gfx::popupVisible() && _showRefresh && !_data.refreshSprite().visible())
				_data.refreshSprite().visible(true).update();
		} while(pmMainLoop() && (!pressed && key == Kbd::NOKEY));

		// Process keyboard
		switch(key) {
			case Kbd::NOKEY:
				break;

			case Kbd::ENTER:
				// Ensure _guess is a choice or valid _guess
				if(std::find(_data.choices().begin(), _data.choices().end(), _guess) != _data.choices().end()
				|| std::binary_search(_data.guesses().begin(), _data.guesses().end(), _guess)) {
					// check if meets hard mode requirements
					if(settings->hardMode()) {
						char invalidMessage[64] = {0};
						for(char16_t letter : _knownLetters) {
							if(std::count(_knownLetters.begin(), _knownLetters.end(), letter) > std::count(_guess.begin(), _guess.end(), letter)) {
								sprintf(invalidMessage, _data.guessMustContainX().c_str(), Font::utf16to8(letter).c_str());
								break;
							}
						}
						for(uint i = 0; i < _knownPositions.size(); i++) {
							if(_knownPositions[i] != u' ' && _guess[i] != _knownPositions[i]) {
								sprintf(invalidMessage, _data.nthMustBeX().c_str(), i + 1, _data.numberSuffix(i + 1).c_str(), Font::utf16to8(_knownPositions[i]).c_str());
								break;
							}
						}

						if(strlen(invalidMessage) > 0) {
							if(_showRefresh)
								_data.refreshSprite().visible(false).update();
							Gfx::showPopup(invalidMessage, 120);
							break;
						}
					}

					// Find status of the letters
					std::vector<TilePalette> newPalettes = check(_guess);
					_won = std::find_if_not(newPalettes.begin(), newPalettes.end(), [](TilePalette a) { return a == TilePalette::green; }) == newPalettes.end();
					Gfx::flipSprites(&_letterSprites[_currentGuess * _answer.size()], _answer.size(), newPalettes);
					Sprite::update(false);

					// Save info needed for hard mode
					if(settings->hardMode()) {
						_knownLetters = u"";
						for(uint i = 0; i < _guess.size(); i++) {
							if(newPalettes[i] != TilePalette::gray)
								_knownLetters += _guess[i];
							if(newPalettes[i] == TilePalette::green)
								_knownPositions[i] = _guess[i];
						}
					}

					_stats
						.boardState(Font::utf16to8(_guess))
						.lastPlayed(_today)
						.save();

					_guess = u"";
					_currentGuess++;
				} else {
					if(_showRefresh)
						_data.refreshSprite().visible(false).update();
					Gfx::showPopup(_guess.size() < _answer.size() ? _data.tooShortMessage() : _data.notWordMessage(), 120);
				}
				break;

			case Kbd::BACKSPACE:
				if(_guess.size() > 0) {
					_guess.pop_back();
					_letterSprites[_currentGuess * _answer.size() + _guess.size()].palette(TilePalette::white).gfx(_data.letterGfx(0)).update();
				}
				break;

			default: // Letter
				if(_guess.size() < _answer.size()) {
					Sprite sprite = _letterSprites[_currentGuess * _answer.size() + _guess.size()];
					sprite.palette(TilePalette::whiteDark).gfx(_data.letterGfx(_kbd.letterIndex(key) + 1)).affineIndex(0, false);
					for(int i = 0; i < 6; i++) {
						swiWaitForVBlank();
						sprite.rotateScale(0, 1.1f - .1f / (6 - i), 1.1f - .1f / (6 - i)).update();
					}
					sprite.affineIndex(-1, false).update();
					_guess += key;

					// Start timer if not started yet
					if(!_timerStarted) {
						timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(1), timerHandler);
						_timerStarted = true;
					}
				}
				break;
		}

		if(pressed & KEY_START && _bootstubExists)
			return false;

		if(pressed & KEY_TOUCH) {
			if(touch.py < 24) { // One of the icons at the top
				if(_data.howtoBtn().touching(touch)) {
					fadeOut(FADE_FAST, FADE_TOP | FADE_BOTTOM);
					howtoMenu(false);
					fadeIn(FADE_FAST, FADE_TOP | FADE_BOTTOM);
				} else if(_data.statsBtn().touching(touch)) {
					fadeOut(FADE_FAST, FADE_BOTTOM);
					_stats.showMenu();
					fadeIn(FADE_FAST, FADE_BOTTOM);
				} else if(_data.choiceOrderUrl() != "" && _data.updateBtn().touching(touch)) {
					WiFi::getWords(_data.choiceOrderUrl().c_str());
					if(_answer == u"" && _data.getAnswer(_today) != u"") {
						return true;
					}
				} else if(_data.settingsBtn().touching(touch)) {
					fadeOut(FADE_FAST, FADE_BOTTOM);

					std::string loadedMod = settings->mod();
					bool loadedInfinite = settings->infiniteMode();
					settings->showMenu();

					if(settings->mod() != loadedMod || settings->infiniteMode() != loadedInfinite)
						return true;

					Gfx::fadeOut(FADE_FAST, FADE_BOTTOM);
					Font::clear(false);
					Font::update(false);

					fadeIn(FADE_FAST, FADE_BOTTOM);
				}
			} else if(_showRefresh && !Gfx::popupVisible() && (touch.py >= 36 && touch.py <= 36 + 64 && touch.px >= 96 && touch.px <= 96 + 64)) {
				// Refresh button
				if(settings->infiniteMode()) {
					if(!_won)
						_stats.streak(0);
					_stats.save();
				}
				return true;
			}
		}

		if(!_statsSaved && (_won || _currentGuess >= _data.maxGuesses())) {
			_statsSaved = true;

			if(_currentGuess == _data.maxGuesses() && !_won)
				_currentGuess++;

			_kbd.hide();

			timerStop(0);

			// Update stats
			_stats
				.completionTimes(_stats.timeElapsed())
				.lastWon(_today)
				.guessCounts(_currentGuess)
				.gamesPlayed(_stats.gamesPlayed() + 1)
				.streak(_won ? _stats.streak() + 1 : 0)
				.save();

			// Generate sharable txt
			FILE *file = fopen((DATA_PATH + settings->mod() + "/share.txt").c_str(), "w");
			if(file) {
				std::string str = _stats.shareMessage();
				fwrite(str.c_str(), 1, str.size(), file);
				fclose(file);
			}

			if(_showRefresh)
				_data.refreshSprite().visible(false).update();
			if(_won) {
				Gfx::showPopup(_data.victoryMessage(_currentGuess - 1));
				for(int i = 0; i < 180; i++)
					swiWaitForVBlank();
			} else {
				Gfx::showPopup(settings->infiniteMode() ? _data.lossMessageInfinite() : _data.lossMessage());

				std::vector<Sprite> answerSprites;
				for(uint i = 0; i < _answer.size(); i++) {
					answerSprites.emplace_back(false, SpriteSize_32x32, SpriteColorFormat_16Color);
					answerSprites.back()
						.move((((256 - (_answer.size() * 24 + (_answer.size() - 1) * 2)) / 2) - 4) + (i % _answer.size()) * 26, 96)
						.palette(TilePalette::white)
						.gfx(_data.letterGfxSub(_kbd.letterIndex(_answer[i]) + 1))
						.visible(false);
				}

				Gfx::flipSprites(answerSprites.data(), answerSprites.size(), {}, FlipOptions::show);

				for(int i = 0; i < 180; i++)
					swiWaitForVBlank();

				Gfx::flipSprites(answerSprites.data(), answerSprites.size(), {}, FlipOptions::hide);
			}

			fadeOut(FADE_FAST, FADE_BOTTOM);

			// Show stats
			_stats.showMenu();

			fadeIn(FADE_FAST, FADE_BOTTOM);
		}
	}

	return false;
}

void Game::fadeOut(int frames, int screen) {
	Gfx::fadeOut(frames, screen);
	_kbd.hide();
	if(_showRefresh)
		_data.refreshSprite().visible(false);

	_data.btnHowtoSprite().visible(false);
	_data.btnStatsSprite().visible(false);
	_data.btnUpdateSprite().visible(false);
	_data.btnSettingsSprite().visible(false);

	Sprite::update(false);

	Font::clear(false);
	Font::update(false);
}

void Game::fadeIn(int frames, int screen) {
	if(!_won && _currentGuess <= _data.maxGuesses() && _answer != u"") {
		_kbd.show();

		if(_showRefresh)
			_data.refreshSprite().visible(true);
	} else if(_answer == u"") {
		Gfx::showPopup("Update word list\nor play infinite");
	}

	if(_data.mainMenuSprites()) {
		_data.btnHowtoSprite().visible(true);
		_data.btnStatsSprite().visible(true);
		if(_data.choiceOrderUrl() != "")
			_data.btnUpdateSprite().visible(true);
		_data.btnSettingsSprite().visible(true);
	}

	Sprite::update(false);

	_data.bgTop().decompressAll(BG(0));
	_data.bgBottom().decompressAll(BG_SUB(0));
	Gfx::fadeIn(frames, screen);
}
