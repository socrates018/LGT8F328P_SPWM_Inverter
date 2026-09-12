#include <Arduino.h>
#include "spwm.hpp"

void setup() {
  // 20 kHz carrier and 50 Hz sine output on Timer1 OC1A/OC1B.
  spwm::begin(20000, 50);

  analogReference(DEFAULT);  // Use AVCC as the ADC reference.
  analogReadResolution(10);  // Return ADC values from 0 to 1023.
}

void loop() {
  int amplitude = map(analogRead(A0), 0, 1023, 0, 100);
  spwm::setAmplitude(amplitude);
}