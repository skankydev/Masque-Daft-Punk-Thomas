#ifndef DEF_SETTING
#define DEF_SETTING


#include <stdlib.h>
#include <stdio.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <SPI.h>
#include "function.h"

#define MATRIX_W  32
#define MATRIX_H   8
#define NUM_LEDS  (MATRIX_W * MATRIX_H)

#define PIN_LEDS  15

// INMP441 — micro numérique I2S
#define PIN_MIC_SCK  41
#define PIN_MIC_WS   42
#define PIN_MIC_SD    2

#endif
