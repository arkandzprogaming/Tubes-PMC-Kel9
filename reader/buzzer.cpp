#include <cstdint>
#include <Arduino.h>
#include "buzzer.h"
#include "indicator.h"

const uint8_t buzzerPin = 5;

void buzzer_1() {
  digitalWrite(buzzerPin, HIGH);
  delay(150);
  digitalWrite(buzzerPin, LOW);
  delay(100);
  digitalWrite(buzzerPin, HIGH);
  delay(150);
  digitalWrite(buzzerPin, LOW);

  greenIndic();
}

void buzzer_0() {
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(buzzerPin, LOW);

  yellowIndic();
}

void buzzKey() {
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);

}
