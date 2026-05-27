#pragma once
#include "Effect.h"

class EffetFireworks : public Effect {

		static const uint8_t MAX_PARTICULES = 40;
		static const uint8_t PAR_EXPLOSION  = 8;  // particules par explosion
		static const uint8_t MAX_EXPLOSIONS = 3;  // explosions simultanées max

		struct Particule {
			float   x, y;
			float   vx, vy;
			uint8_t brightness;
			CRGB    color;
			bool    active;
		};

		Particule _pool[MAX_PARTICULES];

		// Palette festive pour les explosions
		static const CRGB PALETTE[];
		static const uint8_t NB_COULEURS = 8;

		// Buffer de persistance (trainée légère)
		uint8_t _buffer[MATRIX_W][MATRIX_H];

		uint8_t _explosionCount; // explosions actives

		void _newExplosion() {
			// Centre aléatoire (évite les bords d'1px)
			float cx = random(1, MATRIX_W - 1);
			float cy = random(1, MATRIX_H - 1);
			CRGB  col = PALETTE[random(NB_COULEURS)];

			uint8_t spawned = 0;
			for (uint8_t i = 0; i < MAX_PARTICULES && spawned < PAR_EXPLOSION; i++) {
				if (_pool[i].active) continue;

				// Angle réparti uniformément + légère variation
				float angle = (TWO_PI / PAR_EXPLOSION) * spawned
							+ random(-10, 10) / 100.0f;
				float speed = random(4, 10) / 10.0f; // vitesse modérée pour la taille de la matrice

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
			memset(_pool,   0, sizeof(_pool));
			memset(_buffer, 0, sizeof(_buffer));
			_explosionCount = 0;

			// Première explosion dès le départ
			_newExplosion();
		}

		void step(CRGB* leds) override {
			// Fade du buffer de persistance
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				for (uint8_t y = 0; y < MATRIX_H; y++) {
					if (_buffer[x][y] > 30) {
						_buffer[x][y] = _buffer[x][y] * 5 / 8;
					} else {
						_buffer[x][y] = 0;
					}
				}
			}

			// Mise à jour des particules
			uint8_t actives = 0;
			for (uint8_t i = 0; i < MAX_PARTICULES; i++) {
				if (!_pool[i].active) continue;
				actives++;

				// Déplacement + légère gravité
				_pool[i].x  += _pool[i].vx;
				_pool[i].y  += _pool[i].vy;
				_pool[i].vy += 0.08f; // gravité douce

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
					continue;
				}

				// Écriture dans le buffer (garde le max)
				if (_pool[i].brightness > _buffer[px][py]) {
					_buffer[px][py] = _pool[i].brightness;
					// Associer la couleur au pixel via les leds directement
				}
			}

			// Nouvelle explosion si besoin
			if (actives == 0 || (actives < PAR_EXPLOSION * MAX_EXPLOSIONS && random(10) < 3)) {
				_newExplosion();
			}

			// Rendu : fond noir + particules
			fill_solid(leds, NUM_LEDS, CRGB::Black);

			for (uint8_t i = 0; i < MAX_PARTICULES; i++) {
				if (!_pool[i].active) continue;
				int8_t px = (int8_t)_pool[i].x;
				int8_t py = (int8_t)_pool[i].y;
				if (px < 0 || px >= MATRIX_W || py < 0 || py >= MATRIX_H) continue;

				// Couleur scalée par brightness
				CRGB c = _pool[i].color;
				c.nscale8(_pool[i].brightness);

				// Additionner (plusieurs particules peuvent se superposer)
				leds[XY(px, py)] += c;
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			// Flashs colorés — chaque LED clignote indépendamment
			uint32_t t = millis();
			for (uint8_t i = 0; i < len; i++) {
				uint8_t phase = (t / 80 + i * 41) % 200;
				if (phase < 30) {
					strip[i] = PALETTE[(t / 400 + i) % NB_COULEURS];
					strip[i].nscale8(255 - phase * 7);
				} else {
					strip[i] = CRGB::Black;
				}
			}
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			// Blocs fixes festifs — une couleur de palette par LED
			for (uint8_t i = 0; i < len; i++) {
				strip[i] = PALETTE[i % NB_COULEURS];
				strip[i].nscale8(80);
			}
		}

		String name() override { return "Fireworks"; }
};

const CRGB EffetFireworks::PALETTE[] = {
	CRGB(255,  80,   0), // orange
	CRGB(255,   0,  80), // rose
	CRGB(  0, 200, 255), // cyan
	CRGB(255, 220,   0), // jaune
	CRGB(  0, 255, 100), // vert
	CRGB(180,   0, 255), // violet
	CRGB(255,  30,  30), // rouge
	CRGB(255, 255, 255), // blanc
};
