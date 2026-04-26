#pragma once

#include "../setting.h"
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Visualiseur audio — barres larges ───────────────────────────────────────
// 16 barres de 2 colonnes chacune (MATRIX_W / 2).
// Dégradé fixe rouge (bas) → jaune (milieu) → vert (haut).

class EffetAudio2 : public Effect {
	public:
		EffetAudio2() {}

		String name() override { return "Audio2"; }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			fill_solid(leds, NUM_LEDS, CRGB::Black);

			const uint8_t N_BARS = MATRIX_W / 2;  // 16 barres

			for (uint8_t b = 0; b < N_BARS; b++) {
				// Moyenne des deux bandes FFT correspondantes
				float band = (mic->getBand(b * 2) + mic->getBand(b * 2 + 1)) * 0.5f;
				uint8_t barH = (uint8_t)(band * MATRIX_H);

				// ── Dessin des deux colonnes ───────────────────────────────────
				for (uint8_t col = 0; col < 2; col++) {
					uint8_t x = b * 2 + col;

					for (uint8_t y = 0; y < MATRIX_H; y++) {
						uint8_t row = MATRIX_H - 1 - y;

						if (y < barH) {
							leds[XY(x, row)] = _barColor(y);
						}
					}
				}
			}
		}

	private:
		// Dégradé fixe : rouge (y=0) → jaune (y=milieu) → vert (y=haut)
		CRGB _barColor(uint8_t y) {
			float t = (float)y / (MATRIX_H - 1);  // 0.0 (bas) → 1.0 (haut)

			if (t < 0.5f) {
				// rouge → jaune
				uint8_t g = (uint8_t)(t * 2.0f * 255);
				return CRGB(255, g, 0);
			} else {
				// jaune → vert
				uint8_t r = (uint8_t)((1.0f - (t - 0.5f) * 2.0f) * 255);
				return CRGB(r, 255, 0);
			}
		}
};
