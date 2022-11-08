#!/usr/bin/env python3

from argparse import ArgumentParser, FileType
from requests import get
from bs4 import BeautifulSoup

import json
import re

def update_words(output):
	html = get("https://www.nytimes.com/games/wordle/index.html").text
	soup = BeautifulSoup(html, "html.parser")
	script, = [script["src"] for script in soup.findAll("script") if script.has_attr("src") and script["src"].startswith("https://www.nytimes.com/games-assets/v2/wordle.")]

	js = get(script).text
	guesses = json.loads(re.findall(r'\["aahed"(?:,"\w{5}")*(?=,"cigar")', js)[0] + "]")
	choices = json.loads("[" + re.findall(r'"cigar"(?:,"\w{5}")*\]', js)[0])

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