#include <avr/io.h>
#include <avr/interrupt.h>
#include <Wire.h>
#include "Nunchuk.h"
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

#define PLAYER1 "/thierry.bmp"
#define PLAYER2 "/beer.bmp"
#define BOMB "/bom.bmp"
#define GROUND "/blok.bmp"
#define SHADOW "/shadow.bmp"
#define KIST "/kist.bmp"
#define SPREAD_MID "/bom_spread_mid.bmp"
#define SPREAD_RIGHT "/bom_spread_right.bmp"
#define SPREAD_UP "/bom_spread_up.bmp"
#define SPREAD_LEFT "/bom_spread_left.bmp"
#define SPREAD_DOWN "/bom_spread_down.bmp"
#define EINDSCHERM "/eindscherm_thierry.bmp"

#define USE_SD_CARD //which SD card is used
#define SD_CS   4 // SD card select pin
#define TFT_CS 10 // TFT select pin
#define TFT_DC  9 // TFT display/command pin

//PLAYER 1
uint16_t y_waarde_P1 = 208;
uint16_t x_waarde_P1 = 96;

//PLAYER 2
uint16_t y_waarde_P2 = 32;
uint16_t x_waarde_P2 = 256;

//Makes sure the bomb won't explode at the start
uint8_t first = 0;

//controls movement in  (Dutch because of compilation errors)
uint8_t boven_P1;
uint8_t rechts_P1;
uint8_t links_P1;
uint8_t onder_P1;

uint8_t boven_P2;
uint8_t rechts_P2;
uint8_t links_P2;
uint8_t onder_P2;


/*
   Grid map:
   0 = ground
   1 = borders and walls (non-collisiable)
   2 = chests (collisiable after explosion)
   3 = bomb
*/
uint8_t grid[13][13]
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

//bomb controls
uint8_t bomb_counter_P1 = 0;
uint8_t bomb_set;
uint8_t explode = 0;
uint8_t ground_once;
uint8_t refresh_once_P1;


uint8_t bomb_counter_P2 = 0;
uint8_t refresh_once_P2;

//location bomb
uint16_t y_bom;
uint16_t x_bom;

//bomb spread
uint8_t spread_counter;
uint8_t spread_set;
uint8_t boom;

//lifes
uint8_t life_player = 2;
uint8_t game_over = 0;
uint8_t damage_done = 0;

//1 block in the grid
const uint8_t pixel = 16;

//used for reading SD-card
SdFat SD;
Adafruit_ImageReader reader(SD);

//setup for LCD screen
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//setups prototypes
void timer1_setup();
void setupWire();
void lcd_setup();
void map_setup();

//control functions prototypes
void move_P1();
void go_up_P1();
void go_right_P1();
void go_left_P1();
void go_down_P1();
void place_bomb();
void explode_bomb();
void remove_block();
void damage_player();
void draw_P1();
void draw_P2();

void move_P2();
void go_up_P2();
void go_right_P2();
void go_left_P2();
void go_down_P2();


