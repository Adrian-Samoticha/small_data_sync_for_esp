#include <Arduino.h>

#include "SmallDataSync.h"

void setup() {
  Serial.begin(115200);
  Serial.printf("hello world (compiled at %s %s)\n", __DATE__, __TIME__);
}

void loop() {}