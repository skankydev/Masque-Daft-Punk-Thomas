#pragma once

#include "../setting.h"
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Visualiseur audio — explosions radiales sur beat ────────────────────────
// Un beat = une rafale de particules qui partent du centre dans toutes les
// directions, retombent doucement par fade, et meurent en sortant de l'écran.
// Couleur fixe via setColor() — pour avoir une identité visuelle propre.

#define PART_MAX        40    // particules simultanées max
#define PART_BURST      10    // particules spawnées par beat
#define PART_TRAIL      40    // intensité du fade entre frames (0=traînée infinie, 255=pas de traînée)
#define PART_LIFE_DECAY 6     // life perdue par frame (255 / 6 ≈ 42 frames)

class EffetAudioParticles : public Effect {
	public:
		EffetAudioParticles() : _color(CRGB(255, 80, 0)), _beatArmed(true) {
			for (uint8_t i = 0; i < PART_MAX; i++) _particles[i].life = 0;
		}

		String name() override { return "Particles"; }

		void setColor(CRGB color) override { _color = color; }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			// ── Trigger : un beat = une rafale ────────────────────────────────
			// Armé/désarmé pour éviter de re-déclencher pendant la retombée
			float beat = mic->getBeatLevel();
			if (beat > 0.7f && _beatArmed) {
				_spawnBurst(PART_BURST);
				_beatArmed = false;
			} else if (beat < 0.4f) {
				_beatArmed = true;
			}

			// ── Update : avance, fade, kill hors écran ────────────────────────
			for (uint8_t i = 0; i < PART_MAX; i++) {
				Particle& p = _particles[i];
				if (p.life == 0) continue;

				p.x += p.vx;
				p.y += p.vy;

				if (p.x < 0 || p.x >= MATRIX_W || p.y < 0 || p.y >= MATRIX_H) {
					p.life = 0;
					continue;
				}

				p.life = (p.life > PART_LIFE_DECAY) ? p.life - PART_LIFE_DECAY : 0;
			}

			// ── Render ────────────────────────────────────────────────────────
			// Fade de tout l'écran pour créer une traînée légère
			fadeToBlackBy(leds, NUM_LEDS, PART_TRAIL);

			for (uint8_t i = 0; i < PART_MAX; i++) {
				const Particle& p = _particles[i];
				if (p.life == 0) continue;

				uint8_t x = (uint8_t)p.x;
				uint8_t y = (uint8_t)p.y;

				CRGB c = _color;
				c.nscale8(p.life);
				leds[XY(x, y)] += c;  // additif pour gérer les overlaps
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// Pulse beat — même couleur que les particules
			float beat = mic->getBeatLevel();
			CRGB c = _color;
			c.nscale8((uint8_t)(beat * 255));
			for (uint8_t i = 0; i < len; i++) strip[i] = c;
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// VU global — couleur fixe
			float avg = 0;
			for (uint8_t i = 0; i < MATRIX_W; i++) avg += mic->getBand(i);
			avg /= MATRIX_W;
			uint8_t filled = (uint8_t)(avg * len);
			for (uint8_t i = 0; i < len; i++)
				strip[i] = (i < filled) ? _color : CRGB::Black;
		}

	private:
		struct Particle {
			float   x, y;
			float   vx, vy;
			uint8_t life;  // 0 = morte
		};

		Particle _particles[PART_MAX];
		CRGB     _color;
		bool     _beatArmed;

		// Spawn une rafale de particules depuis le centre dans des directions aléatoires
		void _spawnBurst(uint8_t count) {
			const float cx = MATRIX_W / 2.0f;
			const float cy = MATRIX_H / 2.0f;

			for (uint8_t n = 0; n < count; n++) {
				// Cherche un slot mort
				Particle* p = nullptr;
				for (uint8_t i = 0; i < PART_MAX; i++) {
					if (_particles[i].life == 0) { p = &_particles[i]; break; }
				}
				if (!p) return;  // tableau plein, on abandonne le reste de la rafale

				// Angle aléatoire 0-360°, vélocité 0.3 à 0.8 pixels/frame
				float angle = (float)random(360) * (PI / 180.0f);
				float speed = 0.3f + (float)random(100) / 200.0f;

				p->x    = cx;
				p->y    = cy;
				p->vx   = cosf(angle) * speed;
				p->vy   = sinf(angle) * speed;
				p->life = 255;
			}
		}
};
