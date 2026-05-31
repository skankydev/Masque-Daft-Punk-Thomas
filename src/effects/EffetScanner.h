#pragma once
#include "Effect.h"

class EffetScanner : public Effect {
		int8_t  _pos = 0;
		int8_t  _dir = 1;
		uint8_t _hue = 0;

	public:
		void reset() override {
			_pos = 0;
			_dir = 1;
			_hue = 0;
		}

		void step(CRGB* leds) override {
			fill_solid(leds, NUM_LEDS, CRGB::Black);

			for (uint8_t y = 0; y < MATRIX_H; y++) {
				leds[XY(_pos, y)] = CHSV(_hue, 255, 255);
				if (_pos > 0)            leds[XY(_pos - 1, y)] = CHSV(_hue, 255, 80);
				if (_pos < MATRIX_W - 1) leds[XY(_pos + 1, y)] = CHSV(_hue, 255, 80);
			}

			_pos += _dir;
			if (_pos >= MATRIX_W - 1 || _pos <= 0) {
				_dir = -_dir;
				_hue += 30;
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			// Point scanner synchronisé — position mappée depuis la matrice
			fill_solid(strip, len, CRGB::Black);
			uint8_t pos = _pos * (len - 1) / (MATRIX_W - 1);
			strip[pos] = CHSV(_hue, 255, 255);
			if (pos > 0)       strip[pos - 1] = CHSV(_hue, 255, 80);
			if (pos < len - 1) strip[pos + 1] = CHSV(_hue, 255, 80);
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			// Lueur ambiante dans la couleur courante
			fill_solid(strip, len, CHSV(_hue, 255, 60));
		}

		String name() override { return "Scanner"; }
};
