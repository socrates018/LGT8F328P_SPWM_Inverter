#include <Arduino.h>
#include "spwm.hpp"

void setup() {
  // 10 kHz carrier and 50 Hz sine output on Timer1 OC1A/OC1B.
  spwm::begin(20000, 50);
}

void loop() {
  // Example runtime controls:
  // spwm::setFrequency(60);
  // spwm::setAmplitude(80);
}