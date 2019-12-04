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

#define PLAYER1 "/beer.bmp"
#define PLAYER2 "/thierry.bmp"

int BORDER_UP = 192;
int BORDER_DOWN = 32;
int BORDER_LEFT = 96;
int BORDER_RIGHT = 256;

int y_waarde = 208;
int x_waarde = 96;

int deadzone = 0;
int prev_state = 0;

int y_bom;
int x_bom;

int up;
int rechts;
int links;
int onder;

const int rows = 12;
const int columns = 12;
const int width = 160;
const int height =  160;

int grid[rows][columns];

int bomb_location[rows][columns];

int bomb_counter = 0;

int pixel = 16;

#define SD_CS   4 // SD card select pin
#define TFT_CS 10 // TFT select pin
#define TFT_DC  9 // TFT display/command pin

SdFat SD;
Adafruit_ImageReader reader(SD);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


//setups
void timer1_setup();
void setupWire();
void lcd_setup();
void map_setup();

//control functions
void draw();
void go_up();
void go_right();
void go_left();
void go_down();

ISR(TIMER1_COMPA_vect)
{
  if (nunchuk_joystickY_raw() > 200) //up
  {
    up = 1;
  }
  if (nunchuk_joystickX_raw() > 200) //right
  {
    rechts = 1;
  }
  if (nunchuk_joystickX_raw() < 50) //left
  {
    links = 1;
  }
  if (nunchuk_joystickY_raw() < 50) //down
  {
    onder = 1;
  }

  if (bomb_set)
  {
    bomb_counter++;
  }
}

int main(void)
{
  init();
  lcd_setup();
  setupWire();
  timer1_setup();
  map_setup();

  while (1)
  {
    if (nunchuk_read())
    {
      if (up)
      {
        if (!grid[((208 - y_waarde) / pixel) - 1][(x_waarde - 80) / pixel])
        {
          go_up();
        }
        up = 0;
      }

      if (rechts)
      {
        if (!grid[((208 - y_waarde) / pixel)][((x_waarde - 80) / pixel) + 1])
        {
          go_right();
        }
        rechts = 0;
      }

      if (links)
      {
        if (!grid[((208 - y_waarde) / pixel)][((x_waarde - 80) / pixel) - 1])
        {
          go_left();
        }
        links = 0;
      }

      if (onder)
      {
        if (!grid[((208 - y_waarde) / pixel) + 1][(x_waarde - 80) / pixel])
        {
          go_down();
        }
        onder = 0;
      }

      if(nunchuk_buttonZ())
      {
        place_bomb();
      }
    }
  }
  ImageReturnCode set = reader.drawBMP("/shadow.bmp", tft, 208, 96);
}


void timer1_setup()
{
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 120.00480019200768 Hz increments
  OCR1A = 12000;
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

void lcd_setup()
{
  tft.begin();          // Initialize screen
  SD.begin(SD_CS, SD_SCK_MHZ(25));
}

void map_setup()
{
  ImageReturnCode stat; // Status from image-reading functions
  stat = reader.drawBMP("/map.bmp", tft, 0, 0);
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, y_waarde - 176, x_waarde + 160);

  for (int row = 2; row <= 10; row = row + 2)
  {
    for (int column = 2; column <= 10; column = column + 2)
    {
      grid[row][column] = 1;
    }
  }
}

void go_up()
{
  if (y_waarde >= BORDER_UP)
  {
    y_waarde = BORDER_UP;
  } else
  {
    y_waarde = y_waarde + pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
    ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde - pixel, x_waarde);
  }
}

void go_right()
{
  if (x_waarde >= BORDER_RIGHT)
  {
    x_waarde = BORDER_RIGHT;
  } else
  {
    x_waarde = x_waarde + pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
    ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde, x_waarde - pixel);
  }
}

void go_left()
{
  if (x_waarde <= BORDER_LEFT)
  {
    x_waarde = BORDER_LEFT;
  } else
  {
    x_waarde = x_waarde - pixel;
    tft.fillRect(y_waarde, x_waarde, pixel, pixel, 0x0000);
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
    ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde, x_waarde + pixel);
  }
}

void go_down()
{
  if (y_waarde <= BORDER_DOWN)
  {
    y_waarde = BORDER_DOWN;
  } else
  {
    y_waarde = y_waarde - pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
    ImageReturnCode ground_refresh = reader.drawBMP("/blok.bmp", tft, y_waarde + pixel, x_waarde);
  }
}

void place_bomb()
{
  bomb_set = 1;
  bomb_location[(208 - y_waarde) / pixel)][(x_waarde - 80) / pixel];
  
  if(bomb_counter >= 100)
  {
    bomb_set = 0;
    bomb_counter = 0;
  }
}
