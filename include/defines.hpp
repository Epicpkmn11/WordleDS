#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <array>
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
	notWordMessage = "Not in word list";

constexpr const char
	*nthMustBeX = "%d%s letter must be %c",
	*guessMustContainX = "Guess must contain %c";

inline const char *numberSuffix(int i) {
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

#endif // DEFINES_HPP
