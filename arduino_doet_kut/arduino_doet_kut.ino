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
#define BOMB "/bom.bmp"
#define GROUND "/blok.bmp"
#define KIST "/kist.bmp"
#define SPREAD_MID "/bom_spread_mid.bmp"
#define SPREAD_RIGHT "/bom_spread_right.bmp"
#define SPREAD_UP "/bom_spread_up.bmp"
#define SPREAD_LEFT "/bom_spread_left.bmp"
#define SPREAD_DOWN "/bom_spread_down.bmp"

uint8_t BORDER_UP = 192;
uint8_t BORDER_DOWN = 32;
uint8_t BORDER_LEFT = 96;
uint16_t BORDER_RIGHT = 256;

int y_waarde = 208;
int x_waarde = 96;

int y_bom;
int x_bom;

int first = 0;

int up;
int rechts;
int links;
int onder;

const int rows = 13;
const int columns = 13;
const int width = 160;
const int height =  160;

uint8_t grid[rows][columns]
{
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1},
  {1, 0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 0, 1},
  {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
  {1, 2, 1, 2, 1, 0, 1, 0, 1, 2, 1, 2, 1},
  {1, 2, 2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 1},
  {1, 2, 1, 2, 1, 0, 1, 0, 1, 2, 1, 2, 1},
  {1, 2, 2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 1},
  {1, 2, 1, 2, 1, 0, 1, 0, 1, 2, 1, 2, 1},
  {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
  {1, 0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 0, 1},
  {1, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

int bomb_counter = 0;
int bomb_set;
int explode = 0;
int ground_once;
int refresh_once;

int spread_counter;
int spread_set;
int boom;

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
void go_up();
void go_right();
void go_left();
void go_down();
void place_bomb();
void explode_bomb();
void remove_block();

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

  if (bomb_counter >= 10)
  {
    explode = 1;
    bomb_set = 0;
    spread_set = 1;
  }

  if (spread_set)
  {
    spread_counter++;
  }

  if (spread_counter >= 8)
  {
    boom = 1;
    spread_counter = 0;
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

      if (nunchuk_buttonC())
      {
        if (bomb_set == 0 && spread_set == 0)
        {
          if (first)
          {
            place_bomb();
            ground_once = 1;
          }
        }
        first = 1;
      }

      if (bomb_set == 0 && ground_once == 1)
      {
        bomb_counter = 0;
        ImageReturnCode remove_bomb = reader.drawBMP(GROUND, tft, y_bom, x_bom);
        ground_once = 0;
      }

      if (explode)
      {
        explode_bomb();
        explode = 0;
      }

      if (boom)
      {
        remove_block();
        boom = 0;
        spread_set = 0;
      }

      ImageReturnCode set = reader.drawBMP("/shadow.bmp", tft, 208, 96);
    }
  }
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

  for (int row = 1; row <= 11; row++)
  {
    for (int column = 1; column <= 11; column++)
    {
      if (grid[row][column] == 2)
      {
        ImageReturnCode chest = reader.drawBMP(KIST, tft, 208 - (row * pixel), 80 + column * pixel);
      }
    }
  }
}

void go_up()
{
  y_waarde = y_waarde + pixel;
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
  if (refresh_once == 0)
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde - pixel, x_waarde);
  } else
  {
    refresh_once = 0;
  }
}

void go_right()
{
  x_waarde = x_waarde + pixel;
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
  if (refresh_once == 0)
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde, x_waarde - pixel);
  } else
  {
    refresh_once = 0;
  }
}

void go_left()
{
  x_waarde = x_waarde - pixel;
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
  if (refresh_once == 0)
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND , tft, y_waarde, x_waarde + pixel);
  } else
  {
    refresh_once = 0;
  }
}

void go_down()
{

  y_waarde = y_waarde - pixel;
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde, x_waarde);
  if (refresh_once == 0)
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde + pixel, x_waarde);
  } else
  {
    refresh_once = 0;
  }
}

void place_bomb()
{
  y_bom = y_waarde;

  x_bom = x_waarde;
  refresh_once = 1;

  grid[(208 - y_bom) / pixel][(x_bom - 80) / pixel] = 1;
  ImageReturnCode place_bomb = reader.drawBMP(BOMB, tft, y_bom, x_bom);
  bomb_set = 1;
}

void explode_bomb()
{
  grid[(208 - y_bom) / pixel][(x_bom - 80) / pixel] = 0;

  if (grid[((208 - y_bom) / pixel)][(x_bom - 80) / pixel] != 1)//spread bomb
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_MID, tft, y_bom, x_bom);
  }
  if (grid[((208 - y_bom) / pixel) - 1][(x_bom - 80) / pixel] != 1)//spread up
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_UP, tft, y_bom + pixel, x_bom);
  }
  if (grid[((208 - y_bom) / pixel) + 1][(x_bom - 80) / pixel] != 1)//spread down
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_DOWN, tft, y_bom - pixel, x_bom);
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) + 1] != 1)//spread right
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_RIGHT, tft, y_bom, x_bom + pixel);
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) - 1] != 1)//spread left
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_LEFT, tft, y_bom, x_bom - pixel);
  }
}


void remove_block()
{
  if (grid[((208 - y_bom) / pixel)][(x_bom - 80) / pixel] != 1)//spread bomb
  {
    grid[((208 - y_bom) / pixel)][(x_bom - 80) / pixel] = 0;
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom, x_bom);
  }
  if (grid[((208 - y_bom) / pixel) - 1][(x_bom - 80) / pixel] != 1)//spread up
  {
    grid[((208 - y_bom) / pixel) - 1][(x_bom - 80) / pixel] = 0;
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom + pixel, x_bom);
  }
  if (grid[((208 - y_bom) / pixel) + 1][(x_bom - 80) / pixel] != 1)//spread down
  {
    grid[((208 - y_bom) / pixel) + 1][(x_bom - 80) / pixel] = 0;
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom - pixel, x_bom);
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) + 1] != 1)//spread right
  {
    grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) + 1] = 0;
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom, x_bom + pixel);
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) - 1] != 1)//spread left
  {
    grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) - 1] = 0;
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom, x_bom - pixel);
  }
}