ISR(TIMER1_COMPA_vect)
{
  if (nunchuk_joystickY_raw() > 200) //up
  {
    boven_P1 = 1;
  }
  if (nunchuk_joystickX_raw() > 200) //right
  {
    rechts_P1 = 1;
  }
  if (nunchuk_joystickX_raw() < 50) //left
  {
    links_P1 = 1;
  }
  if (nunchuk_joystickY_raw() < 50) //down
  {
    onder_P1 = 1;
  }

  if (bomb_set) //is placed when bom is placed
  {
    bomb_counter_P1++; //leaves the bomb for a while
  }

  if (bomb_counter_P1 >= 10)
  {
    explode = 1;
    bomb_set = 0; //bomb removes
    spread_set = 1;
  }

    if (bomb_counter_P2 >= 10)
  {
    explode = 1;
    bomb_set = 0; //bomb removes
    spread_set = 1;
  }

  if (spread_set)
  {
    spread_counter++; //leaves the spread for a while
    damage_player();
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
    if (nunchuk_read() && game_over == 0)
    {
      move_P1(); //move functions for PLAYER1
      move_P2(); //move functions for PLAYER2

      ImageReturnCode set = reader.drawBMP(SHADOW , tft, 208, 96);
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
  tft.begin(); //Initialize screen
  SD.begin(SD_CS, SD_SCK_MHZ(25)); //starts reading out the SD
}

void map_setup()
{
  ImageReturnCode stat; //Status from image-reading functions
  stat = reader.drawBMP("/map.bmp", tft, 0, 0); //draws the map

  for (int row = 1; row <= 11; row++) //goes through all rows off the grid
  {
    for (int column = 1; column <= 11; column++) //goes through all columns off the grid
    {
      if (grid[row][column] == 2) //checks if a chest needs to be placed
      {
        ImageReturnCode chest = reader.drawBMP(KIST, tft, 208 - (row * pixel), 80 + column * pixel); //displayes chest on the LCD
      }
    }
  }
}

void go_up_P1()
{
  y_waarde_P1 = y_waarde_P1 + pixel;

  draw_P1();
}

void go_right_P1()
{
  x_waarde_P1 = x_waarde_P1 + pixel;

  draw_P1();
}

void go_left_P1()
{
  x_waarde_P1 = x_waarde_P1 - pixel;

  draw_P1();
}

void go_down_P1()
{
  y_waarde_P1 = y_waarde_P1 - pixel;

  draw_P1();
}

void go_up_P2()
{
  y_waarde_P2 = y_waarde_P2 + pixel;

  draw_P2();
}

void go_right_P2()
{
  x_waarde_P2 = x_waarde_P2 + pixel;

  draw_P2();
}

void go_left_P2()
{
  x_waarde_P2 = x_waarde_P2 - pixel;

  draw_P2();
}

void go_down_P2()
{
  y_waarde_P2 = y_waarde_P2 - pixel;

  draw_P2();
}

void draw_P1()
{
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, y_waarde_P1, x_waarde_P1); //draws PLAYER1 on y and x coordinates

  if (refresh_once_P1 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P1 + pixel, x_waarde_P1); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P1 = 0;
  }

}

void draw_P2()
{
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde_P2, x_waarde_P2); //draws PLAYER2 on y and x coordinates

  if (refresh_once_P2 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P2 + pixel, x_waarde_P2); //makes sure PLAYER2 won't be printed twice
  } else
  {
    refresh_once_P2 = 0;
  }

}

void place_bomb()
{
  y_bom = y_waarde_P1; //bomb is placed ON the player
  x_bom = x_waarde_P1;
  refresh_once_P1 = 1;
  grid[(208 - y_bom) / pixel][(x_bom - 80) / pixel] = 3; //places bomb on the map
  ImageReturnCode place_bomb = reader.drawBMP(BOMB, tft, y_bom, x_bom); //draws the bomb
  bomb_set = 1; //triggers the counter for the bomb
}

void explode_bomb()
{
  grid[(208 - y_bom) / pixel][(x_bom - 80) / pixel] = 0; //remove bomb from the map

  if (grid[((208 - y_bom) / pixel)][(x_bom - 80) / pixel] != 1)//spread bomb middle
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_MID, tft, y_bom, x_bom); //draw spread on bomb coordinates
  }
  if (grid[((208 - y_bom) / pixel) - 1][(x_bom - 80) / pixel] != 1)//spread up
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_UP, tft, y_bom + pixel, x_bom); //draw spread on bomb coordinates, y + 16
  }
  if (grid[((208 - y_bom) / pixel) + 1][(x_bom - 80) / pixel] != 1)//spread down
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_DOWN, tft, y_bom - pixel, x_bom); //draw spread on bomb coordinates, y - 16
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) + 1] != 1)//spread right
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_RIGHT, tft, y_bom, x_bom + pixel); //draw spread on bomb coordinates, x + 16
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) - 1] != 1)//spread left
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_LEFT, tft, y_bom, x_bom - pixel); //draw spread on bomb coordinates, x - 16
  }
}

void damage_player()
{
  if ((x_bom == x_waarde_P1 || x_bom == x_waarde_P1 + pixel || x_bom == x_waarde_P1 - pixel) && y_bom == y_waarde_P1 && damage_done == 0)
  { //checks if PLAYER1  walks through bomb spread
    life_player--;
    damage_done = 1; //makes sure the bomb doesn't do damage twice
  }
  if (( y_bom == y_waarde_P1 + pixel || y_bom == y_waarde_P1 - pixel) && x_bom == x_waarde_P1 && damage_done == 0)
  {
    life_player--;
    damage_done = 1; //makes sure the bomb doesn't do damage twice
  }

  if (life_player == 0)
  {
    remove_block(); //removes bomb spread
    game_over = 1; //stops the game in the while loop
    ImageReturnCode stat2 = reader.drawBMP(EINDSCHERM, tft, 0, 0); //end-screen
  }
}

