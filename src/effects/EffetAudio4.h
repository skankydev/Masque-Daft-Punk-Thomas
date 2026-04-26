#pragma once

#include "../setting.h"
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Visualiseur audio — explosion depuis le centre ───────────────────────────
// Les barres naissent au centre de la matrice et poussent vers les 4 coins.
// Dégradé : foncé au centre, couleur pleine aux extrémités.

class EffetAudio4 : public Effect {
	public:
		EffetAudio4() : _color(CRGB(0, 200, 255)) {}

		String name() override { return "Audio4"; }

		void setColor(CRGB color) override { _color = color; }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			fill_solid(leds, NUM_LEDS, CRGB::Black);

			const uint8_t HALF_W = MATRIX_W / 2;  // 16
			const uint8_t HALF_H = MATRIX_H / 2;  // 4

			// x part du centre vers la droite, xL est son miroir vers la gauche
			// → bande 0 (basses, plus d'énergie) au centre, hautes fréquences aux bords
			for (uint8_t x = HALF_W; x < MATRIX_W; x++) {
				uint8_t bandIdx = x - HALF_W;  // 0 (centre) → 15 (bord)
				float band = (mic->getBand(bandIdx * 2) + mic->getBand(bandIdx * 2 + 1)) * 0.5f;
				uint8_t barH = (uint8_t)(band * HALF_H);

				for (uint8_t h = 0; h < barH; h++) {
					// h=0 → centre (foncé), h=barH-1 → coin (couleur pleine)
					float t = (barH > 1) ? (float)h / (barH - 1) : 1.0f;
					CRGB c = CRGB(
						(uint8_t)(_color.r * (0.15f + 0.85f * t)),
						(uint8_t)(_color.g * (0.15f + 0.85f * t)),
						(uint8_t)(_color.b * (0.15f + 0.85f * t))
					);

					uint8_t yUp = HALF_H - 1 - h;   // centre → haut
					uint8_t yDown = HALF_H + h;      // centre → bas
					uint8_t xL = MATRIX_W - 1 - x;  // miroir gauche

					leds[XY(x,  yUp  )] = c;  // droite haut
					leds[XY(xL, yUp  )] = c;  // gauche haut
					leds[XY(x,  yDown)] = c;  // droite bas
					leds[XY(xL, yDown)] = c;  // gauche bas
				}
			}
		}

	private:
		CRGB _color;
};
