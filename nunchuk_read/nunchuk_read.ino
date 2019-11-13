#include <Wire.h>
#include "Nunchuk.h"
#include <avr/io.h>

void setupWire() {
    init();
    Serial.begin(9600);
    Wire.begin();
    // nunchuk_init_power(); // A1 and A2 is power supply
    nunchuk_init();
}

int main()
{
setupWire();
  while(1)
  {
      if (nunchuk_read()) {
          // Work with nunchuk_data
          Serial.println(nunchuk_joystickX_raw());
          Serial.println(nunchuk_joystickY_raw());
      }
      delay(10);
  }
}
