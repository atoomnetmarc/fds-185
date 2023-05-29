/*

Copyright 2021-2022 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <Arduino.h>

#include "main.h"
#include "gameoflife.h"

// Get the number of alive neighbours.
uint8_t IRAM_ATTR countNeighbours(int8_t x, int8_t y)
{
    uint8_t n = 0;
    n += (int)getPixel(x - 1, y - 1); // left above
    n += (int)getPixel(x, y - 1);     // above
    n += (int)getPixel(x + 1, y - 1); // right above
    n += (int)getPixel(x - 1, y);     // left
    n += (int)getPixel(x + 1, y);     // right
    n += (int)getPixel(x - 1, y + 1); // left below
    n += (int)getPixel(x, y + 1);     // below
    n += (int)getPixel(x + 1, y + 1); // right below

    return n;
}

void IRAM_ATTR randomize(void)
{
    int8_t centerX = MATRIX_WIDTH / 2;
    int8_t centerY = MATRIX_HEIGHT / 2;
    int8_t margin = 4 + esp_random() % 8;

    for (int8_t x = centerX - margin; x < centerX + margin; x++)
    {
        yield();
        for (int8_t y = centerY - margin; y < centerY + margin; y++)
        {
            setPixel(x, y, (esp_random() % 2));
        }
    }
}

uint8_t IRAM_ATTR countbits(uint8_t var)
{
    uint8_t bits = 0;

    for (uint8_t i = 0; i < 8; i++)
        if (var & _BV(i))
            bits++;

    return bits;
}

TaskHandle_t gameTask;
// Update the LED matrix with a new frame
void IRAM_ATTR game(void *param)
{
    TickType_t xLastWakeTime;
    const TickType_t xTicks = 250 / portTICK_PERIOD_MS;
    BaseType_t xWasDelayed;

    uint16_t deadcounter = 0;
    uint16_t stablecounter = 0;
    uint16_t possiblystablecounter = 0;
    uint16_t samecounter = 0;
    uint16_t cyclecounter = 0;

    xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        // Determine game if game is frozen, dead or alive.

        uint16_t matrixNewPixels = 0;
        uint16_t matrixPixels = 0;
        bool sameMatrix = true;
        for (uint16_t i = 0; i < MATRIXSIZE; i++)
        {
            yield();
            matrixNewPixels += countbits(matrixNew[i]);
            matrixPixels += countbits(matrix[i]);

            if (matrixNew[i] != matrix[i])
                sameMatrix = false;
        }

        if (matrixNewPixels == 0)
            deadcounter++;
        else if (sameMatrix)
            samecounter++;
        else if (matrixNewPixels == matrixPixels)
            stablecounter++;
        else if (abs(matrixNewPixels - matrixPixels) < 5)
            possiblystablecounter++;
        else
            samecounter = possiblystablecounter = stablecounter = deadcounter = 0;

        cyclecounter++;

        /*
        char buffer[128];
        sprintf(buffer, "deadcounter:%2d, samecounter:%2d, possiblystablecounter: %3d, stablecounter: %2d, cyclecounter:%6d", deadcounter, samecounter, possiblystablecounter, stablecounter, cyclecounter);
        Serial.println(buffer);
        */

        // Erase matrix if number of cells alive stays the same for a while or if pattern does not end.
        if (samecounter > 8 || stablecounter > 40 || possiblystablecounter > 500 || cyclecounter == UINT16_MAX)
        {
            memset((void *)matrixNew, 0, MATRIXSIZE);
            cyclecounter = 0;
        }

        // Randomize matrix if all cells dead.
        if (deadcounter >= 4)
            randomize();

        // Flip buffers to draw matrix to display.
        if (matrix == matrixA)
        {
            matrix = matrixB;
            matrixNew = matrixA;
        }
        else
        {
            matrix = matrixA;
            matrixNew = matrixB;
        }

        // Loop though all pixels.
        for (int8_t y = 0; y < MATRIX_HEIGHT; y++)
        {
            yield();
            for (int8_t x = 0; x < MATRIX_WIDTH; x++)
            {
                // Determine life or death based on number of neighboring cells.
                uint8_t neighbours = countNeighbours(x, y);

                switch (neighbours)
                {
                case 2:
                    setPixel(x, y, getPixel(x, y));
                    break;
                case 3:
                    setPixel(x, y, true);
                    break;
                default:
                    setPixel(x, y, false);
                }
            }
        }

        xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xTicks);
    }
}

void gameSetup(void)
{
    randomize();
    xTaskCreate(game, "Game of life", 8192, NULL, 25, &gameTask);
}
