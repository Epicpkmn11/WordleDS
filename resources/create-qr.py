#!/usr/bin/env python3

import qrcode

from argparse import ArgumentParser, FileType
from PIL import Image, ImageDraw

parser = ArgumentParser(description="Makes a QR")
# parser.add_argument("url", type=str)
parser.add_argument("-i", "--icon", type=FileType("rb"))
parser.add_argument("-v", "--version", type=str)

args = parser.parse_args()

url = f"https://github.com/Epicpkmn11/WordleDS/releases/download/{args.version}/WordleDS"

for ext in (".cia", ".dsi"):
	qr = qrcode.make(url + ext, box_size=5, version=5).convert("RGBA")

	with Image.open(args.icon) as img:
		draw = ImageDraw.Draw(qr)
		draw.rectangle((((qr.width - img.width) // 2 - 5, (qr.height - img.height) // 2 - 10), ((qr.width + img.width) // 2 + 4, (qr.height + img.height) // 2 + 10)), fill=(255, 255, 255))
		qr.paste(img, ((qr.width - img.width) // 2, (qr.height - img.height) // 2), mask=img if img.mode == "RGBA" else None)
		if ext == ".cia":
			draw.text(((qr.width - img.width) // 2, (qr.height - img.height) // 2 - 10), "3", (255, 0, 0))
			draw.text(((qr.width - img.width) // 2 + 6, (qr.height - img.height) // 2 - 10), "DS", (0, 0, 0))
		else:
			draw.text(((qr.width - img.width) // 2, (qr.height - img.height) // 2 - 10), "DSi", (0, 0, 0))

		if img.width == 32 and len(args.version) > 5:
			draw.text(((qr.width - img.width) // 2 - 2, (qr.height - img.height) // 2 + img.height), args.version[:(img.width + 4) // 6], (0, 0, 0))
		else:
			draw.text(((qr.width - img.width) // 2, (qr.height - img.height) // 2 + img.height), args.version[:img.width // 6], (0, 0, 0))

	qr.save(f"{args.version}{ext}.png")
