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

// Bandes latérales — 4 strips de 10 LEDs
#define PIN_STRIP_RIGHT_TOP   11
#define PIN_STRIP_RIGHT_BOT   12
#define PIN_STRIP_LEFT_TOP    17
#define PIN_STRIP_LEFT_BOT    18

#define NUM_STRIP_RIGHT_TOP   10
#define NUM_STRIP_RIGHT_BOT   10
#define NUM_STRIP_LEFT_TOP    10
#define NUM_STRIP_LEFT_BOT    10

// INMP441 — micro numérique I2S
#define PIN_MIC_SCK  41
#define PIN_MIC_WS   42
#define PIN_MIC_SD    2

#endif

