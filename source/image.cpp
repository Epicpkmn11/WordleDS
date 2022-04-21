#include "image.hpp"
#include "tonccpy.h"

#include <nds/arm9/sassert.h>
#include <nds/arm9/decompress.h>

#define CHUNK_ID(a, b, c, d) ((u32)((a) | (b) << 8 | (c) << 16 | (d) << 24))

bool Image::grfDecompress(const void *src, void *dst, bool vram) {
	if(src == nullptr || dst == nullptr)
		return false;

	u32 header = *(u32*)src;
	uint size = header >> 8;
	
	switch(header & 0xF0) {
		case 0x00: // No compression
			tonccpy(dst, (u8*)src + 4, size);
			return true;

		case 0x10: // LZ77
			decompress(src, dst, vram ? LZ77Vram : LZ77);
			return true;

		case 0x20: // Huffman
			decompress(src, dst, HUFF);
			return true;

		case 0x30: // RLE
			decompress(src, dst, vram ? RLEVram : RLE);
			return true;

		default:
			return false;
	}
}

Image::Image(const char *path, u32 width, u32 height, const u8 *fallback) : _width(width), _height(height) {
	FILE *file = fopen(path, "rb");
	if(file) {
		fread(&_header, 1, sizeof(GrfHeader), file);
		_buffer = std::shared_ptr<u8[]>(new u8[_header.fileSize - sizeof(GrfHeader) + 8]);
		fread(_buffer.get(), 1, _header.fileSize - sizeof(GrfHeader) + 8, file);
		fclose(file);
	} else {
		tonccpy(&_header, fallback, sizeof(GrfHeader));
	}

	sassert(_header.texWidth == _width, "Invalid image width\n(%ld, should be %ld)\n\n%s", _header.texWidth, _width, path);
	sassert(_header.texHeight == _height, "Invalid image height\n(%ld, should be %ld)\n\n%s", _header.texHeight, _height, path);

	const u8 *src = _buffer != nullptr ? _buffer.get() : (fallback + sizeof(GrfHeader));
	for(u32 i = 0, size = 0; i < _header.fileSize - 4; i += size + 8) {
		switch(*(u32 *)src) {
			case CHUNK_ID('G','F','X',' '):
				_tiles = src + 8;
				break;

			case CHUNK_ID('M', 'A','P',' '):
				_map = (u16 *)(src + 8);
				break;
			
			case CHUNK_ID('P','A','L',' '):
				_pal = (u16 *)(src + 8);
				break;
			default:
				break;
		}

		src += ((u32 *)src)[1] + 8;
	}
}
