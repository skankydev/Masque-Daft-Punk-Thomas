#include "MicManager.h"
#include <arduinoFFT.h>
#include <math.h>

// ─── Singleton ───────────────────────────────────────────────────────────────

MicManager* MicManager::instance = nullptr;

MicManager* MicManager::getInstance() {
	if (!instance) {
		instance = new MicManager;
	}
	return instance;
}

MicManager::MicManager() : _volume(0.0f), _sensitivity(1.0f), _noiseFloor(0.12f) {
	_mutex = xSemaphoreCreateMutex();
	memset(_bands, 0, sizeof(_bands));
	memset(_vReal,  0, sizeof(_vReal));
	memset(_vImag,  0, sizeof(_vImag));
	_computeBinRanges();
	_initI2S();
}

// ─── Init I2S ────────────────────────────────────────────────────────────────

void MicManager::_initI2S() {
	i2s_config_t cfg = {
		.mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate          = MIC_SAMPLE_RATE,
		.bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
		// INMP441 sort sur la voie gauche (WS bas = gauche)
		.channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = I2S_COMM_FORMAT_STAND_I2S,
		.intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count        = 4,
		.dma_buf_len          = MIC_FFT_SAMPLES,
		.use_apll             = false,
		.tx_desc_auto_clear   = false,
		.fixed_mclk           = 0
	};

	i2s_pin_config_t pins = {
		.mck_io_num     = I2S_PIN_NO_CHANGE,
		.bck_io_num     = PIN_MIC_SCK,
		.ws_io_num      = PIN_MIC_WS,
		.data_out_num   = I2S_PIN_NO_CHANGE,
		.data_in_num    = PIN_MIC_SD
	};

	i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr);
	i2s_set_pin(I2S_NUM_0, &pins);
	i2s_zero_dma_buffer(I2S_NUM_0);

	warning("I2S initialisé","Mic");
}

// ─── Mapping logarithmique fréquences → bins ─────────────────────────────────
// 32 bandes de ~40 Hz à ~8000 Hz, échelle logarithmique

void MicManager::_computeBinRanges() {
	const float freqLow  = 40.0f;
	const float freqHigh = 8000.0f;
	const float ratio    = freqHigh / freqLow;

	for (int b = 0; b < MIC_N_BANDS; b++) {
		float fLo = freqLow * powf(ratio, (float)b       / MIC_N_BANDS);
		float fHi = freqLow * powf(ratio, (float)(b + 1) / MIC_N_BANDS);

		// Conversion fréquence → indice de bin
		uint16_t binLo = (uint16_t)(fLo * MIC_FFT_SAMPLES / MIC_SAMPLE_RATE);
		uint16_t binHi = (uint16_t)(fHi * MIC_FFT_SAMPLES / MIC_SAMPLE_RATE);

		_binStart[b] = max((uint16_t)1, binLo);
		_binEnd[b]   = max(_binStart[b], min((uint16_t)(MIC_FFT_SAMPLES / 2 - 1), binHi));
	}
}

// ─── Tâche FreeRTOS ──────────────────────────────────────────────────────────

void MicManager::startTask() {
	xTaskCreatePinnedToCore(
		_task,
		"micTask",
		16384,    // stack — FFT + buffers I2S
		this,
		1,        // priorité normale
		nullptr,
		0         // core 0 (loop() tourne sur core 1)
	);
	success("Tâche démarrée sur core 0","Mic");
}

void MicManager::_task(void* param) {
	static_cast<MicManager*>(param)->_loop();
}

