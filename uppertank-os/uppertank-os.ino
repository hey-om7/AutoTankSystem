#include "OTAUpdate.h"
#include "DistanceMeasure.h"


void setup() {
  Serial.begin(115200);
  connectToWifi();
  checkForOTAandUpdate();
  initializeSensor();
}

void loop() {
  // Your sensor code here
  Serial.println("\nMeasure in mm:");
  // Serial.print(getPerfectDistanceMM());
  // Serial.print(getDistanceMM());
  Serial.print(getDistanceCM());
  delay(2000);
}