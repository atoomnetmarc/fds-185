/*

Copyright 2021-2023 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <Arduino.h>

#ifndef config_h
#define config_h

extern const char *ssid;
extern const char *password;

extern const char *otapassword;
extern const char *otahostname;

/*
MOSI: 11
SCK: 7
*/

#define OE 3
#define STROBE 16
#define RSA 39
#define RSB 37
#define RSC 35
#define RSU3CS 33
#define RSU6CS 18

#endif