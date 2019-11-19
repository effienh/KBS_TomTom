#include <avr/io.h>
#include <Arduino.h>
#include <avr/interrupt.h>

int counter1 = 0;
int counter2 = 0;
int positie = 7;
int sent = 0;

uint8_t bytje = 0b00000000;


ISR(TIMER2_COMPA_vect)
{
  if(~(bytje |~(1 << positie)))
  {
    if(counter1 < 10)
    {
      counter1++;
      PORTD ^= (1<<PORTD3);
    }
    
    if(counter1 == 10)
    {
      sent = 1;
    }
  
    if(sent)
    {
      counter2++;
      PORTD &= ~(1<<PORTD3);
    }
    
    if(counter2 == 10)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;
      sent = 0;
    }
  }else
  {
    if(counter1 < 10)
    {
      counter1++;
      PORTD ^= (1<<PORTD3);
    }
    
    if(counter1 == 10)
    {
      sent = 1;
    }
    
    if(sent)
    {
      counter2++;
      PORTD &= ~(1<<PORTD3);
    }
    
    if(counter2 == 30)
    {
      counter2 = 0;
      counter1 = 0;
      positie--;
      sent = 0;
    }
  }
  
  if(positie == -1)
  {
    positie = 7;
  }
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