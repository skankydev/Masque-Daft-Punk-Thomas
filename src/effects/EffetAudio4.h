#pragma once

#include "../setting.h"
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Visualiseur audio — explosion depuis le centre, couleur réactive ────────
// Les barres naissent au centre et poussent vers les 4 coins.
// La couleur suit la dominante spectrale : basses = rouge, aigus = bleu/violet.
// Les strips top pulsent au beat (couleur réactive aussi).

class EffetAudio4 : public Effect {
	public:
		EffetAudio4() {}

		String name() override { return "Audio4"; }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			fill_solid(leds, NUM_LEDS, CRGB::Black);

			// Couleur audio-réactive (calculée une fois par frame, pas par pixel)
			CRGB color = audioColor(mic->getSpectralCentroid());

			const uint8_t HALF_W = MATRIX_W / 2;  // 16
			const uint8_t HALF_H = MATRIX_H / 2;  // 4

			// x part du centre vers la droite, xL est son miroir vers la gauche
			// → bande 0 (basses, plus d'énergie) au centre, hautes fréquences aux bords
			for (uint8_t x = HALF_W; x < MATRIX_W; x++) {
				uint8_t bandIdx = x - HALF_W;  // 0 (centre) → 15 (bord)
				float band = (mic->getBand(bandIdx * 2) + mic->getBand(bandIdx * 2 + 1)) * 0.5f;
				uint8_t barH = (uint8_t)(band * HALF_H);

				for (uint8_t h = 0; h < barH; h++) {
					// Dégradé : centre = 15% (38/255) → coin = 100% (255/255)
					uint8_t frac   = (barH > 1) ? (h * 255) / (barH - 1) : 255;
					uint8_t bright = lerp8by8(38, 255, frac);
					CRGB c = color;
					c.nscale8(bright);

					uint8_t yUp   = HALF_H - 1 - h;   // centre → haut
					uint8_t yDown = HALF_H + h;       // centre → bas
					uint8_t xL    = MATRIX_W - 1 - x; // miroir gauche

					leds[XY(x,  yUp  )] = c;  // droite haut
					leds[XY(xL, yUp  )] = c;  // gauche haut
					leds[XY(x,  yDown)] = c;  // droite bas
					leds[XY(xL, yDown)] = c;  // gauche bas
				}
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// Pulse beat avec couleur audio-réactive
			float beat = mic->getBeatLevel();
			CRGB color  = audioColor(mic->getSpectralCentroid());
			uint8_t bright = (uint8_t)(beat * 255);
			CRGB c = CRGB(
				scale8(color.r, bright),
				scale8(color.g, bright),
				scale8(color.b, bright)
			);
			for (uint8_t i = 0; i < len; i++) strip[i] = c;
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// VU global avec couleur réactive
			float avg = 0;
			for (uint8_t i = 0; i < MATRIX_W; i++) avg += mic->getBand(i);
			avg /= MATRIX_W;
			uint8_t filled = (uint8_t)(avg * len);
			CRGB color = audioColor(mic->getSpectralCentroid());
			for (uint8_t i = 0; i < len; i++)
				strip[i] = (i < filled) ? color : CRGB::Black;
		}
};