void MicManager::_loop() {
	static int32_t rawBuf[MIC_FFT_SAMPLES];

	ArduinoFFT<float> fft(_vReal, _vImag, MIC_FFT_SAMPLES, MIC_SAMPLE_RATE);

	while (true) {
		// ── Lecture I2S ───────────────────────────────────────────────────────
		size_t bytesRead = 0;
		i2s_read(I2S_NUM_0, rawBuf, sizeof(rawBuf), &bytesRead, portMAX_DELAY);
		int nSamples = bytesRead / sizeof(int32_t);

		if (nSamples < MIC_FFT_SAMPLES) {
			// Trame incomplète — compléter à zéro
			memset(rawBuf + nSamples, 0, (MIC_FFT_SAMPLES - nSamples) * sizeof(int32_t));
		}

		// ── Conversion → float normalisé [-1, 1] ─────────────────────────────
		// INMP441 : données 24-bit dans les bits [31:8] d'un mot 32-bit
		float volSum = 0.0f;
		for (int i = 0; i < MIC_FFT_SAMPLES; i++) {
			int32_t s = rawBuf[i] >> 8;          // décalage pour récupérer 24 bits
			_vReal[i] = (float)s / 8388608.0f;   // normalisation par 2^23
			_vImag[i] = 0.0f;
			volSum += fabsf(_vReal[i]);
		}

		// Volume global (moyenne des amplitudes absolues)
		float vol = volSum / MIC_FFT_SAMPLES;
		// Log scale pour que le volume soit plus lisible
		vol = log10f(1.0f + vol * 100.0f) / 2.0f;
		vol = constrain(vol, 0.0f, 1.0f);

		// ── FFT ───────────────────────────────────────────────────────────────
		fft.windowing(FFTWindow::Hamming, FFTDirection::Forward);
		fft.compute(FFTDirection::Forward);
		fft.complexToMagnitude();
		// _vReal[0..FFT_SAMPLES/2-1] contient maintenant les magnitudes

		// ── Calcul des bandes ─────────────────────────────────────────────────
		// Lecture locale des paramètres volatiles (atomique sur ESP32 32-bit)
		float sens  = _sensitivity;
		float floor = _noiseFloor;

		float raw[MIC_N_BANDS];
		for (int b = 0; b < MIC_N_BANDS; b++) {
			float sum = 0.0f;
			int   cnt = 0;
			for (int k = _binStart[b]; k <= _binEnd[b]; k++) {
				sum += _vReal[k];
				cnt++;
			}
			float avg = (cnt > 0) ? sum / cnt : 0.0f;

			// Sensibilité appliquée avant le log — impact maximal
			avg *= sens;

			// Log scale
			raw[b] = log10f(1.0f + avg * 200.0f) / 3.5f;
			raw[b] = constrain(raw[b], 0.0f, 1.0f);

			// Noise gate : coupe sous le seuil, rescale le reste sur [0, 1]
			if (raw[b] < floor) {
				raw[b] = 0.0f;
			} else {
				raw[b] = (raw[b] - floor) / (1.0f - floor);
			}
		}

		// ── Mise à jour partagée (lissage temporel) ───────────────────────────
		xSemaphoreTake(_mutex, portMAX_DELAY);

		_volume = _volume * 0.6f + vol * 0.4f;

		for (int b = 0; b < MIC_N_BANDS; b++) {
			// Montée rapide, descente lente
			float alpha = (raw[b] > _bands[b]) ? 0.5f : 0.15f;
			_bands[b] = _bands[b] * (1.0f - alpha) + raw[b] * alpha;
		}

		xSemaphoreGive(_mutex);
	}
}

// ─── Sensibilité / noise gate ────────────────────────────────────────────────

void MicManager::setSensitivity(float s) {
	_sensitivity = constrain(s, 0.1f, 5.0f);
	println(vert("sensitivity") + " : " + rouge(String(_sensitivity, 2)),"Mic");
}

float MicManager::getSensitivity() {
	return _sensitivity;
}

// ─── Getters thread-safe ─────────────────────────────────────────────────────

float MicManager::getVolume() {
	xSemaphoreTake(_mutex, portMAX_DELAY);
	float v = _volume;
	xSemaphoreGive(_mutex);
	return v;
}

float MicManager::getBand(uint8_t i) {
	if (i >= MIC_N_BANDS) return 0.0f;
	xSemaphoreTake(_mutex, portMAX_DELAY);
	float v = _bands[i];
	xSemaphoreGive(_mutex);
	return v;
}

float MicManager::getBass() {
	xSemaphoreTake(_mutex, portMAX_DELAY);
	float sum = 0;
	for (int i = 0; i < 4; i++) sum += _bands[i];
	xSemaphoreGive(_mutex);
	return sum / 4.0f;
}

float MicManager::getTreble() {
	xSemaphoreTake(_mutex, portMAX_DELAY);
	float sum = 0;
	for (int i = MIC_N_BANDS - 4; i < MIC_N_BANDS; i++) sum += _bands[i];
	xSemaphoreGive(_mutex);
	return sum / 4.0f;
}
