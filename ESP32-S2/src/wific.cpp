/*

Copyright 2021-2023 Marc Ketel
SPDX-License-Identifier: Apache-2.0

*/

#include <WiFi.h>
#include <ESPmDNS.h>

#include "main.h"
#include "config.h"
#include "ota.h"

void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());

    OTASetup();
}

void onStationModeConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print(F("WiFi connected to: "));
    Serial.println(WiFi.SSID());
}

void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println(F("WiFi got disconnected."));
}

void WificSetup()
{
    WiFi.onEvent(onStationModeGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onStationModeConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(onStationModeDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.mode(WIFI_STA);
    WiFi.hostname(otahostname);
    WiFi.begin(ssid, password);
}

void WiFicLoop()
{
    OTALoop();
}