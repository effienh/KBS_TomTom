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

//graphics
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
#define EINDSCHERM_THIERRY "/endscreen-thierry.bmp"
#define EINDSCHERM_BEER "/endscreen-beer.bmp"

#define USE_SD_CARD //which SD card is used
#define SD_CS   4 // SD card select pin
#define TFT_CS 10 // TFT select pin
#define TFT_DC  9 // TFT display/command pin

//PLAYER 1
uint16_t y_waarde_P1 = 192;
uint16_t x_waarde_P1 = 96;

//PLAYER 2
uint16_t y_waarde_P2 = 32;
uint16_t x_waarde_P2 = 256;

//Makes sure the bomb won't explode at the start
uint8_t first_P1 = 0;
uint8_t first_P2 = 0;

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
   4 = spread
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
uint16_t bomb_counter_P1 = 0;
uint8_t bomb_set_P1;
uint8_t explode_P1 = 0;
uint8_t ground_once_P1;
uint8_t refresh_once_P1;

uint16_t bomb_counter_P2 = 0;
uint8_t bomb_set_P2;
uint8_t explode_P2 = 0;
uint8_t ground_once_P2;
uint8_t refresh_once_P2;

//location bomb
uint16_t y_bom_P1;
uint16_t x_bom_P1;

uint16_t y_bom_P2;
uint16_t x_bom_P2;

//bomb spread
uint16_t spread_counter_P1;
uint8_t spread_set_P1;
uint8_t boom_P1;

uint16_t spread_counter_P2;
uint8_t spread_set_P2;
uint8_t boom_P2;

//endscreen
uint8_t game_over_P1 = 0;
uint8_t life_P1 = 2;
uint8_t damage_done_P1 = 0;

uint8_t game_over_P2 = 0;
uint8_t life_P2 = 2;
uint8_t damage_done_P2 = 0;

//1 block in the grid
const uint8_t pixel = 16;

//used for reading SD-card
SdFat SD;
Adafruit_ImageReader reader(SD);

//setup for LCD screen
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//setups prototypes
void timer0_setup();
void setupWire();
void lcd_setup();
void map_setup();
void endscreen();

void timer2_setup();
void PCINT1_setup();
void setup_pin3();

void timer1_setup();
void IR_led_setup();

//control functions prototypes
void move_P1();
void go_up_P1();
void go_right_P1();
void go_left_P1();
void go_down_P1();
void place_bomb_P1();
void explode_bomb_P1();
void damage_player_P1();
void draw_P1();
void remove_block_P1();

void move_P2();
void go_up_P2();
void go_right_P2();
void go_left_P2();
void go_down_P2();
void place_bomb_P2();
void explode_bomb_P2();
void damage_player_P2();
void draw_P2();
void remove_block_P2();

//IR variables
int counter1 = 0;
int counter2 = 0;
int positie = 7;
int sent = 0;

uint8_t bytje = 0b00000000; //byte which is send

//IR checks
int counter = 0;
int prev_counter = 0;
int current_counter = 0;
int difference_counters = 0;
int count_interrupts = 0;
int bit_positie = 7;
int falling_edge = 1;

uint8_t data_correct = 0;

int midden, boven, onder, links, rechts, buttonc;

uint8_t count_interrupt0 = 0;

