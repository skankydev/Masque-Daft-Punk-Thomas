#pragma once

#include "../setting.h"
#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ─── Paramètres FFT ──────────────────────────────────────────────────────────
#define MIC_SAMPLE_RATE  22050
#define MIC_FFT_SAMPLES  512      // doit être une puissance de 2
#define MIC_N_BANDS      MATRIX_W // une bande par colonne

class MicManager {
	public:
		static MicManager* getInstance();

		// Lance la tâche FreeRTOS sur core 0
		void startTask();

		// Sensibilité et noise gate
		void  setSensitivity(float s);   // multiplicateur signal, défaut 1.0 (0.1 – 5.0)
		float getSensitivity();

		// Getters — thread-safe
		float getVolume();                // 0.0 – 1.0 niveau global
		float getBand(uint8_t i);        // 0.0 – 1.0 par bande (i < MIC_N_BANDS)
		float getBass();                  // moyenne bandes basses (0-3)
		float getTreble();                // moyenne bandes hautes (28-31)

	private:
		MicManager();

		void _initI2S();
		void _computeBinRanges();         // mapping log fréquence → bins FFT
		void _loop();                     // boucle de la tâche

		static void _task(void* param);
		static MicManager* instance;

		SemaphoreHandle_t _mutex;

		// Volatiles : écrits depuis loop(), lus depuis micTask — atomique sur ESP32
		volatile float _sensitivity;  // multiplicateur avant log scale (défaut 1.0)
		volatile float _noiseFloor;   // seuil de coupure après log scale (défaut 0.08)

		// Données partagées (protégées par _mutex)
		float _bands[MIC_N_BANDS];
		float _volume;

		// Buffers FFT (alloués dans l'objet, pas sur la stack de la tâche)
		float _vReal[MIC_FFT_SAMPLES];
		float _vImag[MIC_FFT_SAMPLES];

		// Mapping bandes → bins
		uint16_t _binStart[MIC_N_BANDS];
		uint16_t _binEnd[MIC_N_BANDS];
};
