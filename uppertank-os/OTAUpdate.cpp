#include "OTAUpdate.h"


String firmwareBinUrl;
String versionCheckUrl;
String CURRENT_VERSION;

const char *ipServer = "52.66.6.129";

WiFiClient client;

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

// ---------- EEPROM ----------
void saveVersionToEEPROM(const String &version) {
  int addr = VERSION_EEPROM_ADDR;
  Serial.println("Saving version on EEPROM: " + version);
  byte len = version.length();
  EEPROM.write(addr++, len);
  for (byte i = 0; i < len; i++) {
    EEPROM.write(addr++, version[i]);
  }
  // Commit changes to EEPROM
  EEPROM.commit();
}

void loadVersionFromEEPROM() {
  int addr = VERSION_EEPROM_ADDR;
  byte len = EEPROM.read(addr++);
  String value = "";
  for (byte i = 0; i < len; i++) {
    value += (char)EEPROM.read(addr++);
  }
  if (value.length() == 0 || value.length() > 20) {
    CURRENT_VERSION = "0.0.0";  // default if nothing valid is stored
  } else {
    CURRENT_VERSION = value;
  }
  Serial.println("Loaded version from EEPROM: " + CURRENT_VERSION);
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


void checkForOTAandUpdate() {
  EEPROM.begin(EEPROM_SIZE);

  // ---------- Prepare URLs ----------
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