ISR(TIMER0_COMPA_vect)
{
  count_interrupt0++;

  if (count_interrupt0 >= 240)
  {
    count_interrupt0 = 0;
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
  }

  if (bomb_set_P1) //is placed when bom is placed
  {
    bomb_counter_P1++; //leaves the bomb for a while
  }

  if (bomb_counter_P1 >= 2000)
  {
    explode_P1 = 1;
    bomb_set_P1 = 0; //bomb removes
    spread_set_P1 = 1;
  }

  if (spread_set_P1)
  {
    spread_counter_P1++; //leaves the spread for a while
    damage_player_P1(); //damages players
    damage_player_P2();
  }

  if (spread_counter_P1 >= 1500)
  {
    boom_P1 = 1;
    spread_counter_P1 = 0;
  }

  if (bomb_set_P2) //is placed when bom is placed
  {
    bomb_counter_P2++; //leaves the bomb for a while
  }

  if (bomb_counter_P2 >= 2000)
  {
    explode_P2 = 1;
    bomb_set_P2 = 0; //bomb removes
    spread_set_P2 = 1;
  }

  if (spread_set_P2)
  {
    spread_counter_P2++; //leaves the spread for a while
    damage_player_P2(); //damages players
    damage_player_P1();
  }

  if (spread_counter_P2 >= 1500)
  {
    boom_P2 = 1;
    spread_counter_P2 = 0;
  }
}

ISR(TIMER1_COMPA_vect)
{
  counter++;
}

ISR(TIMER2_COMPA_vect)
{
  if (~(bytje | ~(1 << positie)))
  {
    if (counter1 < 100)
    {
      counter1++;
      PORTD ^= (1 << PORTD6);//toggles IR led
    }
    if (counter1 >= 100)
    {
      sent = 1;
    }
    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD6);//sents 1 bit
    }

    if (counter2 >= 100)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;//moves to next bit
      sent = 0;
    }
  } else
  {
    if (counter1 < 100)
    {
      counter1++;
      PORTD ^= (1 << PORTD6);//toggles IR led
    }
    if (counter1 >= 100)
    {
      sent = 1;
    }

    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD6);//sends 1 bit
    }

    if (counter2 >= 300)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;//moves to next bit
      sent = 0;
    }
  }

  if (positie == -1)
  {
    positie = 7;//starts over if position reaches the end
  }
}

ISR(INT1_vect)
{
  EICRA ^= (1 << ISC10);
  count_interrupts++;//checks if bit is 1 or 0
  if (falling_edge)
  {
    prev_counter = counter;
    falling_edge = 0;
  } else
  {
    current_counter = counter;
    falling_edge = 1;
    counter = 0;
    difference_counters = current_counter - prev_counter;
    counter = 0;
  }

  if (difference_counters >= 380 && difference_counters <= 600 && (count_interrupts % 2 == 0)) //bit is 1
  {
    bit_positie--;
    count_interrupts = 0;
    data_correct++;//checks what bit is send
  } else if (difference_counters <= 300 && difference_counters >= 100 && (count_interrupts % 2 == 0)) //bit is 0
  {
    bit_positie--;
    count_interrupts = 0;
  }
  if (bit_positie < 0)
  {
    bit_positie = 7;
    if (data_correct == 5)//0b00000101
    {
      midden++;
    } else if (data_correct == 1)//0b00000001
    {
      boven++;
    } else if (data_correct == 2)//0b00000010
    {
      onder++;
    } else if (data_correct == 3)//0b00000011
    {
      links++;
    } else if (data_correct == 4)//0b00000100
    {
      rechts++;
    } else if (data_correct == 0)//0b00000000
    {
      buttonc++;
    }
    data_correct = 0;
  }
}


int main(void)
{
  init();
  lcd_setup();
  map_setup();
  setupWire();

  //send and receive
  IR_led_setup();
  setup_pin3();
  timer0_setup();
  timer1_setup();
  timer2_setup();
  PCINT1_setup();
  sei();

  while (1)
  {
    if (nunchuk_read()&& (game_over_P1 == 0 && game_over_P2 == 0))//checks if a player has died
    {
      move_P1(); //move functions for PLAYER1
      move_P2(); //move functions for PLAYER2

      ImageReturnCode set = reader.drawBMP(SHADOW , tft, 208, 96);
    }



    if (game_over_P1 || game_over_P2)
    {
      endscreen();//draws endscreen
    }
  }
}

void setup_pin3()
{
  DDRD &= ~(1 << DDD3);
}

