/*

Copyright 2021-2023 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <ArduinoOTA.h>

#include "config.h"

void OTASetup(void)
{
    ArduinoOTA.setPort(3232);
    ArduinoOTA.setHostname(otahostname);
    ArduinoOTA.setPassword(otapassword);

    ArduinoOTA.onStart([]()
                       { Serial.println(F("OTA Start")); });
    ArduinoOTA.onEnd([]()
                     { Serial.println(F("\nOTA End")); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf_P(PSTR("OTA Progress: %u%%\r"), (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
                           Serial.printf_P(PSTR("OTA Error[%u]: "), error);
                           if (error == OTA_AUTH_ERROR)
                               Serial.println(F("OTA Auth Failed"));
                           else if (error == OTA_BEGIN_ERROR)
                               Serial.println(F("OTA Begin Failed"));
                           else if (error == OTA_CONNECT_ERROR)
                               Serial.println(F("OTA Connect Failed"));
                           else if (error == OTA_RECEIVE_ERROR)
                               Serial.println(F("OTA Receive Failed"));
                           else if (error == OTA_END_ERROR)
                               Serial.println(F("OTA End Failed"));
                       });
    ArduinoOTA.begin();
}

void OTALoop()
{
    ArduinoOTA.handle();
}