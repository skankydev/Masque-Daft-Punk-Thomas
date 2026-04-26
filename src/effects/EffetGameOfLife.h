#pragma once
#include "Effect.h"

class EffetGameOfLife : public Effect {

		bool _grid[MATRIX_H][MATRIX_W];
		bool _next[MATRIX_H][MATRIX_W];

		CRGB    _colorLight;  // cellules vivantes
		uint8_t _stableCount; // générations sans changement
		uint8_t _stableMax;   // seuil avant régénération

		void _randomize() {
			for (uint8_t y = 0; y < MATRIX_H; y++) {
				for (uint8_t x = 0; x < MATRIX_W; x++) {
					_grid[y][x] = random(3) == 0; // ~33% de cellules vivantes
				}
			}
			_stableCount = 0;
		}

		// Mode torique : les bords se connectent entre eux
		uint8_t _countNeighbors(uint8_t x, uint8_t y) {
			uint8_t count = 0;
			for (int8_t dy = -1; dy <= 1; dy++) {
				for (int8_t dx = -1; dx <= 1; dx++) {
					if (dx == 0 && dy == 0) continue;
					uint8_t nx = (x + dx + MATRIX_W) % MATRIX_W;
					uint8_t ny = (y + dy + MATRIX_H) % MATRIX_H;
					if (_grid[ny][nx]) count++;
				}
			}
			return count;
		}

	public:
		EffetGameOfLife()
			: _colorLight(CRGB(0, 255, 0)),
			  _stableCount(0),
			  _stableMax(8)
		{
			_randomize();
		}

		void reset() override {
			_randomize();
		}

		void setColor(CRGB color) override {
			_colorLight = color;
		}

		void step(CRGB* leds) override {
			bool changed = false;

			for (uint8_t y = 0; y < MATRIX_H; y++) {
				for (uint8_t x = 0; x < MATRIX_W; x++) {
					uint8_t n     = _countNeighbors(x, y);
					bool    alive = _grid[y][x];
					_next[y][x]   = alive ? (n == 2 || n == 3) : (n == 3);
					if (_next[y][x] != alive) changed = true;
				}
			}

			// Stagnation → régénération
			if (!changed) {
				if (++_stableCount >= _stableMax) {
					_randomize();
					return;
				}
			} else {
				_stableCount = 0;
			}

			memcpy(_grid, _next, sizeof(_grid));

			// Rendu
			for (uint8_t y = 0; y < MATRIX_H; y++) {
				for (uint8_t x = 0; x < MATRIX_W; x++) {
					leds[XY(x, y)] = _grid[y][x] ? _colorLight : CRGB::Black;
				}
			}
		}

		String name() override { return "Game of Life"; }
};
