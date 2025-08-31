#include "OTAUpdate.h"


String firmwareBinUrl;
String versionCheckUrl;
String CURRENT_VERSION;

const char *ipServer = "52.66.6.129";

WiFiClient client;

// ---------- EEPROM ----------
void writeStringToEEPROM(int addr, const String &data) {
  byte len = data.length();
  EEPROM.write(addr++, len);
  for (byte i = 0; i < len; i++) {
    EEPROM.write(addr++, data[i]);
  }
}

void saveVersionToEEPROM(const String &version) {
  Serial.println("Saving version on EEPROM: " + version);
  writeStringToEEPROM(VERSION_EEPROM_ADDR, version);
  EEPROM.commit();
}

String readStringFromEEPROM(int addr) {
  byte len = EEPROM.read(addr++);
  String value = "";
  for (byte i = 0; i < len; i++) {
    value += (char)EEPROM.read(addr++);
  }
  return value;
}

void loadVersionFromEEPROM() {
  CURRENT_VERSION = readStringFromEEPROM(VERSION_EEPROM_ADDR);
  if (CURRENT_VERSION.length() == 0 || CURRENT_VERSION.length() > 20) {
    CURRENT_VERSION = "0.0.0";  // default if not stored yet
  }
}

// ---------- Version Comparison ----------
bool isVersionNewer(String current, String latest) {
  int currentMajor, currentMinor, currentPatch;
  int latestMajor, latestMinor, latestPatch;

  sscanf(current.c_str(), "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);
  sscanf(latest.c_str(), "%d.%d.%d", &latestMajor, &latestMinor, &latestPatch);

  if (latestMajor > currentMajor) return true;
  if (latestMajor == currentMajor && latestMinor > currentMinor) return true;
  if (latestMajor == currentMajor && latestMinor == currentMinor && latestPatch > currentPatch) return true;

  return false;
}

// ---------- OTA Check ----------
void checkForOTAUpdate() {
  EEPROM.begin(EEPROM_SIZE);
  String macAddress = WiFi.macAddress();
  firmwareBinUrl = "http://" + String(ipServer) + ":8080/api/v1/device/firmware?macAddress=" + macAddress;
  versionCheckUrl = "http://" + String(ipServer) + ":8080/api/v1/device/firmware/version?macAddress=" + macAddress;
  Serial.println("Mac address: " + macAddress);
  loadVersionFromEEPROM();
  
  WiFiClient client;
  HTTPClient http;

  Serial.println("Checking for latest firmware version...");
  http.begin(client, versionCheckUrl);

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    int dataIndex = payload.indexOf("\"data\":\"");
    if (dataIndex != -1) {
      int start = dataIndex + 8;
      int end = payload.indexOf("\"", start);
      String latestVersion = payload.substring(start, end);
      bool canUpdate = isVersionNewer(CURRENT_VERSION, latestVersion);
      Serial.println("Current: " + CURRENT_VERSION + ", Latest: " + latestVersion);
      if (canUpdate) {
        Serial.println("Firmware update available. Starting OTA...");
        ESPhttpUpdate.rebootOnUpdate(false);
        t_httpUpdate_return result = ESPhttpUpdate.update(client, firmwareBinUrl);

        switch (result) {
          case HTTP_UPDATE_FAILED:
            Serial.printf("Update failed: %s\n", ESPhttpUpdate.getLastErrorString().c_str());
            break;
          case HTTP_UPDATE_NO_UPDATES:
            Serial.println("No update available.");
            break;
          case HTTP_UPDATE_OK:
            saveVersionToEEPROM(latestVersion);
            Serial.println("Update successful. Rebooting...");
            delay(500);
            ESP.restart();
            break;
        }
      } else {
        Serial.println("Firmware is already up to date.");
      }
    } else {
      Serial.println("Version parsing failed.");
    }
  } else {
    Serial.printf("Version check failed. HTTP code: %d\n", httpCode);
  }

  http.end();
}