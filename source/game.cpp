#include "game.hpp"
#include "font.hpp"
#include "gfx.hpp"
#include "howto.hpp"
#include "kbd.hpp"
#include "settings.hpp"
#include "tonccpy.h"

#include <algorithm>
#include <map>
#include <nds.h>
#include <time.h>

Game *game;

void Game::drawBgBottom(std::string_view msg, int timeout = -1) {
	swiWaitForVBlank();

	Font::clear(false);

	if(msg.size() == 0) {
		_data.bgBottom()
			.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
			.decompressMap(bgGetMapPtr(BG_SUB(0)))
			.decompressPal(BG_PALETTE_SUB);
	} else {
		_data.bgBottomBox()
			.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
			.decompressMap(bgGetMapPtr(BG_SUB(0)))
			.decompressPal(BG_PALETTE_SUB);

		_data.mainFont().palette(TEXT_WHITE).print(0, 56 - _data.mainFont().calcHeight(msg) / 2, false, msg, Alignment::center);
	}

	Font::update(false);

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

Game::Game() :
		_stats(DATA_PATH + settings->mod() + STATS_JSON),
		_data(settings->mod()),
		_kbd(_data.keyboard(), _data.letters(), _data.kbdGfx(), _data.backspaceKeyGfx(), _data.enterKeyGfx()) {
	// Get random word based on date
	_today = time(NULL) / 24 / 60 / 60;
	_answer = _data.choices((unsigned int)(_today - _data.firstDay()) % _data.choices().size());

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

	_data.bgTop()
		.decompressTiles(bgGetGfxPtr(BG(0)))
		.decompressMap(bgGetMapPtr(BG(0)))
		.decompressPal(BG_PALETTE);
	_data.bgBottom()
		.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
		.decompressMap(bgGetMapPtr(BG_SUB(0)))
		.decompressPal(BG_PALETTE_SUB);

	// Check if bootstub exists
	extern char *fake_heap_end;
	__bootstub *bootstub = (struct __bootstub *)fake_heap_end;
	_bootstubExists = bootstub->bootsig == BOOTSIG;
}

Game::~Game() {
	_stats.save();
}

bool Game::run() {
	// Reload game state from config
	if(_stats.boardState().size() > 0) {
		std::vector<TilePalette> palettes;
		for(const std::string &guess8 : _stats.boardState()) {
			std::u16string _guess = Font::utf8to16(guess8);

			Sprite *sprite = &_letterSprites[_currentGuess * _answer.size()];
			for(char letter : _guess) {
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
		Gfx::flipSprites(_letterSprites.data(), _currentGuess * _answer.size(), palettes);
	}

	if(_currentGuess == _data.maxGuesses() && !_won)
		_currentGuess++;

	if(_won || _currentGuess > _data.maxGuesses())
		_statsSaved = true; // an already completed game was loaded, don't re-save

	if(!_won && _currentGuess <= _data.maxGuesses())
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
					_data.refreshSprite().visible(true).update();
			}
			if(_popupTimeout >= 0) {
				_popupTimeout--;
			}

			if(!_showRefresh && time(NULL) / 24 / 60 / 60 != _today) { // New day, show refresh button
				_showRefresh = true;
				_data.refreshSprite().visible(true).update();
			}
		} while(!pressed && key == Kbd::NOKEY);

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
							drawBgBottom(invalidMessage, 120);
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
						.timeElapsed(time(NULL) - _startTime)
						.save();

					_guess = u"";
					_currentGuess++;
				} else {
					if(_showRefresh)
						_data.refreshSprite().visible(false).update();
					drawBgBottom(_guess.size() < _answer.size() ? _data.tooShortMessage() : _data.notWordMessage(), 120);
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
					if(!_startTime)
						_startTime = time(NULL) - _stats.timeElapsed();
				}
				break;
		}

		if(pressed & KEY_START && _bootstubExists)
			return false;

		if(pressed & KEY_TOUCH) {
			if(touch.py < 24) { // One of the icons at the top
				swiWaitForVBlank();

				bool showKeyboard = _kbd.visible();
				if(showKeyboard)
					_kbd.hide();
				if(_showRefresh)
					_data.refreshSprite().visible(false).update();
				Font::clear(false);
				Font::update(false);

				if(touch.px < 24) {
					howtoMenu();
				} else if(touch.px > 116 && touch.px < 140) {
					_stats.showMenu();
				} else if(touch.px > 232) {
					std::string loadedMod = settings->mod();
					settings->showMenu();

					if(settings->mod() != loadedMod)
						return true;
				}

				if(showKeyboard)
					_kbd.show();
				if(_showRefresh)
					_data.refreshSprite().visible(true).update();

				// Restore normal background
				swiWaitForVBlank();
				_data.bgTop()
					.decompressTiles(bgGetGfxPtr(BG(0)))
					.decompressMap(bgGetMapPtr(BG(0)))
					.decompressPal(BG_PALETTE);
				_data.bgBottom()
					.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
					.decompressMap(bgGetMapPtr(BG_SUB(0)))
					.decompressPal(BG_PALETTE_SUB);
			} else if(_showRefresh && _popupTimeout == -1 && (touch.py >= 36 && touch.py <= 36 + 64 && touch.px >= 96 && touch.px <= 96 + 64)) {
				// Refresh button
				return true;
			}
		}

		if(!_statsSaved && (_won || _currentGuess >= _data.maxGuesses())) {
			_statsSaved = true;

			if(_currentGuess == _data.maxGuesses() && !_won)
				_currentGuess++;

			_kbd.hide();

			// Update stats
			_stats
				.timeElapsed(time(NULL) - _startTime)
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
				drawBgBottom(_data.victoryMessage(_currentGuess - 1));
				for(int i = 0; i < 180; i++)
					swiWaitForVBlank();
			} else {
				drawBgBottom(_data.lossMessage());

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
			Font::clear(false);
			Font::update(false);

			// Show stats
			_stats.showMenu();

			if(_showRefresh)
				_data.refreshSprite().visible(true).update();

			// Restore normal background
			swiWaitForVBlank();
			_data.bgBottom()
				.decompressTiles(bgGetGfxPtr(BG_SUB(0)))
				.decompressMap(bgGetMapPtr(BG_SUB(0)))
				.decompressPal(BG_PALETTE_SUB);
		}
	}

	return false;
}
