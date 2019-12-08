#include <avr/io.h>
#include <avr/interrupt.h>

int counter = 0;
int prev_counter = 0;
int current_counter = 0;
int difference_counters = 0;
int count_interrupts = 0;
int bit_positie = 7;
int falling_edge = 1;

uint8_t data_correct = 0;
int data_byte[9];

int middle, up, down, left, right, buttonc;

void timer2_setup();
void PCINT1_setup();
void setup_pin3();

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
    if(data_correct == 0)
    {
      middle = 1;
    }else if(data_correct == 1)
    {
      up = 1;
    }else if(data_correct == 2)
    {
      down = 1;
    }else if(data_correct == 3)
    {
      left = 1;
    }else if(data_correct == 4)
    {
      right = 1;
    }else if(data_correct == 8)
    {
      buttonc = 1;
    }
    data_correct = 0;
  }
}




int main()
{
  init();
  Serial.begin(9600);
  setup_pin3();
  timer2_setup();
  PCINT1_setup();
  sei();

  while (1)
  {
    if(middle)
    {
      Serial.println("MIDDLE");
      middle = 0;
    }else if(up)
    {
      Serial.println("UP");
      up = 0;
    }else if(down)
    {
      Serial.println("DOWN");
      down = 0;
    }else if(left)
    {
      Serial.println("LEFT");
      left = 0;
    }else if(right)
    {
      Serial.println("RIGHT");
      right = 0;
    }else if(buttonc)
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
