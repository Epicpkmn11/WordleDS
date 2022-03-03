# Wordle DS
A clone of [Wordle](https://www.nytimes.com/games/wordle/index.html) for the Nintendo DS(i). Unless the NY Times changes the word list, this will have the same word each day as the official Wordle. A sharable text file will be made as `WordleDS.txt` on completion.

## Building
Follow devkitPro's [Getting Started](https://devkitpro.org/wiki/Getting_Started) guide to install libnds, devkitARM, and grit, then simply run `make`.

### Customizing
The word lists are defined in `words.cpp`. Any ASCII wordlist should work fine, however adapting to other languages would require a bit of tweaking to the graphics and letter handling.

Many simple customizations such as the word length, starting day, and any text strings are defined in `defines.hpp`.

All graphics are in `gfx` and should mostly work fine if the PNGs are edited. A couple have hardcoded palette or map tweaks at runtime so they may glitch out of edited. The more complex images also have their [GIMP](https://www.gimp.org) files in `resources`, the font used is nintendo_NTLG-DB_001. The fonts are in `data` and can be edited using an [NFTR editor](https://pk11.us/nftr-editor/).

## Credits
- [Pk11](https://github.com/Epicmn11): This DS port
- [Josh Wardle](https://github.com/powerlanguage): The original Wordle