void remove_block()
{
  if (grid[((208 - y_bom) / pixel)][(x_bom - 80) / pixel] != 1) //spread bomb middle
  {
    grid[((208 - y_bom) / pixel)][(x_bom - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom, x_bom); //draws ground block on the removed chest

    damage_done = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom) / pixel) - 1][(x_bom - 80) / pixel] != 1)//spread up
  {
    grid[((208 - y_bom) / pixel) - 1][(x_bom - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom + pixel, x_bom); //draws ground block on the removed chest

    damage_done = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom) / pixel) + 1][(x_bom - 80) / pixel] != 1)//spread down
  {
    grid[((208 - y_bom) / pixel) + 1][(x_bom - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom - pixel, x_bom); //draws ground block on the removed chest

    damage_done = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) + 1] != 1)//spread right
  {
    grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) + 1] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom, x_bom + pixel); //draws ground block on the removed chest

    damage_done = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) - 1] != 1)//spread left
  {
    grid[((208 - y_bom) / pixel)][((x_bom - 80) / pixel) - 1] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom, x_bom - pixel); //draws ground block on the removed chest

    damage_done = 0; //makes sure the next bomb will do damage
  }
}

void move_P1()
{
  if (boven_P1) //checks if the nunchuk moves up (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel) - 1][(x_waarde_P1 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      go_up_P1();
    }
    boven_P1 = 0;
  }

  if (rechts_P1) //checks if the nunchuk moves right (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel)][((x_waarde_P1 - 80) / pixel) + 1]) //player can't move over borders, walls, bombs or chests
    {
      go_right_P1();
    }
    rechts_P1 = 0;
  }

  if (links_P1) //checks if the nunchuk moves left (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel)][((x_waarde_P1 - 80) / pixel) - 1]) //player can't move over borders, walls, bombs or chests
    {
      go_left_P1();
    }
    links_P1 = 0;
  }

  if (onder_P1) //checks if the nunchuk moves down (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel) + 1][(x_waarde_P1 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      go_down_P1();
    }
    onder_P1 = 0;
  }

  if (nunchuk_buttonC()) //checks if button C is pressed
  {
    if (bomb_set == 0 && spread_set == 0) //makes sure a player can only place one bomb at a time
    {
      if (first) //doesn't place a bomb at the start of the game
      {
        place_bomb();
        ground_once = 1; //flag to reset the ground once after a bomb exploded
      }
    }
    first = 1;
  }

  if (bomb_set == 0 && ground_once == 1)
  {
    bomb_counter_P1 = 0;
    ImageReturnCode remove_bomb = reader.drawBMP(GROUND, tft, y_bom, x_bom); //removes the bomb by replacing it with a ground block
    ground_once = 0;
  }

  if (explode) //is set when bomb_counter reaches 10
  {
    explode_bomb(); //explodes the bomb after 1.5 seconds
    explode = 0;
  }

  if (boom) //is set when spread_counter reaches 8
  {
    remove_block(); //removes the spread from the map
    boom = 0;
    spread_set = 0;
  }
}

void move_P2()
{
  //check_buttonZ();
  
  if (up > 3)
  {
    if (!grid[((208 - y_waarde_P2) / pixel) - 1][(x_waarde_P2 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      Serial.println("UP");
      go_up_P2();
    }
    up = 0;
  }
  if (down > 3)
  {
    if (!grid[((208 - y_waarde_P2) / pixel) + 1][(x_waarde_P2 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      Serial.println("DOWN");
      go_down_P2();
    }
    down = 0;
  }
  if (left > 3)
  {
    if (!grid[((208 - y_waarde_P2) / pixel)][((x_waarde_P2 - 80) / pixel) - 1]) //player can't move over borders, walls, bombs or chests
    {
      Serial.println("LEFT");
      go_left_P2();
    }
    left = 0;
  }
  if (right > 3)
  {
    if (!grid[((208 - y_waarde_P2) / pixel)][((x_waarde_P2 - 80) / pixel) + 1]) //player can't move over borders, walls, bombs or chests
    {
      Serial.println("RIGHT");
      go_right_P2();
    }
    right = 0;
  }
  if (buttonc > 3)
  {
    Serial.println("BUTTONC");
    buttonc = 0;
  }
}
