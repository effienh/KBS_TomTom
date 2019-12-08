#include <avr/io.h>
#include <avr/interrupt.h>

int counter = 0;
int prev_counter = 0;
int current_counter = 0;
int difference_counters = 0;
int count_interrupts = 0;
int bit_positie = 7;
int falling_edge = 1;
int data_correct = 1;

uint8_t data_byte;
int for_counter = 0;

void timer2_setup();
void PCINT1_setup();
void setup_pin3();

ISR(TIMER2_COMPA_vect)
{
  counter++;
}


ISR(INT1_vect)
{
  for (int i = 0; i < 7; i++)
  {
    if (~(data_byte |~ (1 << i)))
    {
      for_counter++;
    }
  }
  if(for_counter == 7)
  {
     bit_positie = 7;
     data_byte = 0;
     for_counter = 0;
     Serial.println("");
  }

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
    data_byte |= (1 << bit_positie);
    bit_positie--;
    Serial.print(1);
    count_interrupts = 0;
  } else if (difference_counters <= 200 && difference_counters >= 100 && (count_interrupts % 2 == 0)) //is 0
  {
    data_byte &= ~(1 << bit_positie);
    bit_positie--;
    Serial.print(0);
    count_interrupts = 0;
  }
}



int main()
{
  init();
  Serial.begin(38400);
  setup_pin3();
  timer2_setup();
  PCINT1_setup();
  sei();

  while (1)
  {

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
