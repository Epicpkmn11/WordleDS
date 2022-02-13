#include "gfx.hpp"
#include "words.hpp"

#include <map>
#include <nds.h>
#include <stdio.h>
#include <string>
#include <time.h>

enum class LetterStatus {
	wrong,
	wrongLoc,
	correct
};

std::vector<LetterStatus> check(const std::string &guess, const std::string &answer) {
	std::vector<LetterStatus> res;
	res.resize(std::min(guess.size(), answer.size()));

	// Get map of letters for wrong location
	std::map<char, int> letters;
	for(char letter : answer) {
		letters[letter]++;
	}

	// First check for exact matches
	for(uint i = 0; i < res.size(); i++) {
		if(guess[i] == answer[i]) {
			res[i] = LetterStatus::correct;
			letters[guess[i]]--;
		}
	}
	// Then check for matches in the wrong location
	for(uint i = 0; i < res.size(); i++) {
		if(res[i] == LetterStatus::correct) {
			continue;
		} else if(letters[guess[i]] > 0) {
			res[i] = LetterStatus::wrongLoc;
			letters[guess[i]]--;
		} else {
			res[i] = LetterStatus::wrong;
		}
	}

	return res;
}

int searchPredicate(const void *a, const void *b) {
	return strcasecmp((const char *)a, *(const char **)b);
}

int main(void) {
	initGraphics();

	// Get random word based on date
	time_t day = time(NULL) / 24 / 60 / 60;
	srand(day);
	std::string answer = choices[rand() % choices.size()];

	keyboardDemoInit();
	keyboardShow();

	u16 pressed;
	s8 key = NOKEY;
	std::string guess = "";
	int currentGuess = 0;
	bool won = false;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			key = tolower(keyboardUpdate());
		} while(!pressed && key == NOKEY);

		// Process keyboard
		switch(key) {
			case NOKEY:
			case DVK_TAB:
			case DVK_MENU:
			case DVK_SHIFT:
			case DVK_CAPS:
			case DVK_CTRL:
			case DVK_UP:
			case DVK_RIGHT:
			case DVK_DOWN:
			case DVK_LEFT:
			case DVK_FOLD:
			case DVK_ALT:
				break;
			case DVK_ENTER:
				// Ensure guess is a choice or valid guess
				if(bsearch(guess.c_str(), choices.data(), choices.size(), sizeof(choices[0]), searchPredicate) != nullptr || bsearch(guess.c_str(), guesses.data(), guesses.size(), sizeof(guesses[0]), searchPredicate) != nullptr) {
					// Find status of the letters
					std::vector<LetterStatus> status = check(guess, answer);

					// Print word with colors
					won = true;
					for(int i = 0; i < WORD_LEN; i++) {
						Sprite &sprite = sprites[currentGuess * WORD_LEN + i];
						sprite.affineIndex(0, false);
						for(int j = 0; j < 15; j++) {
							swiWaitForVBlank();
							sprite.affineTransform(1.0f, 0.0f, 0.0f, 1.0f - (j * (1.0f / 15.0f)) + 0.0001f).update();
						}
						sprite.palette(status[i] == LetterStatus::correct ? TilePalette::green : (status[i] == LetterStatus::wrongLoc ? TilePalette::yellow : TilePalette::gray)).update();
						for(int j = 0; j < 15; j++) {
							swiWaitForVBlank();
							sprite.affineTransform(1.0f, 0.0f, 0.0f, 1.0f - ((15 - j) * (1.0f / 15.0f)) + 0.0001f).update();
						}
						sprite.affineIndex(-1, false).update();

						if(status[i] != LetterStatus::correct)
							won = false;
					}

					guess = "";
					currentGuess++;
				} else {
					if(guess.length() < 5)
						printf("\x1B[41mNot enough letters.\x1B[47m\n");
					else
						printf("\x1B[41mNot in word list.\x1B[47m\n");
				}
				break;
			case DVK_BACKSPACE:
				if(guess.length() > 0) {
					guess.pop_back();
					sprites[currentGuess * WORD_LEN + guess.length()].palette(TilePalette::white).gfx(letterGfx[0]).update();
				}
				break;
			default: // Letter
				if(key >= 'a' && key <= 'z' && guess.length() < WORD_LEN) {
					Sprite sprite = sprites[currentGuess * WORD_LEN + guess.length()];
					sprite.palette(TilePalette::whiteDark).gfx(letterGfx[key - 'a' + 1]).affineIndex(0, false);
					for(int i = 0; i < 6; i++) {
						swiWaitForVBlank();
						sprite.rotateScale(0, 1.1f - .1f / (6 - i), 1.1f - .1f / (6 - i)).update();
					}
					sprite.affineIndex(-1, false).update();
					guess += key;
				}
				break;
		}

		// Break loop if game won
		if(won || currentGuess >= MAX_GUESSES)
			break;

		if(pressed & KEY_START)
			return 0;
	}

	keyboardHide();

	if(won) {
		printf("\nCongrats!\n");
	} else {
		printf("\nBetter luck tomorrow :'(\n");
		printf("\nThe correct answer was: %s\n", answer.c_str());
	}

	while(!(keysDown() & KEY_START)) {
		swiWaitForVBlank();
		scanKeys();
	}
}
