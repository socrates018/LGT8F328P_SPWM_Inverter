#pragma once

#include <stdint.h>

namespace spwm {

// Timer1 OC1A/OC1B are used on PB1/PB2 (Arduino pins 9/10).
bool begin(uint16_t carrierHz = 10000, uint16_t sineHz = 50);
bool setFrequency(uint16_t sineHz);
bool setAmplitude(uint8_t percent);
uint16_t carrierFrequency();
uint16_t frequency();
uint8_t amplitude();

}  // namespace spwm