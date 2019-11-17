#include <avr/io.h>
#include <Arduino.h>
#include <avr/interrupt.h>

ISR(TIMER2_COMPA_vect)
{
  PORTD ^= (1<<PORTD3);
}

void timer2_setup();
void IR_led_setup();

int main()
{
  init();
  timer2_setup();
  IR_led_setup();
  sei();

  while(1)
  {
  }
}

void IR_led_setup()
{
  DDRD |= (1<<DDD3);
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
