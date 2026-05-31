#pragma once
#include "Effect.h"

class EffetFire : public Effect {

		uint8_t _heat[MATRIX_W][MATRIX_H] = {};
		CRGB    _color;

		// Mappe la chaleur (0-255) sur un dégradé noir → couleur → blanc
		CRGB _heatToColor(uint8_t heat) {
			if (heat < 128) {
				// noir → couleur
				CRGB c = _color;
				c.nscale8(heat * 2);
				return c;
			} else {
				// couleur → blanc
				return blend(_color, CRGB::White, (heat - 128) * 2);
			}
		}

	public:
		EffetFire() : _color(CRGB(255, 80, 0)) {} // orange feu par défaut

		void setColor(CRGB color) override { _color = color; }

		void reset() override {
			memset(_heat, 0, sizeof(_heat));
		}

		void step(CRGB* leds) override {
			// Refroidissement
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				for (uint8_t y = 0; y < MATRIX_H; y++) {
					//plus les valeurs sont hautes, plus ça refroidit vite
					_heat[x][y] = qsub8(_heat[x][y], random8(35, 50));
				}
			}

			// Propagation vers le haut
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				for (uint8_t y = MATRIX_H - 1; y > 0; y--) {
					_heat[x][y] = (_heat[x][y - 1] + _heat[x][max(0, y - 2)] * 2) / 3;
				}
			}

			// Allumage
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				if (random8() < 100) {
					//plus les valeurs sont hautes, plus la source est chaude
					_heat[x][0] = qadd8(_heat[x][0], random8(50, 130));
				}
			}

			// Rendu
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				for (uint8_t y = 0; y < MATRIX_H; y++) {
					leds[XY(x, MATRIX_H - 1 - y)] = _heatToColor(_heat[x][y]);
				}
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			// Feu 1D — plus chaud en bas (index len-1), plus froid en haut (index 0)
			for (uint8_t i = 0; i < len; i++) {
				uint8_t base = 220 - i * (220 / len);
				uint8_t heat = qsub8(base, random8(0, 40));
				strip[i] = _heatToColor(heat);
			}
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			// Blocs fixes : rouge → orange → jaune
			uint8_t third = len / 3;
			for (uint8_t i = 0; i < len; i++) {
				if      (i < third)         strip[i] = CRGB(255,  20,   0);  // rouge
				else if (i < third * 2)     strip[i] = CRGB(255, 120,   0);  // orange
				else                        strip[i] = CRGB(255, 220,   0);  // jaune
			}
		}

		String name() override { return "Fire"; }
};
