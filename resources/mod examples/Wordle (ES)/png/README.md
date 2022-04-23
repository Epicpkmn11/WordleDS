These are the PNG files of the graphics edited in this mod. They aren't used by Wordle DS at all, but are here as an example of how to convert with grit.

The `.grit` files in this folder are exactly the same as those in the [gfx folder](https://github.com/Epicpkmn11/WordleDS/tree/main/gfx) and running `./grit.sh` (macOS/Linux) or `grit.bat` (Windows) or the following grit command in this folder will convert all the graphics into `.grf` files that Wordle DS can use.

```bash
grit $IMAGE -ftr -fh! -o../$IMAGE
```
(replace `$IMAGE` with the name of the file you wish to convert)