#!/usr/bin/env python3

from argparse import ArgumentParser, FileType
from datetime import datetime, timezone
from requests import get
from os import path

import json
import re


def update_words(mod_json):
	"""Updates the word lists"""

	j = {}

	# Open the mod.json if it exists
	if mod_json and path.exists(mod_json):
		with open(mod_json, "r") as f:
			j = json.load(f)

	# Ensure the structure exists
	if "words" not in j:
			j["words"] = {}

	# Get all of the known word order
	j["words"]["order"] = get("https://wordle.xn--rck9c.xn--tckwe/words.php?date=2021-06-19&limit=10000&include=id").json()

	with open(mod_json, "w") as f:
		json.dump(j, f)


if __name__ == "__main__":
	parser = ArgumentParser(description="Updates the Wordle DS word list")
	parser.add_argument("--file", "-f", default="mod.json", help="mod.json file to update")

	args = parser.parse_args()

	update_words(args.file)