void IR_led_setup()
{
  DDRD |= (1 << DDD6);
}

void timer1_setup()
{
  // TIMER 1 for interrupt frequency 56000 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 56000 Hz increments
  OCR1A = 284; // = 16000000 / (8 * 56000) - 1 (must be <256)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 bits for prescaler
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}


void timer2_setup()
{
  // TIMER 2 for interrupt frequency 37735.84905660377 Hz:
  cli(); // stop interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  // set compare match register for 37735.84905660377 Hz increments
  OCR2A = 52; // = 16000000 / (8 * 37735.84905660377) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS22, CS21 and CS20 bits for 8 prescaler
  TCCR2B |= (0 << CS22) | (1 << CS21) | (0 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}

void PCINT1_setup()
{
  EIMSK |= (1 << INT1);
  EICRA |= (1 << ISC11);
  EICRA |= (1 << ISC10);
}

void timer0_setup()
{
  // TIMER 0 for interrupt frequency 62.00396825396825 Hz:
  cli(); // stop interrupts
  TCCR0A = 0; // set entire TCCR0A register to 0
  TCCR0B = 0; // same for TCCR0B
  TCNT0  = 0; // initialize counter value to 0
  // set compare match register for 1000 Hz increments
  OCR0A = 249; // = 16000000 / (64 * 1000) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS02, CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (0 << CS02) | (1 << CS01) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei(); // allow interrupts
}

void setupWire()
{
  Wire.begin(0x52);
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
  y_waarde_P1 = y_waarde_P1 + pixel;//moves up
  draw_P1();

  if (refresh_once_P1 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P1 - pixel, x_waarde_P1); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P1 = 0;
  }
}

void go_right_P1()
{
  x_waarde_P1 = x_waarde_P1 + pixel;//moves right

  draw_P1();

  if (refresh_once_P1 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P1, x_waarde_P1  - pixel); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P1 = 0;
  }
}

void go_left_P1()
{
  x_waarde_P1 = x_waarde_P1 - pixel;//moves left

  draw_P1();

  if (refresh_once_P1 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P1, x_waarde_P1  + pixel); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P1 = 0;
  }
}

void go_down_P1()
{
  y_waarde_P1 = y_waarde_P1 - pixel;//moves down
  draw_P1();

  if (refresh_once_P1 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P1 + pixel, x_waarde_P1); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P1 = 0;
  }
}

void go_up_P2()
{
  y_waarde_P2 = y_waarde_P2 + pixel;//moves up
  draw_P2();

  if (refresh_once_P2 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P2 - pixel, x_waarde_P2); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P2 = 0;
  }
}

void go_right_P2()
{
  x_waarde_P2 = x_waarde_P2 + pixel;//moves right
  draw_P2();
  if (refresh_once_P2 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P2, x_waarde_P2 - pixel); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P2 = 0;
  }
}

void go_left_P2()
{
  x_waarde_P2 = x_waarde_P2 - pixel;//moves left
  draw_P2();

  if (refresh_once_P2 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P2, x_waarde_P2 + pixel); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P2 = 0;
  }
}

void go_down_P2()
{
  y_waarde_P2 = y_waarde_P2 - pixel;//moves down
  draw_P2();

  if (refresh_once_P2 == 0) //is set after a bomb is placed, makes sure the bomb won't be ereased
  {
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_waarde_P2 + pixel, x_waarde_P2); //makes sure PLAYER1 won't be printed twice
  } else
  {
    refresh_once_P2 = 0;
  }
}

void draw_P1()
{
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, y_waarde_P1, x_waarde_P1); //draws PLAYER1 on y and x coordinates
}

void draw_P2()
{
  ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, y_waarde_P2, x_waarde_P2); //draws PLAYER2 on y and x coordinates
}

