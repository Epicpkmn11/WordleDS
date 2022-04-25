#!/usr/bin/env python3

import git
import json
import urllib.parse
import yaml

from datetime import datetime
from glob import glob
from os import makedirs, mkdir, path, system
from PIL import Image
from py7zr import SevenZipFile
from textwrap import dedent


def getTheme(path: str) -> int:
	"""Gets the theme of a mod based on its path"""

	if "3dsmenu/" in path:
		return "Nintendo 3DS"
	elif "akmenu/" in path:
		return "Wood UI"
	elif "dsimenu/" in path:
		return "Nintendo DSi"
	elif "r4menu/" in path:
		return "R4 Original"
	elif "extras/fonts/" in path:
		return "Font"
	elif "icons/" in path:
		return "Icon"
	elif "unlaunch/" in path:
		return "Unlaunch"
	return ""



def lastUpdated(sevenZip):
	"""Gets the latest date from the items in a zip"""

	latest = None
	for item in sevenZip.list():
		if latest is None or item.creationtime > latest:
			latest = item.creationtime

	return latest


def downloadScript(mod: str, inFolder: bool) -> list:
	"""Makes a script to download the specified mod"""

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
			"input": (modName + "/") if inFolder else "",
			"output": f"/_nds/WordleDS/{modName}/"
		},
		{
			"type": "deleteFile",
			"file": f"/{modName}.zip"
		}
	]


# Read version from old unistore
unistoreOld = {}
if path.exists(path.join("unistore", "twlmenu-mods.unistore")):
	with open(path.join("unistore", "twlmenu-mods.unistore"), "r", encoding="utf8") as file:
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

# Get mod files
files = glob("mods/*.zip")

# Generate UniStore entries
for mod in files:
	print(mod)

	info = {}
	updated = datetime.utcfromtimestamp(0)
	inFolder = False
	modName = mod[mod.rfind("/") + 1:mod.rfind(".")]

	if mod[-2:] == "7z":
		with SevenZipFile(mod) as a:
			updated = lastUpdated(a)
			inFolder = modName in a.getnames()
	else:
		updated = datetime.utcfromtimestamp(int(git.Repo(".").git.log(["-n1", "--pretty=format:%ct", "--", mod]) or 0))

	created = datetime.utcfromtimestamp(int(git.Repo(".").git.log(["--pretty=format:%ct", "--", mod]).split("\n")[-1] or 0))

	with open("meta.json") as file:
		j = json.load(file)
		if modName in j:
			info = j[modName]

	modInfo = {
		"title": info["title"] if "title" in info else modName,
		"version": info["version"] if "version" in info else "v1.0.0",
		"author": info["author"] if "author" in info else "",
		"category": info["categories"] if "categories" in info else [],
		"icon_index": -1,
		"description": info["description"] if "description" in info else "",
		"license": "Creative Commons Zero v1.0 Universal",
		"last_updated": updated.strftime("%Y-%m-%d at %H:%M (UTC)")
	}

	color = None

	# Make icon for UniStore
	if not path.exists(path.join("unistore", "temp")):
		mkdir(path.join("unistore", "temp"))
	iconPath = None
	if path.exists(path.join("icons", modName + ".png")):
		iconPath = path.join("icons", modName + ".png")

	if iconPath:
		with Image.open(iconPath) as icon:
			if mod[-3:] not in ("png", "bin"):
				icon.thumbnail((48, 48))

			icon.save(path.join("unistore", "temp", str(iconIndex) + ".png"))
			icons.append(str(iconIndex) + ".png")
			modInfo["icon_index"] = iconIndex
			iconIndex += 1

			color = icon.copy().convert("RGB")
			color.thumbnail((1, 1))
			color = color.getpixel((0, 0))
			color = f"#{color[0]:02x}{color[1]:02x}{color[2]:02x}"

	# Add entry to UniStore
	unistore["storeContent"].append({
		"info": modInfo,
		info["title"] if "title" in info else modName: downloadScript(mod, inFolder)
	})

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
	file.write(json.dumps(unistore, sort_keys=True))

# Write mods.md
with open("mods.md", "w") as file:
	file.write("<!-- Do not edit this file manually, it is generated automatically. -->\n")

	for item in unistore["storeContent"]:
		info = item["info"]

		icon = ""
		if info["icon_index"] != -1:
			icon = f"![Icon](https://raw.githubusercontent.com/Epicpkmn11/WordleDS/mods/icons/{urllib.parse.quote(info['title'])}.png)"

		file.write(dedent(f"""
			## {icon} {info["title"]} by {info["author"]}
			{info["description"]}

			Categories: {", ".join(info["category"])}

			Download:
			"""
		))

		for script in item:
			if script == "info":
				continue

			file.write(f"- [{script}]({item[script][0]['file']})\n")
