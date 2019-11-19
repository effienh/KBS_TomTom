#include <avr/io.h>
#include <Arduino.h>
#include <avr/interrupt.h>

int counter = 0;
int prev_counter = 0;
int current_counter = 0;
int difference_counters = 0;
unsigned int state;

int falling_edge = 1;

void timer2_setup();

ISR(TIMER2_COMPA_vect)
{
  counter++;
  //Serial.println(counter);
}

ISR(INT1_vect)
{
  EICRA ^= (1<<ISC10);
  if(falling_edge)
  {
    prev_counter = counter;
    Serial.println(prev_counter);
    falling_edge = 0;
  }else if(falling_edge == 0)
  {
    current_counter = counter;
    falling_edge = 1;
    counter = 0;
    difference_counters = current_counter - prev_counter;
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

  while(1)
  {  
  }
}

void setup_pin3()
{
  DDRD &= ~(1<<DDD2);
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
  TCCR2B |= (1 << WGM21);
  // Set CS22, CS21 and CS20 bits for 8 prescaler
  TCCR2B |= (1 << CS22) | (1 << CS21) | (0 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}

void PCINT1_setup()
{
  EIMSK |= (1<<INT1);
  EICRA |= (1<<ISC11);
  EICRA |= (1<<ISC10);
}
