#include <avr/io.h>
#include <avr/interrupt.h>
#include <Nunchuk.h>
#include <Wire.h>

int counter = 0;
int prev_counter = 0;
int current_counter = 0;
int difference_counters = 0;
int count_interrupts = 0;
int bit_positie = 7;
int falling_edge = 1;

int data_correct = 0;

int middle, up, down, left, right, buttonc;

void timer1_setup();
void PCINT1_setup();
void setup_pin3();

ISR(TIMER1_COMPA_vect)
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
    counter = 0;
    difference_counters = current_counter - prev_counter;
    counter = 0;
    //Serial.println(difference_counters);
  }

  if (difference_counters >= 570 && difference_counters <= 670 && (count_interrupts % 2 == 0)) //is 1
  {
    bit_positie--;
    count_interrupts = 0;
    data_correct++;
  } else if (difference_counters <= 250 && difference_counters >= 150 && (count_interrupts % 2 == 0)) //is 0
  {
    bit_positie--;
    count_interrupts = 0;
  }
  if (bit_positie < 0)
  {
    bit_positie = 7;
    if (data_correct == 5)
    {
      middle++;
    } else if (data_correct == 1)
    {
      up++;
    } else if (data_correct == 2)
    {
      down++;
    } else if (data_correct == 3)
    {
      left++;
    } else if (data_correct == 4)
    {
      right++;
    } else if (data_correct == 0)
    {
      buttonc++;
    }
    data_correct = 0;
  }
}

int main()
{
  init();
  Serial.begin(9600);
  setup_pin3();
  timer1_setup();
  PCINT1_setup();
  sei();

  while (1)
  {
    if (middle > 3)
    {
      Serial.println("MIDDLE");
      middle = 0;
    }
    if (up > 3)
    {
      Serial.println("UP");
      up = 0;
    }
    if (down > 3)
    {
      Serial.println("DOWN");
      down = 0;
    }
    if (left > 3)
    {
      Serial.println("LEFT");
      left = 0;
    }
    if (right > 3)
    {
      Serial.println("RIGHT");
      right = 0;
    }
    if (buttonc > 3)
    {
      Serial.println("BUTTONC");
      buttonc = 0;
    }
  }
}

void setup_pin3()
{
  DDRD &= ~(1 << DDD3);
}

void timer1_setup()
{
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 56140.350877192985 Hz increments
  OCR1A = 284; // = 16000000 / (1 * 56140.350877192985) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}

void PCINT1_setup()
{
  EIMSK |= (1 << INT1);
  EICRA |= (1 << ISC11);
  EICRA |= (1 << ISC10);
}
