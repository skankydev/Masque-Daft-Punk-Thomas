#pragma once
#include "Effect.h"

class EffetBounce : public Effect {

		// Sprite cible 5x5 — anneau extérieur + point central
		static const bool _target[5][5];
		static const uint8_t HALF = 2; // demi-taille du sprite (5/2 arrondi)

		float _x, _y;   // position du centre
		float _vx, _vy; // vitesse

		CRGB _color;

	public:
		EffetBounce() : _color(CRGB(255, 0, 0)) {
			reset();
		}

		void reset() override {
			// Démarre au centre avec une vitesse aléatoire
			_x  = MATRIX_W / 2.0f;
			_y  = MATRIX_H / 2.0f;
			_vx = random(5, 15) / 10.0f * (random(2) ? 1 : -1);
			_vy = random(5, 15) / 10.0f * (random(2) ? 1 : -1);
		}

		void setColor(CRGB color) override {
			_color = color;
		}

		void step(CRGB* leds) override {
			fill_solid(leds, NUM_LEDS, CRGB::Black);

			// Déplacement
			_x += _vx;
			_y += _vy;

			// Rebond sur les bords (centre ne dépasse pas HALF du bord)
			if (_x < HALF) {
				_x  = HALF;
				_vx = abs(_vx);
			} else if (_x > MATRIX_W - 1 - HALF) {
				_x  = MATRIX_W - 1 - HALF;
				_vx = -abs(_vx);
			}
			if (_y < HALF) {
				_y  = HALF;
				_vy = abs(_vy);
			} else if (_y > MATRIX_H - 1 - HALF) {
				_y  = MATRIX_H - 1 - HALF;
				_vy = -abs(_vy);
			}

			// Dessin du sprite centré sur (_x, _y)
			int8_t cx = (int8_t)_x;
			int8_t cy = (int8_t)_y;

			for (int8_t sy = 0; sy < 5; sy++) {
				for (int8_t sx = 0; sx < 5; sx++) {
					if (!_target[sy][sx]) continue;
					int8_t px = cx - HALF + sx;
					int8_t py = cy - HALF + sy;
					if (px < 0 || px >= MATRIX_W) continue;
					if (py < 0 || py >= MATRIX_H) continue;
					leds[XY(px, py)] = _color;
				}
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			// Point qui suit la position X du sprite
			fill_solid(strip, len, CRGB::Black);
			uint8_t pos = constrain(
				(uint8_t)((_x - HALF) * (len - 1) / (MATRIX_W - 1 - HALF * 2)),
				0, len - 1
			);
			if (pos > 0)       { strip[pos - 1] = _color; strip[pos - 1].nscale8(80); }
			strip[pos] = _color;
			if (pos < len - 1) { strip[pos + 1] = _color; strip[pos + 1].nscale8(80); }
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			CRGB c = _color;
			c.nscale8(40);
			fill_solid(strip, len, c);
		}

		String name() override { return "Bounce"; }
};

// Cible 5x5 — anneau extérieur + point central
const bool EffetBounce::_target[5][5] = {
	{ 0, 1, 1, 1, 0 },
	{ 1, 1, 0, 1, 1 },
	{ 1, 0, 0, 0, 1 },
	{ 1, 1, 0, 1, 1 },
	{ 0, 1, 1, 1, 0 },
};
