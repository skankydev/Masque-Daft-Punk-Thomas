#pragma once

#include "../setting.h"
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Visualiseur audio ────────────────────────────────────────────────────────
// Affiche 32 barres verticales (une par colonne) correspondant aux bandes
// de fréquences du microphone INMP441.
// setColor() change la teinte de base — les barres dégradent vers le blanc
// au sommet. Par défaut : cyan.

class EffetAudio : public Effect {
	public:
		EffetAudio() : _color(CRGB(0, 200, 255)) {
			memset(_peaks, 0, sizeof(_peaks));
		}

		String name() override { return "Audio"; }

		void setColor(CRGB color) override { _color = color; }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			// Efface la matrice
			fill_solid(leds, NUM_LEDS, CRGB::Black);

			for (uint8_t x = 0; x < MATRIX_W; x++) {
				float band = mic->getBand(x);

				// Hauteur en pixels (0 à MATRIX_H)
				uint8_t barH = (uint8_t)(band * MATRIX_H);

				// ── Peak indicator ─────────────────────────────────────────────
				// Monte instantanément, redescend d'un pixel toutes les 3 frames
				if (barH >= _peaks[x]) {
					_peaks[x] = barH;
				} else {
					if ((_frame % 3) == 0 && _peaks[x] > 0) {
						_peaks[x]--;
					}
				}

				// ── Dessin de la barre ─────────────────────────────────────────
				for (uint8_t y = 0; y < MATRIX_H; y++) {
					uint8_t row = MATRIX_H - 1 - y; // row 0 = haut, on dessine de bas en haut

					if (y < barH) {
						// Dernière led (sommet) en blanc, reste en couleur de base
						leds[XY(x, row)] = (y == barH - 1) ? CRGB::White : _color;
					}

					// Pixel peak (blanc légèrement teinté)
					if (_peaks[x] > 0 && row == MATRIX_H - _peaks[x]) {
						leds[XY(x, row)] = CRGB(
							lerp8by8(_color.r, 255, 200),
							lerp8by8(_color.g, 255, 200),
							lerp8by8(_color.b, 255, 200)
						);
					}
				}
			}

			_frame++;
		}

	private:
		CRGB    _color;
		uint8_t _peaks[MATRIX_W];  // position du pic par colonne
		uint32_t _frame = 0;
};
