#pragma once
#include "Effect.h"

class EffetHeartBeat : public Effect {

		// ECG pré-calculé : pour chaque colonne x, la ligne y du pixel
		int8_t _ecg[MATRIX_W];

		uint8_t _phase;     // 0 = dessin, 1 = hold, 2 = effacement colonne par colonne
		int16_t _x;
		uint8_t _holdCount;

		void _buildEcg() {
			// Breakpoints scalés depuis les valeurs originales (matrice 40 de large)
			const int b1 = MATRIX_W * 5  / 40;
			const int b2 = MATRIX_W * 8  / 40;
			const int b3 = MATRIX_W * 15 / 40;
			const int b4 = MATRIX_W * 18 / 40;
			const int b5 = MATRIX_W * 21 / 40;
			const int b6 = MATRIX_W * 25 / 40;
			const int b7 = MATRIX_W * 32 / 40;
			const int b8 = MATRIX_W * 36 / 40;

			int y = MATRIX_H / 2; // startRow

			for (int x = 0; x < MATRIX_W; x++) {
				if (x < b1) {
					// rien
				} else if (x < b2) {
					y++;
				} else if (x < b3) {
					y--;
				} else if (x < b4) {
					y++;
				} else if (x < b5) {
					y = MATRIX_H / 2 - 1; // endRow
				} else if (x < b6) {
					y++;
				} else if (x < b7) {
					y--;
				} else if (x < b8) {
					y++;
				} else {
					y = MATRIX_H / 2; // retour startRow
				}
				_ecg[x] = constrain(y, 0, MATRIX_H - 1);
			}
		}

		CRGB _color;

	public:
		EffetHeartBeat() : _phase(0), _x(0), _holdCount(0), _color(CRGB::Red) {
			_buildEcg();
		}

		void setColor(CRGB color) override { _color = color; }

		void reset() override {
			_phase     = 0;
			_x         = 0;
			_holdCount = 0;
		}

		void step(CRGB* leds) override {
			if (_phase == 0) {
				// Efface au début d'un nouveau cycle
				if (_x == 0) fill_solid(leds, NUM_LEDS, CRGB::Black);

				// Dessine la colonne courante — une seule ligne
				leds[XY(_x, _ecg[_x])] = _color;

				_x++;
				if (_x >= MATRIX_W) {
					_x         = 0;
					_phase     = 1;
					_holdCount = 0;
				}

			} else if (_phase == 1) {
				// Maintien de l'image ~10 frames
				if (++_holdCount >= 10) {
					_phase = 2;
				}

			} else {
				// Effacement colonne par colonne de gauche à droite
				for (uint8_t y = 0; y < MATRIX_H; y++) {
					leds[XY(_x, y)] = CRGB::Black;
				}
				_x++;
				if (_x >= MATRIX_W) {
					_phase = 0;
					_x     = 0;
				}
			}
		}

		String name() override { return "HeartBeat"; }
};
