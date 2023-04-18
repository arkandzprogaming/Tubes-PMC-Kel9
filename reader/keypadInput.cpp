#include <Arduino.h>
#include <cstdint>
#include "keypadInput.h"
#include "buzzer.h"

//define the cymbols on the buttons of the keypads
char hexaKeys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[4] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[4] = {26, 25, 33, 32}; //connect to the column pinouts of the keypad