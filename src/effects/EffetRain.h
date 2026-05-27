#pragma once
#include "Effect.h"

class EffetRain : public Effect {

		uint8_t _pos[MATRIX_W]   = {};
		uint8_t _speed[MATRIX_W] = {};
		CRGB    _color;

	public:
		EffetRain() : _color(CRGB(30, 100, 255)) {} // bleu pluie par défaut

		void setColor(CRGB color) override { _color = color; }

		void reset() override {
			memset(_pos,   0, sizeof(_pos));
			memset(_speed, 0, sizeof(_speed));
		}

		void step(CRGB* leds) override {
			// Fondu progressif
			for (uint16_t i = 0; i < NUM_LEDS; i++) {
				leds[i].nscale8(180);
			}

			for (uint8_t x = 0; x < MATRIX_W; x++) {
				if (_speed[x] == 0) {
					if (random8() < 40) {
						_pos[x]   = 0;
						_speed[x] = random8(1, 4);
					}
				} else {
					_pos[x] += _speed[x];
					if (_pos[x] >= MATRIX_H) {
						_speed[x] = 0;
					}
				}

				if (_speed[x] > 0) {
					leds[XY(x, _pos[x])] = _color;
				}
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			// Goutte qui descend en boucle
			fill_solid(strip, len, CRGB::Black);
			uint8_t pos = (millis() / 80) % (len + 2);
			if (pos < len)     { strip[pos] = _color; }
			if (pos > 0 && pos - 1 < len) { strip[pos - 1] = _color; strip[pos - 1].nscale8(100); }
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			// Lueur ambiante pulsée
			CRGB c = _color;
			c.nscale8(beatsin8(15, 30, 90));
			fill_solid(strip, len, c);
		}

		String name() override { return "Rain"; }
};
