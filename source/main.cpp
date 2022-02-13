#include "words.hpp"

#include <map>
#include <nds.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

#define WORD_LEN 5
#define MAX_GUESSES 6

enum class Status {
	wrong,
	wrongLoc,
	correct
};

std::vector<Status> check(const std::string &guess, const std::string &answer) {
	std::vector<Status> res;

	// Get map of letters for wrong location
	std::map<char, int> letters;
	for(char letter : answer) {
		letters[letter]++;
	}

	// Scan through word and build vector of letter statuses
	for(uint i = 0; i < guess.length() && i < answer.length(); i++) {
		if(guess[i] == answer[i]) {
			res.push_back(Status::correct);
			letters[guess[i]]--;
		} else if(letters[guess[i]] > 0) {
			res.push_back(Status::wrongLoc);
			letters[guess[i]]--;
		} else {
			res.push_back(Status::wrong);
		}
	}

	return res;
}

int searchPredicate(const void *a, const void *b) {
	return strcasecmp((const char *)a, *(const char **)b);
}

int main(void) {
	consoleDemoInit();

	iprintf("\t\t\t\tWordle DS\n\n>");

	time_t day = time(NULL) / 24 / 60 / 60;
	srand(day);
	std::string answer = choices[rand() % CHOICES_LEN];

	keyboardDemoInit();
	keyboardShow();

	u16 pressed;
	s8 key = NOKEY;
	std::string guess = "";
	int currentGuess = 0;
	bool done = false;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			key = keyboardUpdate();
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
			case DVK_BACKSPACE:
				if(guess.length() > 0) {
					guess.pop_back();
					printf("\x1B[%d;0H", 2 + currentGuess); // Move to line
					printf(">%-31s\n", guess.c_str()); // Print guess
				}
				break;
			case DVK_ENTER:
				{
					// Ensure guess is a choice or valid guess
					if(bsearch(guess.c_str(), choices, CHOICES_LEN, sizeof(choices[0]), searchPredicate) != nullptr || bsearch(guess.c_str(), guesses, GUESSES_LEN, sizeof(guesses[0]), searchPredicate) != nullptr) {
						// Find status of the letters
						std::vector<Status> status = check(guess, answer);

						// Print word with colors
						done = true;
						printf("\x1B[%d;0H%*c\x1B[%d;0H", 2 + currentGuess, WORD_LEN + 1, ' ', 2 + currentGuess); // Move to line, clear guess
						for(int i = 0; i < WORD_LEN; i++) {
							printf("\x1B[%02om%c", status[i] == Status::correct ? 042 : (status[i] == Status::wrongLoc ? 043 : 040), guess[i]);
							if(status[i] != Status::correct)
								done = false;
						}
						printf("\x1B[47m\n"); // Reset to white
						guess = "";
						currentGuess++;

						if(!done && currentGuess < MAX_GUESSES)
							printf(">");
					} else {
						printf("\x1B[%d;7H", 2 + currentGuess); // Move to line
						if(guess.length() < WORD_LEN)
							printf("\x1B[41m[Must have 5 letters]\x1B[47m");
						else
							printf("\x1B[41m[Invalid word]\x1B[47m");
					}
				}
				break;
			default: // Letter
				if(guess.length() < WORD_LEN) {
					guess += tolower(key);
					printf("\x1B[%d;0H", 2 + currentGuess); // Move to line
					printf(">%-31s\n", guess.c_str()); // Print guess
				}
				break;
		}

		// Break loop if game done
		if(done || currentGuess >= MAX_GUESSES)
			break;

		if(pressed & KEY_START)
			return 0;
	}

	keyboardHide();

	if(done) {
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
