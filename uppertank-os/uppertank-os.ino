#include "OTAUpdate.h"


void setup() {
  Serial.begin(115200);
  connectToWifi();
  checkForOTAandUpdate();
}

void loop() {
  // Your sensor code here
  Serial.println("\nLooping!");
  delay(2000);
}