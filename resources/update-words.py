#!/usr/bin/env python3

from argparse import ArgumentParser, FileType
from datetime import datetime, timezone
from requests import get
from bs4 import BeautifulSoup

import json
import re


def scrape_answers(start):
	"""Scrapes the upcoming answers"""

	day = start
	words = []

	while True:
		date = datetime.fromtimestamp(1624078800 + day * 60 * 60 * 24, timezone.utc)
		day += 1
		json = get(date.strftime("https://www.nytimes.com/svc/wordle/v2/%Y-%m-%d.json")).json()
		if "status" in json:
			break

		print("%(days_since_launch)d (%(print_date)s) - %(editor)s" % json)
		words.append(json["solution"])

	return words


def update_words(output):
	"""Updates the word lists"""

	# First get the HTML and figure out what the current JS is
	html = get("https://www.nytimes.com/games/wordle/index.html").text
	soup = BeautifulSoup(html, "html.parser")
	script, = [script["src"] for script in soup.findAll("script") if script.has_attr("src") and script["src"].startswith("https://www.nytimes.com/games-assets/v2/wordle.")]

	# So we can get that and grab the base list incase they ever update that again
	js = get(script).text
	guesses = json.loads(re.findall(r'\["aahed"(?:,"\w{5}")*(?=,"cigar")', js)[0] + "]")
	guesses.sort()
	choices = json.loads("[" + re.findall(r'"cigar"(?:,"\w{5}")*\]', js)[0])

	# Now scrape the real upcoming answers since that's in a special API now
	new_answers = scrape_answers(506)  # 506 is the day they started this

	choices = [word for word in choices if word not in new_answers]
	choices = choices[:506] + new_answers + choices[506:]

	output.write('''#include "words.hpp"

// This list is in order, don't look if you don't want spoilers, if editing you should shuffle it somehow
std::vector<std::u16string> Words::choices = {u"''')

	output.write('", u"'.join([word.upper() for word in choices]))

	output.write('''"};

std::vector<std::u16string> Words::guesses = {u"''')

	output.write('", u"'.join([word.upper() for word in guesses]))

	output.write('"};\n')


if __name__ == "__main__":
	parser = ArgumentParser(description="Updates the Wordle DS word list")
	parser.add_argument("output", metavar="output.cpp", type=FileType("w"), help="output C++ source")

	args = parser.parse_args()

	update_words(args.output)
