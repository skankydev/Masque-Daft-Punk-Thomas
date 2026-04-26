#pragma once
#include "Effect.h"

class EffetRainbow : public Effect {
	public:
		void step(CRGB* leds) override {
			uint8_t t = millis() >> 4;
			for (uint8_t y = 0; y < MATRIX_H; y++) {
				for (uint8_t x = 0; x < MATRIX_W; x++) {
					leds[XY(x, y)] = CHSV(t + x * 8 + y * 12, 255, 255);
				}
			}
		}

		String name() override { return "Rainbow"; }
};
