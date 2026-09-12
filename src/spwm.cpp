#include "spwm.hpp"

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>

namespace {

constexpr uint16_t kTableSize = 256;
constexpr uint64_t kPhaseScale = 1ULL << 32;
constexpr double kPi = 3.14159265358979323846;

uint16_t sineTable[kTableSize];
volatile uint32_t phase;
volatile uint32_t phaseStep;
volatile uint16_t pwmTop;
volatile uint16_t currentFrequency;
volatile uint8_t currentAmplitude = 100;
uint16_t currentCarrier;

void selectOutput(bool negativeHalf) {
  // Keep only one Timer1 compare output connected at a time.
  uint8_t mode = _BV(WGM11);
  mode |= negativeHalf ? _BV(COM1B1) : _BV(COM1A1);
  TCCR1A = mode;
}

}  // namespace

namespace spwm {

bool begin(uint16_t carrierHz, uint16_t sineHz) {
  if (carrierHz == 0 || sineHz == 0 || sineHz >= carrierHz / 2) {
    return false;
  }

  const uint32_t top = (F_CPU / carrierHz) - 1UL;
  if (top == 0 || top > 65535UL) {
    return false;
  }

  cli();
  pwmTop = static_cast<uint16_t>(top);
  currentCarrier = carrierHz;
  currentFrequency = sineHz;
  currentAmplitude = 100;
  phase = 0;
  phaseStep = static_cast<uint32_t>((static_cast<uint64_t>(sineHz) * kPhaseScale) / carrierHz);

  for (uint16_t index = 0; index < kTableSize; ++index) {
    const double angle = (static_cast<double>(index) * kPi) / kTableSize;
    sineTable[index] = static_cast<uint16_t>(sin(angle) * pwmTop + 0.5);
  }

  DDRB |= _BV(DDB1) | _BV(DDB2);
  ICR1 = pwmTop;
  OCR1A = 0;
  OCR1B = 0;
  TCCR1A = _BV(WGM11) | _BV(COM1A1);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
  TIMSK1 = _BV(TOIE1);
  sei();
  return true;
}

bool setFrequency(uint16_t sineHz) {
  if (sineHz == 0 || sineHz >= currentCarrier / 2) {
    return false;
  }

  const uint32_t newStep = static_cast<uint32_t>(
      (static_cast<uint64_t>(sineHz) * kPhaseScale) / currentCarrier);
  const uint8_t interruptState = SREG;
  cli();
  phaseStep = newStep;
  currentFrequency = sineHz;
  SREG = interruptState;
  return true;
}

bool setAmplitude(uint8_t percent) {
  if (percent > 100) {
    return false;
  }

  const uint8_t interruptState = SREG;
  cli();
  currentAmplitude = percent;
  SREG = interruptState;
  return true;
}

uint16_t carrierFrequency() {
  return currentCarrier;
}

uint16_t frequency() {
  return currentFrequency;
}

uint8_t amplitude() {
  return currentAmplitude;
}

}  // namespace spwm

ISR(TIMER1_OVF_vect) {
  const uint32_t nextPhase = phase + phaseStep;
  phase = nextPhase;

  const uint8_t tableIndex = static_cast<uint8_t>(nextPhase >> 23);
  const uint32_t scaledDuty =
      static_cast<uint32_t>(sineTable[tableIndex]) * currentAmplitude;
  const uint16_t duty = static_cast<uint16_t>(scaledDuty / 100U);

  OCR1A = duty;
  OCR1B = duty;
  selectOutput((nextPhase & 0x80000000UL) != 0);
}