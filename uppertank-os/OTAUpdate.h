#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define WIFI_SSID "AMBARKAR"
#define WIFI_PASSWORD "chikki123"
#define EEPROM_SIZE 128
#define VERSION_EEPROM_ADDR 0

extern String CURRENT_VERSION;  

void writeStringToEEPROM(int addr, const String &data);
void saveVersionToEEPROM(const String &version);
String readStringFromEEPROM(int addr);
void loadVersionFromEEPROM();
bool isVersionNewer(String current, String latest);
void checkForOTAUpdate();


#endif