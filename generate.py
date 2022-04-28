#!/usr/bin/env python3

import git
import json
import re
import urllib.parse

from datetime import datetime
from os import mkdir, path, system
from PIL import Image
from textwrap import dedent
from zipfile import ZipFile


def lastUpdated(sevenZip):
	"""Gets the latest date from the items in a zip"""

	latest = None
	for item in sevenZip.list():
		if latest is None or item.creationtime > latest:
			latest = item.creationtime

	return latest


def downloadScript(mod: str) -> list:
	"""Makes a script to download the specified mod"""

	with ZipFile(mod) as z:
		folder = next(re.findall(r"(.*?\/?)\w+\.(?:json|grf|msl)$", x.filename) for x in z.filelist if len(re.findall(r"(.*?)\/?\w+\.(?:json|grf|msl)$", x.filename)) > 0)[0]

	modName = mod[mod.rfind("/") + 1:mod.rfind(".")]

	return [
		{
			"type": "downloadFile",
			"file": "https://raw.githubusercontent.com/Epicpkmn11/WordleDS/mods/" + urllib.parse.quote(mod),
			"output": f"/{modName}.zip"
		},
		{
			"type": "extractFile",
			"file": f"/{modName}.zip",
			"input": re.escape(folder),
			"output": f"/_nds/WordleDS/{modName}/"
		},
		{
			"type": "deleteFile",
			"file": f"/{modName}.zip"
		}
	]


# Read version from old unistore
unistoreOld = {}
if path.exists(path.join("unistore", "wordle-ds.unistore")):
	with open(path.join("unistore", "wordle-ds.unistore"), "r", encoding="utf8") as file:
		unistoreOld = json.load(file)

# Output JSON
output = []

# Create UniStore base
unistore = {
	"storeInfo": {
		"title": "Wordle DS Mods",
		"author": "Pk11",
		"url": "https://raw.githubusercontent.com/Epicpkmn11/WordleDS/mods/unistore/wordle-ds.unistore",
		"file": "wordle-ds.unistore",
		"sheetURL": "https://raw.githubusercontent.com/Epicpkmn11/WordleDS/mods/unistore/wordle-ds.t3x",
		"sheet": "wordle-ds.t3x",
		"description": "Mods for Wordle DS",
		"version": 3,
		"revision": 0 if ("storeInfo" not in unistoreOld or "revision" not in unistoreOld["storeInfo"]) else unistoreOld["storeInfo"]["revision"]
	},
	"storeContent": [],
}

# Icons array
icons = []
iconIndex = 0
if not path.exists(path.join("unistore", "temp")):
	mkdir(path.join("unistore", "temp"))

# mods.md mod list
modList = open("mods.md", "w")
modList.write("<!-- Do not edit this file manually, it is generated automatically. -->\n")

# Read metadata JSON
with open("meta.json") as file:
	meta = json.load(file)
	meta.sort(key=lambda x: x["title"])

# Generate UniStore entries
for info in meta:
	print(info["title"])

	updated = max(datetime.utcfromtimestamp(int(git.Repo(".").git.log(["-n1", "--pretty=format:%ct", "--", path.join("mods", x)]) or 0)) for x in info["downloads"])

	# Process text for unistore
	author = info["author"] if "author" in info else ""
	author = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", author)  # Remove links

	desc = info["description"] if "description" in info else ""
	desc = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", desc)  # Remove links

	modInfo = {
		"title": info["title"],
		"version": info["version"] if "version" in info else "v1.0.0",
		"author": author,
		"category": info["categories"] if "categories" in info else [],
		"icon_index": -1,
		"description": desc,
		"license": "Creative Commons Zero v1.0 Universal",
		"last_updated": updated.strftime("%Y-%m-%d at %H:%M (UTC)")
	}

	# Make icon for UniStore
	iconPath = None
	if path.exists(path.join("icons", info["title"] + ".png")):
		iconPath = path.join("icons", info["title"] + ".png")

	if iconPath:
		with Image.open(iconPath) as icon:
			icon.thumbnail((48, 48))

			icon.save(path.join("unistore", "temp", str(iconIndex) + ".png"))
			icons.append(str(iconIndex) + ".png")
			modInfo["icon_index"] = iconIndex
			iconIndex += 1

	# Add entry to UniStore
	unistore["storeContent"].append({"info": modInfo})
	for download in info["downloads"]:
		unistore["storeContent"][-1][download] = downloadScript(path.join("mods", download))

	# Add to mods.md
	icon = ""
	if iconPath:
		icon = f"![Icon](https://raw.githubusercontent.com/Epicpkmn11/WordleDS/mods/{urllib.parse.quote(iconPath)})"

	modList.write(dedent(f"""
		## {icon} {info["title"]} by {info["author"] if "author" in info else ""}
		{info["description"]}

		Categories: {", ".join(info["categories"]) if "categories" in info else ""}

		Download:
	"""))

	for download in info["downloads"]:
		modList.write(f"- [{download}](https://raw.githubusercontent.com/Epicpkmn11/WordleDS/mods/mods/{urllib.parse.quote(download)})\n")

# Make t3x
with open(path.join("unistore", "temp", "icons.t3s"), "w", encoding="utf8") as file:
	file.write("--atlas -f rgba -z auto\n\n")
	for icon in icons:
		file.write(icon + "\n")
system("tex3ds -i " + path.join("unistore", "temp", "icons.t3s") + " -o " + path.join("unistore", "wordle-ds.t3x"))

# Increment revision if not the same
if unistore != unistoreOld:
	unistore["storeInfo"]["revision"] += 1

# Write unistore to file
with open(path.join("unistore", "wordle-ds.unistore"), "w", encoding="utf8") as file:
	json.dump(unistore, file, sort_keys=True)
