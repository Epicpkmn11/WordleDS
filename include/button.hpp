#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "json.hpp"

#include <nds/ndstypes.h>
#include <nds/touch.h>

struct Button {
	int x;
	int y;
	int w;
	int h;

	Button(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

	Button(const Json &json) {
		x = json[0].get()->valueint;
		y = json[1].get()->valueint;
		w = json[2].get()->valueint;
		h = json[3].get()->valueint;
	}

	bool touching(touchPosition touch) const {
		return touch.px >= x && touch.px <= x + w && touch.py >= y && touch.py <= y + h;
	}
};

#endif // BUTTON_HPP
