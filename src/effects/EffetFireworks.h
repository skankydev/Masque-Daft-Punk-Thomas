#pragma once
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Feu d'artifice audio-réactif ────────────────────────────────────────────
// Une explosion à chaque beat, position aléatoire dans la matrice, couleur qui
// suit la dominante spectrale (audioColor). Gravité douce + trainée pour
// l'esthétique pyrotechnique classique.

class EffetFireworks : public Effect {

		static const uint8_t MAX_PARTICULES = 40;
		static const uint8_t PAR_EXPLOSION  = 8;  // particules par explosion

		struct Particule {
			float   x, y;
			float   vx, vy;
			uint8_t brightness;
			CRGB    color;
			bool    active;
		};

		Particule _pool[MAX_PARTICULES];
		bool      _beatArmed;

		void _newExplosion(CRGB col) {
			// Centre aléatoire (évite les bords d'1px)
			float cx = random(1, MATRIX_W - 1);
			float cy = random(1, MATRIX_H - 1);

			uint8_t spawned = 0;
			for (uint8_t i = 0; i < MAX_PARTICULES && spawned < PAR_EXPLOSION; i++) {
				if (_pool[i].active) continue;

				// Angle réparti uniformément + légère variation
				float angle = (TWO_PI / PAR_EXPLOSION) * spawned
							+ random(-10, 10) / 100.0f;
				float speed = random(4, 10) / 10.0f;

				_pool[i].x          = cx;
				_pool[i].y          = cy;
				_pool[i].vx         = cosf(angle) * speed;
				_pool[i].vy         = sinf(angle) * speed;
				_pool[i].brightness = 255;
				_pool[i].color      = col;
				_pool[i].active     = true;
				spawned++;
			}
		}

	public:
		EffetFireworks() {
			reset();
		}

		void reset() override {
			memset(_pool, 0, sizeof(_pool));
			_beatArmed = true;
		}

		String name() override { return "Fireworks"; }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			// ── Trigger : un beat = une nouvelle explosion ─────────────────────
			float beat = mic->getBeatLevel();
			if (beat > 0.7f && _beatArmed) {
				_newExplosion(audioColor(mic->getSpectralCentroid()));
				_beatArmed = false;
			} else if (beat < 0.4f) {
				_beatArmed = true;
			}

			// ── Mise à jour des particules ─────────────────────────────────────
			for (uint8_t i = 0; i < MAX_PARTICULES; i++) {
				if (!_pool[i].active) continue;

				// Déplacement + gravité
				_pool[i].x  += _pool[i].vx;
				_pool[i].y  += _pool[i].vy;
				_pool[i].vy += 0.08f;

				// Fade
				if (_pool[i].brightness > 20) {
					_pool[i].brightness = _pool[i].brightness * 6 / 8;
				} else {
					_pool[i].active = false;
					continue;
				}

				// Hors matrice → mort
				int8_t px = (int8_t)_pool[i].x;
				int8_t py = (int8_t)_pool[i].y;
				if (px < 0 || px >= MATRIX_W || py < 0 || py >= MATRIX_H) {
					_pool[i].active = false;
				}
			}

			// ── Rendu : fade léger pour traînée + particules en additif ────────
			fadeToBlackBy(leds, NUM_LEDS, 60);

			for (uint8_t i = 0; i < MAX_PARTICULES; i++) {
				if (!_pool[i].active) continue;
				int8_t px = (int8_t)_pool[i].x;
				int8_t py = (int8_t)_pool[i].y;
				if (px < 0 || px >= MATRIX_W || py < 0 || py >= MATRIX_H) continue;

				CRGB c = _pool[i].color;
				c.nscale8(_pool[i].brightness);
				leds[XY(px, py)] += c;
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// Pulse beat couleur réactive
			float beat = mic->getBeatLevel();
			CRGB c = audioColor(mic->getSpectralCentroid());
			c.nscale8((uint8_t)(beat * 255));
			for (uint8_t i = 0; i < len; i++) strip[i] = c;
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// VU couleur réactive
			float avg = 0;
			for (uint8_t i = 0; i < MATRIX_W; i++) avg += mic->getBand(i);
			avg /= MATRIX_W;
			uint8_t filled = (uint8_t)(avg * len);
			CRGB color = audioColor(mic->getSpectralCentroid());
			for (uint8_t i = 0; i < len; i++)
				strip[i] = (i < filled) ? color : CRGB::Black;
		}
};