void place_bomb_P1()
{
  y_bom_P1 = y_waarde_P1; //bomb is placed ON the player
  x_bom_P1 = x_waarde_P1;
  refresh_once_P1 = 1;
  grid[(208 - y_bom_P1) / pixel][(x_bom_P1 - 80) / pixel] = 3; //places bomb on the map
  ImageReturnCode place_bomb = reader.drawBMP(BOMB, tft, y_bom_P1, x_bom_P1); //draws the bomb
  bomb_set_P1 = 1; //triggers the counter for the bomb
}

void place_bomb_P2()
{
  y_bom_P2 = y_waarde_P2; //bomb is placed ON the player
  x_bom_P2 = x_waarde_P2;
  refresh_once_P2 = 1;
  grid[(208 - y_bom_P2) / pixel][(x_bom_P2 - 80) / pixel] = 3; //places bomb on the map
  ImageReturnCode place_bomb = reader.drawBMP(BOMB, tft, y_bom_P2, x_bom_P2); //draws the bomb
  bomb_set_P2 = 1; //triggers the counter for the bomb
}

void explode_bomb_P1()
{
  grid[(208 - y_bom_P1) / pixel][(x_bom_P1 - 80) / pixel] = 4;//set spread (after bomb)
  if (grid[((208 - y_bom_P1) / pixel)][(x_bom_P1 - 80) / pixel] != 1)//spread bomb middle
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_MID, tft, y_bom_P1, x_bom_P1); //draw spread on bomb coordinates
  }
  if (grid[((208 - y_bom_P1) / pixel) - 1][(x_bom_P1 - 80) / pixel] != 1)//spread up
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_UP, tft, y_bom_P1 + pixel, x_bom_P1); //draw spread on bomb coordinates, y + 16
  }
  if (grid[((208 - y_bom_P1) / pixel) + 1][(x_bom_P1 - 80) / pixel] != 1)//spread down
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_DOWN, tft, y_bom_P1 - pixel, x_bom_P1); //draw spread on bomb coordinates, y - 16
  }
  if (grid[((208 - y_bom_P1) / pixel)][((x_bom_P1 - 80) / pixel) + 1] != 1)//spread right
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_RIGHT, tft, y_bom_P1, x_bom_P1 + pixel); //draw spread on bomb coordinates, x + 16
  }
  if (grid[((208 - y_bom_P1) / pixel)][((x_bom_P1 - 80) / pixel) - 1] != 1)//spread left
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_LEFT, tft, y_bom_P1, x_bom_P1 - pixel); //draw spread on bomb coordinates, x - 16
  }
}

void explode_bomb_P2()
{
  grid[(208 - y_bom_P2) / pixel][(x_bom_P2 - 80) / pixel] = 4;//set spread (after bomb)
  if (grid[((208 - y_bom_P2) / pixel)][(x_bom_P2 - 80) / pixel] != 1)//spread bomb middle
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_MID, tft, y_bom_P2, x_bom_P2); //draw spread on bomb coordinates
  }
  if (grid[((208 - y_bom_P2) / pixel) - 1][(x_bom_P2 - 80) / pixel] != 1)//spread up
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_UP, tft, y_bom_P2 + pixel, x_bom_P2); //draw spread on bomb coordinates, y + 16
  }
  if (grid[((208 - y_bom_P2) / pixel) + 1][(x_bom_P2 - 80) / pixel] != 1)//spread down
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_DOWN, tft, y_bom_P2 - pixel, x_bom_P2); //draw spread on bomb coordinates, y - 16
  }
  if (grid[((208 - y_bom_P2) / pixel)][((x_bom_P2 - 80) / pixel) + 1] != 1)//spread right
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_RIGHT, tft, y_bom_P2, x_bom_P2 + pixel); //draw spread on bomb coordinates, x + 16
  }
  if (grid[((208 - y_bom_P2) / pixel)][((x_bom_P2 - 80) / pixel) - 1] != 1)//spread left
  {
    ImageReturnCode explode = reader.drawBMP(SPREAD_LEFT, tft, y_bom_P2, x_bom_P2 - pixel); //draw spread on bomb coordinates, x - 16
  }
}

