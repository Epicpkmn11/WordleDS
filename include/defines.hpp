#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <algorithm>
#include <array>
#include <map>
#include <string_view>

#define WORD_LEN 5
#define MAX_GUESSES 6

static_assert(WORD_LEN > 0 && WORD_LEN <= 9, "WORD_LEN must be 1-9");
static_assert(MAX_GUESSES > 0 && MAX_GUESSES <= 6, "MAX_GUESSES must be 1-6");

// Used for the number in the TXT and the index in the word list
#define FIRST_DAY 18797

// If changing the MAX_GUESSES, make sure to edit this array
constexpr std::array<std::string_view, MAX_GUESSES> victoryMessages = {
	"Genius",
	"Magnificent",
	"Impressive",
	"Splendid",
	"Great",
	"Phew"
};

constexpr std::string_view
	lossMessage = "Better luck tomorrow...\nThe answer was:",
	tooShortMessage = "Not enough letters",
	notWordMessage = "Not in word list",
	creditStr = "Wordle DS by Pk11";

constexpr const char
	*nthMustBeX = "%d%s letter must be %s",
	*guessMustContainX = "Guess must contain %s";

constexpr const char *numberSuffix(int i) {
	switch(i) {
		case 1:
			return "st";
		case 2:
			return "nd";
		case 3:
			return "rd";
		default:
			return "th";
	}
}

// Order of the letters in the tile/key images
// To add more simply increase the array size and add to the end
constexpr std::array<char16_t, 27> letters = {
	u'A',
	u'B',
	u'C',
	u'D',
	u'E',
	u'F',
	u'G',
	u'H',
	u'I',
	u'J',
	u'K',
	u'L',
	u'M',
	u'N',
	u'O',
	u'P',
	u'Q',
	u'R',
	u'S',
	u'T',
	u'U',
	u'V',
	u'W',
	u'X',
	u'Y',
	u'Z'
};

inline int letterIndex(char16_t c) {
	const char16_t *out = std::find(letters.begin(), letters.end(), c);

	return out != letters.end() ? std::distance(letters.begin(), out) : 0;
}

#endif // DEFINES_HPP
