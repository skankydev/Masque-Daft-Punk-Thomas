#pragma once

#include "../setting.h"
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Visualiseur audio — symétrie 4 quadrants ────────────────────────────────
// L'effet est calculé dans le quart bas-gauche puis mis en miroir
// horizontalement, verticalement et dans les deux sens.
// Dégradé : couleur pleine au sommet de la barre, foncé à la base.

class EffetAudio3 : public Effect {
	public:
		EffetAudio3() : _color(CRGB(0, 200, 255)) {}

		String name() override { return "Audio3"; }

		void setColor(CRGB color) override { _color = color; }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			fill_solid(leds, NUM_LEDS, CRGB::Black);

			const uint8_t HALF_W = MATRIX_W / 2;  // 16
			const uint8_t HALF_H = MATRIX_H / 2;  // 4

			for (uint8_t x = 0; x < HALF_W; x++) {
				// Deux bandes FFT moyennées par colonne
				float band = (mic->getBand(x * 2) + mic->getBand(x * 2 + 1)) * 0.5f;
				uint8_t barH = (uint8_t)(band * HALF_H);

				for (uint8_t h = 0; h < barH; h++) {
					// h=0 → base de la barre (bas), h=barH-1 → sommet
					// t=0 → foncé, t=1 → couleur pleine
					float t = (barH > 1) ? (float)h / (barH - 1) : 1.0f;
					CRGB c = CRGB(
						(uint8_t)(_color.r * (0.15f + 0.85f * t)),
						(uint8_t)(_color.g * (0.15f + 0.85f * t)),
						(uint8_t)(_color.b * (0.15f + 0.85f * t))
					);

					// Coordonnées dans le quart bas-gauche
					// h=0 → y = MATRIX_H-1 (tout en bas), h monte → y remonte
					uint8_t y = MATRIX_H - 1 - h;

					// ── 4 quadrants par symétrie ───────────────────────────────
					uint8_t xR = MATRIX_W - 1 - x;  // miroir horizontal
					uint8_t yT = MATRIX_H - 1 - y;  // miroir vertical

					leds[XY(x,  y )] = c;  // bas-gauche  (base)
					leds[XY(xR, y )] = c;  // bas-droit   (miroir H)
					leds[XY(x,  yT)] = c;  // haut-gauche (miroir V)
					leds[XY(xR, yT)] = c;  // haut-droit  (miroir H+V)
				}
			}
		}

	private:
		CRGB _color;
};