void damage_player_P1()
{
  if (grid[((208 - y_waarde_P1) / pixel)][((x_waarde_P1 - 80) / pixel)] == 4) //checks if player is on spread coördinates
  {
    game_over_P1 = 1;//player 2 wins
  }
  if (grid[((208 - y_waarde_P1) / pixel)][((x_waarde_P1 - 80) / pixel) - 1] == 4) //checks if player is on spread coördinates
  {
    game_over_P1 = 1;//player 2 wins
  }
  if (grid[((208 - y_waarde_P1) / pixel)][((x_waarde_P1 - 80) / pixel) + 1] == 4) //checks if player is on spread coördinates
  {
    game_over_P1 = 1;//player 2 wins
  }
  if (grid[((208 - y_waarde_P1) / pixel) - 1][((x_waarde_P1 - 80) / pixel)] == 4) //checks if player is on spread coördinates
  {
    game_over_P1 = 1;//player 2 wins
  }
  if (grid[((208 - y_waarde_P1) / pixel) + 1][((x_waarde_P1 - 80) / pixel)] == 4) //checks if player is on spread coördinates
  {
    game_over_P1 = 1;//player 2 wins
  }
}

void damage_player_P2()
{
  if (grid[((208 - y_waarde_P2) / pixel)][((x_waarde_P2 - 80) / pixel)] == 4) //checks if player is on spread coördinates
  {
    game_over_P2 = 1;//player 1 wins
  }
  if (grid[((208 - y_waarde_P2) / pixel)][((x_waarde_P2 - 80) / pixel) - 1] == 4) //checks if player is on spread coördinates
  {
    game_over_P2 = 1;//player 1 wins
  }
  if (grid[((208 - y_waarde_P2) / pixel)][((x_waarde_P2 - 80) / pixel) + 1] == 4) //checks if player is on spread coördinates
  {
    game_over_P2 = 1;//player 1 wins
  }
  if (grid[((208 - y_waarde_P2) / pixel) - 1][((x_waarde_P2 - 80) / pixel)] == 4) //checks if player is on spread coördinates
  {
    game_over_P2 = 1;//player 1 wins
  }
  if (grid[((208 - y_waarde_P2) / pixel) + 1][((x_waarde_P2 - 80) / pixel)] == 4) //checks if player is on spread coördinates
  {
    game_over_P2 = 1;//player 1 wins
  }
}

void endscreen()
{
  if (game_over_P1)//checks if player 1 died
  {
    ImageReturnCode stat3 = reader.drawBMP(EINDSCHERM_BEER, tft, 0, 0); //end-screen
  } else if (game_over_P2)//checks if player 2 died
  {
    ImageReturnCode stat4 = reader.drawBMP(EINDSCHERM_THIERRY, tft, 0, 0); //end-screen
  }
}


void remove_block_P1()
{
  if (grid[((208 - y_bom_P1) / pixel)][(x_bom_P1 - 80) / pixel] != 1) //spread bomb middle
  {
    grid[((208 - y_bom_P1) / pixel)][(x_bom_P1 - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P1, x_bom_P1); //draws ground block on the removed chest
    damage_done_P1 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P1) / pixel) - 1][(x_bom_P1 - 80) / pixel] != 1)//spread up
  {
    grid[((208 - y_bom_P1) / pixel) - 1][(x_bom_P1 - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P1 + pixel, x_bom_P1); //draws ground block on the removed chest
    damage_done_P1 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P1) / pixel) + 1][(x_bom_P1 - 80) / pixel] != 1)//spread down
  {
    grid[((208 - y_bom_P1) / pixel) + 1][(x_bom_P1 - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P1 - pixel, x_bom_P1); //draws ground block on the removed chest
    damage_done_P1 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P1) / pixel)][((x_bom_P1 - 80) / pixel) + 1] != 1)//spread right
  {
    grid[((208 - y_bom_P1) / pixel)][((x_bom_P1 - 80) / pixel) + 1] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P1, x_bom_P1 + pixel); //draws ground block on the removed chest
    damage_done_P1 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P1) / pixel)][((x_bom_P1 - 80) / pixel) - 1] != 1)//spread left
  {
    grid[((208 - y_bom_P1) / pixel)][((x_bom_P1 - 80) / pixel) - 1] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P1, x_bom_P1 - pixel); //draws ground block on the removed chest
    damage_done_P1 = 0; //makes sure the next bomb will do damage
  }

  grid[(208 - y_bom_P1) / pixel][(x_bom_P1 - 80) / pixel] = 0; //remove bomb from the map
}

