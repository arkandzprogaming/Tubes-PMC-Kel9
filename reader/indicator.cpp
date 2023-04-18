#include <cstdint>
#include <Arduino.h>
#include "indicator.h"

const uint8_t rPin = 2, yPin = 4, gPin = 16;

void yellowIndic() {
  digitalWrite(yPin, HIGH);
  delay(1000);
  digitalWrite(yPin, LOW);

}

void greenIndic() {
  digitalWrite(gPin, HIGH);
  delay(300);
  digitalWrite(gPin, LOW);
  delay(300);
  digitalWrite(gPin, HIGH);
  delay(300);
  digitalWrite(gPin, LOW);
}