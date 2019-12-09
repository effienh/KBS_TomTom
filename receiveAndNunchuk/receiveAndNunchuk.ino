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

#define PLAYER2 "/beer.bmp"
#define PLAYER1 "/thierry.bmp"
#define BOMB "/bom.bmp"
#define GROUND "/blok.bmp"

uint8_t BORDER_UP = 192;
uint8_t BORDER_DOWN = 32;
uint8_t BORDER_LEFT = 96;
uint16_t BORDER_RIGHT = 256;

int P2_y_waarde = 208;
int P2_x_waarde = 96;

int P1_y_waarde = 32;
int P1_x_waarde = 256;

int P2_y_bom;
int P2_x_bom;

int P1_y_bom;
int P1_x_bom;

int up;
int rechts;
int links;
int onder;

const int rows = 12;
const int columns = 12;
const int width = 160;
const int height =  160;

int grid[rows][columns];

int bomb_location[rows][columns] = {0};

int bomb_counter = 0;
int bomb_set;
int explode = 0;
int ground_once;
int refresh_once;

int pixel = 16;

//------RECEIVE DEPARTMENT---------
int counter = 0;
int prev_counter = 0;
int current_counter = 0;
int difference_counters = 0;
int count_interrupts = 0;
int bit_positie = 7;
int falling_edge = 1;

uint8_t data_correct = 0;
int data_byte[9];

int P2_middle, P2_up, P2_down, P2_left, P2_right, P2_buttonc;

//----------RECEIVE DEPARTMENT---------

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
//--RECEIVE---
void timer2_setup();
void PCINT1_setup();
void setup_pin3();
//--RECEIVE---

//control functions
void P1_go_up();
void P1_go_right();
void P1_go_left();
void P1_go_down();
void P1_place_bomb();
void P1_explode_bomb();

void P2_go_up();
void P2_go_right();
void P2_go_left();
void P2_go_down();
void P2_place_bomb();
void P2_explode_bomb();

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
  }
}

ISR(TIMER2_COMPA_vect)
{
  counter++;
}


ISR(INT1_vect)
{
  EICRA ^= (1 << ISC10);
  count_interrupts++;
  if (falling_edge)
  {
    prev_counter = counter;
    falling_edge = 0;
  } else
  {
    current_counter = counter;
    falling_edge = 1;
    difference_counters = current_counter - prev_counter;
    counter = 0;
  }

  if (difference_counters >= 290 && difference_counters <= 390 && (count_interrupts % 2 == 0)) //is 1
  {
    data_byte[bit_positie] = 1;
    bit_positie--;
    count_interrupts = 0;
    data_correct++;
  } else if (difference_counters <= 200 && difference_counters >= 100 && (count_interrupts % 2 == 0)) //is 0
  {
    data_byte[bit_positie] = 0;
    bit_positie--;
    count_interrupts = 0;
  }
  if (bit_positie < 0)
  {
    bit_positie = 7;
    data_byte[8] = {0};
    if (data_correct == 5)
    {
      P2_middle++;
    } else if (data_correct == 1)
    {
      P2_up++;
    } else if (data_correct == 2)
    {
      P2_down++;
    } else if (data_correct == 3)
    {
      P2_left++;
    } else if (data_correct == 4)
    {
      P2_right++;
    } else if (data_correct == 0)
    {
      P2_buttonc++;
    }
    data_correct = 0;
  }
}

