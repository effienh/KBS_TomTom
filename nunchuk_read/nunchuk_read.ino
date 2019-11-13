#include <Adafruit_GFX.h>
#include <Wire.h>
#include "Nunchuk.h"
#include <avr/io.h>
#include <Adafruit_ILI9341.h>

int BORDER_UP = 235;
int BORDER_DOWN = 0;
int BORDER_LEFT = 0;
int BORDER_RIGHT = 315;

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

int y_waarde = 128;
int x_waarde = 128;

int y_bom;
int x_bom;

void setupWire() {
    init();
    Serial.begin(9600);
    Wire.begin();
    // nunchuk_init_power(); // A1 and A2 is power supply
    nunchuk_init();
    tft.begin();
}

int main()
{
setupWire();
tft.fillScreen(0xFFFF);
  while(1)
  {
      if (nunchuk_read()) 
      {
          if(nunchuk_joystickY_raw() > 200) //up
          {
            y_waarde = y_waarde + 5;
            tft.fillRect(y_waarde, x_waarde, 5, 5, 0x0000);
            tft.fillRect((y_waarde - 5), x_waarde, 5, 5, 0xFFFF);

            if(y_waarde > BORDER_UP)
            {
              y_waarde = BORDER_UP;
            }
          }

          if(nunchuk_joystickX_raw() > 200) //right
          {
            x_waarde = x_waarde + 5;
            tft.fillRect(y_waarde, x_waarde, 5, 5, 0x0000);
            tft.fillRect(y_waarde, (x_waarde - 5), 5, 5, 0xFFFF);

            if(x_waarde > BORDER_RIGHT)
            {
              x_waarde = BORDER_RIGHT;
            }
          }

          if(nunchuk_joystickX_raw() < 50) //left
          {
            x_waarde = x_waarde - 5;
            tft.fillRect(y_waarde, x_waarde, 5, 5, 0x0000);
            tft.fillRect(y_waarde, (x_waarde + 5), 5, 5, 0xFFFF);

            if(x_waarde < BORDER_LEFT)
            {
              x_waarde = BORDER_LEFT;
            }
          }

          if(nunchuk_joystickY_raw() < 50) //down
          {
            y_waarde = y_waarde - 5;
            tft.fillRect(y_waarde, x_waarde, 5, 5, 0x0000);
            tft.fillRect((y_waarde + 5), x_waarde, 5, 5, 0xFFFF);

            if(y_waarde < BORDER_DOWN)
            {
              y_waarde = BORDER_DOWN;
            }
          }

          if(nunchuk_buttonZ())
          {
            y_bom = y_waarde;
            x_bom = x_waarde;
            tft.fillRect(y_bom+5, x_bom, 5, 5, 0xF800);
          }
      delay(5);
      }
   }
}
