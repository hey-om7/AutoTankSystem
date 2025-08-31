#include "DistanceMeasure.h"


void initializeSensor() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

int getDistanceCM() {
  long duration, distance;

  // Clear the trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10µs pulse to trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse
  duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout = 30ms (~5m max)

  // Convert time to distance (cm)
  distance = duration * 0.0343 / 2;

  if (duration == 0) {
    Serial.println("Out of range");
    return -1;
  } else {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    return distance;
  }
}

// Function to measure distance in mm
int getDistanceMM() {
  long duration;

  // Clear trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10µs trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo time (timeout ~30ms → ~5m)
  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;  // Out of range
  }

  // Convert to distance: speed of sound ~343 m/s = 0.343 mm/µs
  // Distance = (duration * 0.343) / 2  → in mm
  int distanceMM = (duration * 343) / 2000;

  return distanceMM;
}

int getPerfectDistanceMM() {
  long readings[NUM_SAMPLES];
  int validCount = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    long duration;

    // Trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Echo time
    duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration > 0) {
      long distanceMM = (duration * 343) / 2000;
      readings[validCount++] = distanceMM;
    }

    delay(100); // ⬅️ longer pause for stability
  }

  if (validCount == 0) return -1;

  // Sort readings
  for (int i = 0; i < validCount - 1; i++) {
    for (int j = i + 1; j < validCount; j++) {
      if (readings[i] > readings[j]) {
        long temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }

  // Discard outliers
  int discardCount = validCount * DISCARD_RATIO;
  int start = discardCount;
  int end = validCount - discardCount;

  if (start >= end) {
    return readings[validCount / 2]; // fallback to median
  }

  // Average remaining
  long sum = 0;
  int count = 0;
  for (int i = start; i < end; i++) {
    sum += readings[i];
    count++;
  }

  int filtered = sum / count;

  // ✅ Apply exponential smoothing across function calls
  static int smoothed = filtered;
  smoothed = (0.7 * smoothed) + (0.3 * filtered);

  return smoothed;
}