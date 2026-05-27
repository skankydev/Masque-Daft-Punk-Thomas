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

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			float avg = 0;
			for (uint8_t i = 0; i < MATRIX_W; i++) avg += mic->getBand(i);
			avg /= MATRIX_W;
			uint8_t filled = (uint8_t)(avg * len);
			for (uint8_t i = 0; i < len; i++)
				strip[i] = (i < filled) ? _color : CRGB::Black;
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// Basses — centre de l'explosion
			float bass = 0;
			for (uint8_t i = 0; i < MATRIX_W / 4; i++) bass += mic->getBand(i);
			bass /= (MATRIX_W / 4);
			uint8_t bright = (uint8_t)(bass * 255);
			uint8_t third  = len / 3;
			for (uint8_t i = 0; i < len; i++) {
				if      (i < third)      strip[i] = CRGB(bright, 0,          0);
				else if (i < third * 2)  strip[i] = CRGB(bright / 2, bright / 2, 0);
				else                     strip[i] = CRGB(0,      bright,     0);
			}
		}

	private:
		CRGB _color;
};