void remove_block_P2()
{
  if (grid[((208 - y_bom_P2) / pixel)][(x_bom_P2 - 80) / pixel] != 1) //spread bomb middle
  {
    grid[((208 - y_bom_P2) / pixel)][(x_bom_P2 - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P2, x_bom_P2); //draws ground block on the removed chest
    damage_done_P1 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P2) / pixel) - 1][(x_bom_P2 - 80) / pixel] != 1)//spread up
  {
    grid[((208 - y_bom_P2) / pixel) - 1][(x_bom_P2 - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P2 + pixel, x_bom_P2); //draws ground block on the removed chest
    damage_done_P2 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P2) / pixel) + 1][(x_bom_P2 - 80) / pixel] != 1)//spread down
  {
    grid[((208 - y_bom_P2) / pixel) + 1][(x_bom_P2 - 80) / pixel] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P2 - pixel, x_bom_P2); //draws ground block on the removed chest
    damage_done_P2 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P2) / pixel)][((x_bom_P2 - 80) / pixel) + 1] != 1)//spread right
  {
    grid[((208 - y_bom_P2) / pixel)][((x_bom_P2 - 80) / pixel) + 1] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P2, x_bom_P2 + pixel); //draws ground block on the removed chest
    damage_done_P2 = 0; //makes sure the next bomb will do damage
  }
  if (grid[((208 - y_bom_P2) / pixel)][((x_bom_P2 - 80) / pixel) - 1] != 1)//spread left
  {
    grid[((208 - y_bom_P2) / pixel)][((x_bom_P2 - 80) / pixel) - 1] = 0; //removes chest if it's placed underneath bomb spread
    ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, y_bom_P2, x_bom_P2 - pixel); //draws ground block on the removed chest
    damage_done_P2 = 0; //makes sure the next bomb will do damage
  }

  grid[(208 - y_bom_P2) / pixel][(x_bom_P2 - 80) / pixel] = 0; //remove bomb from the map
}

