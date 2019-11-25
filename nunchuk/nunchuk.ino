#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <avr/io.h>
#include <Nunchuk.h>
#include <Wire.h>

#define USE_SD_CARD

int BORDER_UP = 203;
int BORDER_DOWN = 33;
int BORDER_LEFT = 102;
int BORDER_RIGHT = 271;

// TFT display and SD card share the hardware SPI interface, using
// 'select' pins for each to identify the active device on the bus.

#define SD_CS   4 // SD card select pin
#define TFT_CS 10 // TFT select pin
#define TFT_DC  9 // TFT display/command pin
  
SdFat SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


int y_waarde = 128;
int x_waarde = 128;

int deadzone = 0;
int prev_state = 0;

int y_bom;
int x_bom;

int pixel = 16;

void setupWire()
{
  Wire.begin();
  init();
  nunchuk_init();
}

void setup(void)
{
  
  tft.begin();          // Initialize screen
  SD.begin(SD_CS, SD_SCK_MHZ(25));
  setupWire();
  
}

void loop() 
{
  
  if (nunchuk_read()) 
      {     
          
          if(nunchuk_joystickY_raw() > 200) //up
          {
            y_waarde = y_waarde + pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            //tft.fillRect((y_waarde - 5), x_waarde, 5, 5, 0xFFFF);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde - pixel, x_waarde);
            
            //prev_state = 1;
            
            if(y_waarde > BORDER_UP)
            {
              y_waarde = BORDER_UP;
            }
          }

          if(nunchuk_joystickX_raw() > 200) //right
          {
            x_waarde = x_waarde + pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            //tft.fillRect(y_waarde, (x_waarde - 5), pixel, pixel, 0xFFFF);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde, x_waarde - pixel);

            //prev_state = 1;
            
            if(x_waarde > BORDER_RIGHT)
            {
              x_waarde = BORDER_RIGHT;
            }
          }

          if(nunchuk_joystickX_raw() < 50) //left
          {
            x_waarde = x_waarde - pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            //tft.fillRect(y_waarde, (x_waarde + 5), 5, 5, 0xFFFF);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde, x_waarde + pixel);

            //prev_state = 1;

            if(x_waarde < BORDER_LEFT)
            {
              x_waarde = BORDER_LEFT;
            }
          }

          if(nunchuk_joystickY_raw() < 50) //down
          {
            y_waarde = y_waarde - pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            //tft.fillRect((y_waarde + 5), x_waarde, 5, 5, 0xFFFF);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde + pixel, x_waarde);

            //prev_state = 1;

            if(y_waarde < BORDER_DOWN)
            {
              y_waarde = BORDER_DOWN;
            }
          }

          /*if(nunchuk_buttonZ())
          {
            y_bom = y_waarde;
            x_bom = x_waarde;
            tft.fillRect(y_bom + pixel, x_bom, pixel, pixel, 0xF800);
          }
          if(nunchuk_buttonC())
          {
            y_bom = y_waarde;
            x_bom = x_waarde;            
            tft.fillRect(y_bom + 5, x_bom, 20, 20, 0xFFFF);
          }*/
        //}
      _delay_ms(120);
      }
}
