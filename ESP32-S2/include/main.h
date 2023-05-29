/*

Copyright 2023 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <stdint.h>

#define MATRIX_WIDTH 48
#define MATRIX_HEIGHT 80
#define MATRIXSIZE (MATRIX_WIDTH * MATRIX_HEIGHT / 8)
#define MATRIXROWS 16
#define SHIFTSIZEROW (MATRIX_WIDTH * MATRIX_HEIGHT / MATRIXROWS / 8)

extern volatile uint8_t matrixA[MATRIXSIZE];
extern volatile uint8_t matrixB[MATRIXSIZE];

extern volatile uint8_t *matrix;
extern volatile uint8_t *matrixNew;

void setPixel(int8_t x, int8_t y, bool color);
bool getPixel(int8_t x, int8_t y);