#pragma once
#include "Effect.h"

class EffetMatrix : public Effect {

		uint8_t _bright[MATRIX_W][MATRIX_H]; // luminosité courante de chaque pixel
		int8_t  _head[MATRIX_W];             // position Y de la tête (-1 = en attente)
		uint8_t _delay[MATRIX_W];            // délai avant prochain départ

		CRGB _color; // couleur de base de la traînée

	public:
		EffetMatrix() : _color(CRGB(0, 200, 0)) {
			reset();
		}

		void reset() override {
			memset(_bright, 0, sizeof(_bright));
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				_head[x]  = -1;
				_delay[x] = random(0, MATRIX_H * 3); // décalage initial aléatoire
			}
		}

		void setColor(CRGB color) override {
			_color = color;
		}

		void step(CRGB* leds) override {
			// Estomper toutes les traînées
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				for (uint8_t y = 0; y < MATRIX_H; y++) {
					if (_bright[x][y] > 40) {
						_bright[x][y] = _bright[x][y] * 6 / 8; // fade rapide
					} else {
						_bright[x][y] = 0;
					}
				}
			}

			// Avancer les gouttes
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				if (_head[x] == -1) {
					// En attente — décompter le délai
					if (_delay[x] == 0) {
						_head[x] = 0; // démarre en haut
					} else {
						_delay[x]--;
					}
				} else {
					// Tête active — allumer le pixel
					if (_head[x] < MATRIX_H) {
						_bright[x][(uint8_t)_head[x]] = 255;
						_head[x]++;
					} else {
						// Arrivée en bas — reset avec délai aléatoire
						_head[x]  = -1;
						_delay[x] = random(2, MATRIX_H * 4);
					}
				}
			}

			// Rendu
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				for (uint8_t y = 0; y < MATRIX_H; y++) {
					uint8_t b = _bright[x][y];
					if (b == 0) {
						leds[XY(x, y)] = CRGB::Black;
					} else if (b > 200) {
						// Tête : blanc éclatant
						leds[XY(x, y)] = CRGB(b, 255, b);
					} else {
						// Traînée : couleur de base scalée
						leds[XY(x, y)] = CRGB(
							scale8(_color.r, b),
							scale8(_color.g, b),
							scale8(_color.b, b)
						);
					}
				}
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			fill_solid(strip, len, CRGB::Black);
			int8_t pos = (millis() / 60) % (len + 4);
			for (int8_t i = 0; i < (int8_t)len; i++) {
				if (i == pos) {
					strip[i] = CRGB(100, 255, 100);          // tête : blanc-vert
				} else if (i < pos && i >= pos - 4) {
					uint8_t fade = 255 - (pos - i) * 55;
					strip[i] = CRGB(0, scale8(_color.g, fade), 0);  // traînée verte
				}
			}
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			CRGB c = _color;
			c.nscale8(beatsin8(10, 20, 70));
			fill_solid(strip, len, c);
		}

		String name() override { return "Matrix"; }
};
