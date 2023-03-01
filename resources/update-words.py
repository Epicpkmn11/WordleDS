#!/usr/bin/env python3

from argparse import ArgumentParser, FileType
from datetime import datetime, timezone
from requests import get
from bs4 import BeautifulSoup

import json
import re


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

	# Then grab the known word order
	words = get("https://wordle.xn--rck9c.xn--tckwe/words.php?date=2021-06-19&limit=10000").json()

	# Now let's make the file, first inclues
	output.write('#include "words.hpp"\n\n')

	# Then the known word order
	output.write("std::vector<int> Words::order = {")
	output.write(", ".join([word["id"] for word in words]))
	output.write("};\n\n")

	# The choice words
	output.write('std::vector<std::u16string> Words::choices = {u"')
	output.write('", u"'.join([word.upper() for word in choices]))
	output.write('"};\n\n')

	# And the guess words
	output.write('std::vector<std::u16string> Words::guesses = {u"')
	output.write('", u"'.join([word.upper() for word in guesses]))
	output.write('"};\n')


if __name__ == "__main__":
	parser = ArgumentParser(description="Updates the Wordle DS word list")
	parser.add_argument("output", metavar="output.cpp", type=FileType("w"), help="output C++ source")

	args = parser.parse_args()

	update_words(args.output)