int main(void)
{
  init();
  Serial.begin(9600);
  lcd_setup();
  setupWire();
  timer1_setup();
  map_setup();
  setup_pin3();
  timer2_setup();
  PCINT1_setup();
  sei();

  while (1)
  {
    if (P2_middle > 3)
    {
      Serial.println("MIDDLE");
      P2_middle = 0;
    }
    if (P2_up > 3)
    {
      Serial.println("UP");
      P2_go_up();
      P2_up = 0;
    }
    if (P2_down > 3)
    {
      Serial.println("DOWN");
      P2_go_down();
      P2_down = 0;
    }
    if (P2_left > 3)
    {
      Serial.println("LEFT");
      P2_go_left();
      P2_left = 0;
    }
    if (P2_right > 3)
    {
      Serial.println("RIGHT");
      P2_go_right();
      P2_right = 0;
    }
    if (P2_buttonc > 3)
    {
      Serial.println("BUTTONC");
      P2_place_bomb();
      P2_buttonc = 0;
    }
    if (nunchuk_read())
    {
      if (up)
      {
        if (!grid[((208 - P1_y_waarde) / pixel) - 1][(P1_x_waarde - 80) / pixel])
        {
          P1_go_up();
        }
        up = 0;
      }

      if (rechts)
      {
        if (!grid[((208 - P1_y_waarde) / pixel)][((P1_x_waarde - 80) / pixel) + 1])
        {
          P1_go_right();
        }
        rechts = 0;
      }

      if (links)
      {
        if (!grid[((208 - P1_y_waarde) / pixel)][((P1_x_waarde - 80) / pixel) - 1])
        {
          P1_go_left();
        }
        links = 0;
      }

      if (onder)
      {
        if (!grid[((208 - P1_y_waarde) / pixel) + 1][(P1_x_waarde - 80) / pixel])
        {
          P1_go_down();
        }
        onder = 0;
      }

      if (nunchuk_buttonC())
      {
        if (bomb_set == 0)
        {
          P1_place_bomb();
          ground_once = 1;
        }
      }

      if (bomb_set == 0 && ground_once == 1)
      {
        bomb_counter = 0;
        ImageReturnCode remove_bomb = reader.drawBMP(GROUND, tft, P1_y_bom, P1_x_bom);
        ground_once = 0;
      }

      if (explode)
      {
        P1_explode_bomb();
        explode = 0;
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
  Wire.begin(0x52);
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
  //ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, P1_y_waarde - 176, P1_x_waarde + 160);

  for (int row = 2; row <= 10; row = row + 2)
  {
    for (int column = 2; column <= 10; column = column + 2)
    {
      grid[row][column] = 1;
    }
  }
}

void P1_go_up()
{
  if (P1_y_waarde >= BORDER_UP)
  {
    P1_y_waarde = BORDER_UP;
  } else
  {
    P1_y_waarde = P1_y_waarde + pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, P1_y_waarde, P1_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, P1_y_waarde - pixel, P1_x_waarde);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P1_go_right()
{
  if (P1_x_waarde >= BORDER_RIGHT)
  {
    P1_x_waarde = BORDER_RIGHT;
  } else
  {
    P1_x_waarde = P1_x_waarde + pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, P1_y_waarde, P1_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, P1_y_waarde, P1_x_waarde - pixel);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P1_go_left()
{
  if (P1_x_waarde <= BORDER_LEFT)
  {
    P1_x_waarde = BORDER_LEFT;
  } else
  {
    P1_x_waarde = P1_x_waarde - pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, P1_y_waarde, P1_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND , tft, P1_y_waarde, P1_x_waarde + pixel);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P1_go_down()
{
  if (P1_y_waarde <= BORDER_DOWN)
  {
    P1_y_waarde = BORDER_DOWN;
  } else
  {
    P1_y_waarde = P1_y_waarde - pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER1, tft, P1_y_waarde, P1_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, P1_y_waarde + pixel, P1_x_waarde);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P1_place_bomb()
{
  P1_y_bom = P1_y_waarde;
  P1_x_bom = P1_x_waarde;
  refresh_once = 1;

  ImageReturnCode place_bomb = reader.drawBMP(BOMB, tft, P1_y_bom, P1_x_bom);
  bomb_set = 1;
}

void P1_explode_bomb()
{
  if (!grid[((208 - P1_y_bom) / pixel)][(P1_x_bom - 80) / pixel])//spread bomb
  {
    tft.fillRect(P1_y_bom, P1_x_bom, pixel, pixel, 0xF0F0F0);
  }
  if (!grid[((208 - P1_y_bom) / pixel) - 1][(P1_x_bom - 80) / pixel] && P1_y_bom + 1 <= BORDER_UP)//spread up
  {
    tft.fillRect(P1_y_bom + pixel, P1_x_bom, pixel, pixel,  0xF0F0F0);
  }
  if (!grid[((208 - P1_y_bom) / pixel) + 1][(P1_x_bom - 80) / pixel] && P1_y_bom - 1 <= BORDER_DOWN)//spread down
  {
    tft.fillRect(P1_y_bom - pixel, P1_x_bom, pixel, pixel,  0xF0F0F0);
  }
  if (!grid[((208 - P1_y_bom) / pixel)][((P1_x_bom - 80) / pixel) + 1] && P1_x_bom + 1 < BORDER_RIGHT)//spread right
  {
    tft.fillRect(P1_y_bom, P1_x_bom + pixel, pixel, pixel, 0xF0F0F0);
  }
  if (!grid[((208 - P1_y_bom) / pixel)][((P1_x_bom - 80) / pixel) + 1] && P1_x_bom - 1 > BORDER_LEFT)//spread left
  {
    tft.fillRect(P1_y_bom, P1_x_bom - pixel, pixel, pixel,  0xF0F0F0);
  }
}

void P2_go_up()
{
  if (P2_y_waarde >= BORDER_UP)
  {
    P2_y_waarde = BORDER_UP;
  } else
  {
    P2_y_waarde = P2_y_waarde + pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, P2_y_waarde, P2_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, P2_y_waarde - pixel, P2_x_waarde);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P2_go_right()
{
  if (P2_x_waarde >= BORDER_RIGHT)
  {
    P2_x_waarde = BORDER_RIGHT;
  } else
  {
    P2_x_waarde = P2_x_waarde + pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, P2_y_waarde, P2_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, P2_y_waarde, P2_x_waarde - pixel);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P2_go_left()
{
  if (P2_x_waarde <= BORDER_LEFT)
  {
    P2_x_waarde = BORDER_LEFT;
  } else
  {
    P2_x_waarde = P2_x_waarde - pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, P2_y_waarde, P2_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND , tft, P2_y_waarde, P2_x_waarde + pixel);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P2_go_down()
{
  if (P2_y_waarde <= BORDER_DOWN)
  {
    P2_y_waarde = BORDER_DOWN;
  } else
  {
    P2_y_waarde = P2_y_waarde - pixel;
    ImageReturnCode char_refresh = reader.drawBMP(PLAYER2, tft, P2_y_waarde, P2_x_waarde);
    if (refresh_once == 0)
    {
      ImageReturnCode ground_refresh = reader.drawBMP(GROUND, tft, P2_y_waarde + pixel, P2_x_waarde);
    } else
    {
      refresh_once = 0;
    }
  }
}

void P2_place_bomb()
{
  P2_y_bom = P2_y_waarde;
  P2_x_bom = P2_x_waarde;
  refresh_once = 1;

  ImageReturnCode place_bomb = reader.drawBMP(BOMB, tft, P2_y_bom, P2_x_bom);
  bomb_set = 1;
}

void P2_explode_bomb()
{
  if (!grid[((208 - P2_y_bom) / pixel)][(P2_x_bom - 80) / pixel])//spread bomb
  {
    tft.fillRect(P2_y_bom, P2_x_bom, pixel, pixel, 0xF0F0F0);
  }
  if (!grid[((208 - P2_y_bom) / pixel) - 1][(P2_x_bom - 80) / pixel] && P2_y_bom + 1 <= BORDER_UP)//spread up
  {
    tft.fillRect(P2_y_bom + pixel, P2_x_bom, pixel, pixel,  0xF0F0F0);
  }
  if (!grid[((208 - P2_y_bom) / pixel) + 1][(P2_x_bom - 80) / pixel] && P2_y_bom - 1 <= BORDER_DOWN)//spread down
  {
    tft.fillRect(P2_y_bom - pixel, P2_x_bom, pixel, pixel,  0xF0F0F0);
  }
  if (!grid[((208 - P2_y_bom) / pixel)][((P2_x_bom - 80) / pixel) + 1] && P2_x_bom + 1 < BORDER_RIGHT)//spread right
  {
    tft.fillRect(P2_y_bom, P2_x_bom + pixel, pixel, pixel, 0xF0F0F0);
  }
  if (!grid[((208 - P2_y_bom) / pixel)][((P2_x_bom - 80) / pixel) + 1] && P2_x_bom - 1 > BORDER_LEFT)//spread left
  {
    tft.fillRect(P2_y_bom, P2_x_bom - pixel, pixel, pixel,  0xF0F0F0);
  }
}
void setup_pin3()
{
  DDRD &= ~(1 << DDD3);
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
