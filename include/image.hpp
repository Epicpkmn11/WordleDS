#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <memory>
#include <nds/ndstypes.h>
#include <string>

class Image {
	std::shared_ptr<u8[]> _buffer = nullptr;

	const u8 *_tiles = nullptr;
	const u16 *_map = nullptr;
	const u16 *_pal = nullptr;

	struct GrfHeader {
		u32 magicRiff, fileSize, magicGrf, magicHdr, hdrSize;
		u8  gfxAttr, mapAttr, mmapAttr, palAttr;
		u8  tileWidth, tileHeight, metaWidth, metaHeight;
		u32 texWidth, texHeight;
	} __packed _header;
	static_assert(sizeof(GrfHeader) == 0x24);

	static bool grfDecompress(const void *src, void *dst, bool vram);

public:
	// width and height are used for sanity checking the GRF
	Image(const char *path, u32 width, u32 height, const u8 *fallback, bool enforceSize = true);
	Image() {}

	u32 width(void) const { return _header.texWidth; }
	u32 height(void) const { return _header.texHeight; }

	size_t tilesLen(void) const { return _header.texWidth * _header.texHeight * _header.gfxAttr / 8; }

	const Image &decompressTiles(void *dst, bool vram = true) const { grfDecompress(_tiles, dst, vram); return *this; }
	const Image &decompressMap(void *dst, bool vram = true) const { grfDecompress(_map, dst, vram); return *this; }
	const Image &decompressPal(void *dst, bool vram = true) const { grfDecompress(_pal, dst, vram); return *this; }
	const inline Image &decompressAll(void *tilesDst, void *mapDst, void *palDst, bool vram = true) const {
		decompressTiles(tilesDst, vram);
		decompressMap(mapDst, vram);
		decompressPal(palDst, vram);
		return *this;
	};
};

#endif // IMAGE_HPP
