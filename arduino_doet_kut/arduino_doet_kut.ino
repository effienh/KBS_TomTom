#include <avr/io.h>
#include <avr/interrupt.h>
#include <Nunchuk.h>
#include <Wire.h>

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <Adafruit_EPD.h>

#include <MinimumSerial.h>
#include <SdFat.h>

#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#define USE_SD_CARD

int BORDER_UP = 176;
int BORDER_DOWN = 48;
int BORDER_LEFT = 102;
int BORDER_RIGHT = 246;

int y_waarde = 208;
int x_waarde = 116;

int deadzone = 0;
int prev_state = 0;

int y_bom;
int x_bom;

int up;
int rechts;
int links;
int onder;

int pixel = 16;

// TFT display and SD card share the hardware SPI interface, using
// 'select' pins for each to identify the active device on the bus.

#define SD_CS   4 // SD card select pin
#define TFT_CS 10 // TFT select pin
#define TFT_DC  9 // TFT display/command pin
  
SdFat SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void timer1_setup();
void setupWire();

ISR(TIMER1_COMPA_vect)
{
   if(nunchuk_joystickY_raw() > 200) //up
   {
    up = 1;
   } 
   if(nunchuk_joystickX_raw() > 200) //right
   {
    rechts = 1;
   }
   if(nunchuk_joystickX_raw() < 50) //left
   {
    links = 1;
   }
   if(nunchuk_joystickY_raw() < 50) //down
   {
    onder = 1;
   }
}

void setup(void)
{
  Serial.begin(9600);
  ImageReturnCode stat; // Status from image-reading functions
  
  tft.begin();          // Initialize screen
  SD.begin(SD_CS, SD_SCK_MHZ(25));
  setupWire();
  timer1_setup();
  
  stat = reader.drawBMP("/map.bmp", tft, 0, 0);
}

void loop() 
{
  if (nunchuk_read()) 
      {
          if(up)
          {
            y_waarde = y_waarde + pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde - pixel, x_waarde);
            
            if(y_waarde > BORDER_UP)
            {
              y_waarde = BORDER_UP;
            }
            up = 0;
          }
          
          if(rechts)
          {
            x_waarde = x_waarde + pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde, x_waarde - pixel);

            if(x_waarde > BORDER_RIGHT)
            {
              x_waarde = BORDER_RIGHT;
            }
            rechts = 0;
          }
          
          if(links)
          {
            x_waarde = x_waarde - pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            //tft.fillRect(y_waarde, (x_waarde + 5), 5, 5, 0xFFFF);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde, x_waarde + pixel);

            if(x_waarde < BORDER_LEFT)
            {
              x_waarde = BORDER_LEFT;
            }
            links = 0;
          }

          if(onder)
          {
            y_waarde = y_waarde - pixel;
            tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
            ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde + pixel, x_waarde);

            if(y_waarde < BORDER_DOWN)
            {
              y_waarde = BORDER_DOWN;
            }
            onder = 0;
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
      }
}

void timer1_setup()
{
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 120.00480019200768 Hz increments
  OCR1A = 12000; // = 16000000 / (8 * 120.00480019200768) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 8 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts
}

void setupWire()
{
  Wire.begin();
  init();
  nunchuk_init();
}