void move_P1()
{
  if ((nunchuk_joystickY_raw() < 135 && nunchuk_joystickY_raw() > 120) && (nunchuk_joystickX_raw() < 135 && nunchuk_joystickX_raw() > 120))
  {
    bytje = 0b00011111;
  } else if (boven_P1) //checks if the nunchuk moves up (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel) - 1][(x_waarde_P1 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      go_up_P1();
      bytje = 0b00000001;
    }
    boven_P1 = 0;
  }

  else if (rechts_P1) //checks if the nunchuk moves right (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel)][((x_waarde_P1 - 80) / pixel) + 1]) //player can't move over borders, walls, bombs or chests
    {
      go_right_P1();
      bytje = 0b00001111;
    }
    rechts_P1 = 0;
  }

  else if (links_P1) //checks if the nunchuk moves left (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel)][((x_waarde_P1 - 80) / pixel) - 1]) //player can't move over borders, walls, bombs or chests
    {
      go_left_P1();
      bytje = 0b00000111;
    }
    links_P1 = 0;
  }

  else if (onder_P1) //checks if the nunchuk moves down (in ISR)
  {
    if (!grid[((208 - y_waarde_P1) / pixel) + 1][(x_waarde_P1 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      go_down_P1();
      bytje = 0b00000011;
    }
    onder_P1 = 0;
  }
  if (nunchuk_buttonC()) //checks if button C is pressed
  {
    bytje = 0b00000000;
    if (bomb_set_P1 == 0 && spread_set_P1 == 0) //makes sure a player can only place one bomb at a time
    {
      if (first_P1) //doesn't place a bomb at the start of the game
      {
        place_bomb_P1();
        ground_once_P1 = 1; //flag to reset the ground once after a bomb exploded
      }
    }
    first_P1 = 1;
  }

  if (bomb_set_P1 == 0 && ground_once_P1 == 1)
  {
    bomb_counter_P1 = 0;
    ImageReturnCode remove_bomb = reader.drawBMP(GROUND, tft, y_bom_P1, x_bom_P1); //removes the bomb by replacing it with a ground block
    ground_once_P1 = 0;
  }

  if (explode_P1) //is set when bomb_counter reaches 10
  {
    explode_bomb_P1(); //explodes the bomb after 1.5 seconds
    explode_P1 = 0;
  }

  if (boom_P1) //is set when spread_counter reaches 8
  {
    remove_block_P1(); //removes the spread from the map
    boom_P1 = 0;
    spread_set_P1 = 0;
  }
}

void move_P2()
{
  if (midden > 2)//reads out middle in ISR (IR)
  {
    midden = 0;
  }
  if (boven > 2)//reads out up in ISR (IR)
  {
    if (!grid[((208 - y_waarde_P2) / pixel) - 1][(x_waarde_P2 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      go_up_P2();
    }
    boven = 0;
  }
  if (onder > 2)//reads out down in ISR (IR)
  {
    if (!grid[((208 - y_waarde_P2) / pixel) + 1][(x_waarde_P2 - 80) / pixel]) //player can't move over borders, walls, bombs or chests
    {
      go_down_P2();
    }
    onder = 0;
  }
  if (links > 2)//reads out left in ISR (IR)
  {
    if (!grid[((208 - y_waarde_P2) / pixel)][((x_waarde_P2 - 80) / pixel) - 1]) //player can't move over borders, walls, bombs or chests
    {
      go_left_P2();
    }
    links = 0;
  }
  if (rechts > 2)//reads out right in ISR (IR)
  {
    if (!grid[((208 - y_waarde_P2) / pixel)][((x_waarde_P2 - 80) / pixel) + 1]) //player can't move over borders, walls, bombs or chests
    {
      go_right_P2();
    }
    rechts = 0;
  }
  if (buttonc > 2)//reads out button C in ISR (IR)
  {
    buttonc = 0;

    if (bomb_set_P2 == 0 && spread_set_P2 == 0) //makes sure a player can only place one bomb at a time
    {
      if (first_P2) //doesn't place a bomb at the start of the game
      {
        place_bomb_P2();
        ground_once_P2 = 1; //flag to reset the ground once after a bomb exploded

      }
    }
    first_P2 = 1;
  }

  if (bomb_set_P2 == 0 && ground_once_P2 == 1) //checks if only one bomb is set at a time
  {
    bomb_counter_P2 = 0;
    ImageReturnCode remove_bomb = reader.drawBMP(GROUND, tft, y_bom_P2, x_bom_P2); //removes the bomb by replacing it with a ground block
    ground_once_P2 = 0;
  }

  if (explode_P2) //is set when bomb_counter reaches 10
  {
    explode_bomb_P2(); //explodes the bomb after 1.5 seconds
    explode_P2 = 0;
  }

  if (boom_P2) //is set when spread_counter reaches 8
  {
    remove_block_P2(); //removes the spread from the map
    boom_P2 = 0;
    spread_set_P2 = 0;
  }
}
