/*

Copyright 2023 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Arduino.h>
#include <SPI.h>

#include "config.h"
#include "wific.h"
#include "main.h"
#include "gameoflife.h"

static const int spiClk = 5 * 1000000;

volatile uint8_t matrixA[MATRIXSIZE];
volatile uint8_t matrixB[MATRIXSIZE];

volatile uint8_t *matrix = matrixA;
volatile uint8_t *matrixNew = matrixB;

// Row 0-15
void rowSelect(uint8_t row)
{
    digitalWrite(RSU3CS, LOW);
    digitalWrite(RSU6CS, LOW);

    row = 7 - row;

    digitalWrite(RSA, (row & 0b0001));
    digitalWrite(RSB, (row & 0b0010));
    digitalWrite(RSC, (row & 0b0100));

    if ((row & 0b1000))
        digitalWrite(RSU6CS, HIGH);
    else
        digitalWrite(RSU3CS, HIGH);
}

volatile uint8_t animate;

TaskHandle_t shiftHandlerTask;
void shiftHandler(void *param)
{
    uint8_t y = 0;

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        uint8_t buffer[SHIFTSIZEROW];
        memcpy(buffer, (void *)&matrix[y * SHIFTSIZEROW], SHIFTSIZEROW);

        SPI.beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
        SPI.transfer(buffer, SHIFTSIZEROW);
        SPI.endTransaction();

        digitalWrite(OE, HIGH); // Disable output.

        rowSelect(y);

        digitalWrite(STROBE, HIGH);
        digitalWrite(STROBE, LOW);

        delayMicroseconds(1);
        digitalWrite(OE, LOW); // Enable output.

        if (++y == 16)
            y = 0;
    }
}

void IRAM_ATTR shiftTimer()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(shiftHandlerTask, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

uint16_t IRAM_ATTR getPixelByteAddress(int8_t x, int8_t y)
{
    int8_t yInvert = MATRIX_HEIGHT - 1 - y;
    int8_t xInvert = MATRIX_WIDTH - 1 - x;

    uint8_t shiftrow = yInvert % MATRIXROWS;
    uint8_t byteoffsetY = (yInvert / MATRIXROWS) * (MATRIX_WIDTH / 8);
    uint8_t byteoffsetX = (xInvert / 8) % MATRIX_WIDTH;

    uint16_t address = (shiftrow * SHIFTSIZEROW) + byteoffsetX + byteoffsetY;

    return address;
}

void IRAM_ATTR setPixel(int8_t x, int8_t y, bool color)
{
    if (x >= MATRIX_WIDTH ||
        x < 0 ||
        y >= MATRIX_HEIGHT ||
        y < 0)
        return;

    uint16_t address = getPixelByteAddress(x, y);
    uint8_t bit = 7 - (x % 8);

    if (color)
        matrixNew[address] |= _BV(bit);
    else
        matrixNew[address] &= ~_BV(bit);
}

bool IRAM_ATTR getPixel(int8_t x, int8_t y)
{
    if (x >= MATRIX_WIDTH)
        x -= MATRIX_WIDTH;
    if (x < 0)
        x += MATRIX_WIDTH;

    if (y >= MATRIX_HEIGHT)
        y -= MATRIX_HEIGHT;
    if (y < 0)
        y += MATRIX_HEIGHT;

    uint16_t address = getPixelByteAddress(x, y);
    uint8_t bit = 7 - (x % 8);

    return matrix[address] & _BV(bit);
}

hw_timer_t *shift_timer = NULL;

void setup()
{
    // Output enable
    digitalWrite(OE, HIGH);
    pinMode(OE, OUTPUT);

    // Strobe enable
    pinMode(STROBE, OUTPUT);

    // Rowselect
    pinMode(RSA, OUTPUT);
    pinMode(RSB, OUTPUT);
    pinMode(RSC, OUTPUT);
    pinMode(RSU3CS, OUTPUT);
    pinMode(RSU6CS, OUTPUT);

    Serial.begin(115200);
    SPI.begin();

    xTaskCreate(shiftHandler, "LED Matrix update", 8192, NULL, 30, &shiftHandlerTask);
    shift_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(shift_timer, &shiftTimer, true);
    timerAlarmWrite(shift_timer, 1000, true);
    timerAlarmEnable(shift_timer);

    gameSetup();

    pinMode(0, INPUT_PULLUP);

    WificSetup();
}

void loop()
{
    static uint8_t x = 0, y = 0;
    static uint16_t delaycounter = 0;

    if (digitalRead(0) == LOW)
    {
        memset((void *)matrixNew, 0, MATRIXSIZE);
    }

    WiFicLoop();
}