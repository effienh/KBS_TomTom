#include <avr/io.h>
#include <avr/interrupt.h>

int counter1 = 0;
int counter2 = 0;
int positie = 7;
int sent = 0;

uint8_t bytje = 0b0;

void timer2_setup();
void IR_led_setup();

ISR(TIMER2_COMPA_vect)
{
  if(~(bytje |~(1 << positie)))
  {
    if(counter1 < 1000)
    {
      counter1++;
      PORTD ^= (1<<PORTD3);
    }
    
    if(counter1 >= 1000)
    {
      sent = 1;
    }
  
    if(sent)
    {
      counter2++;
      PORTD &= ~(1<<PORTD3);
    }
    
    if(counter2 >= 1000)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;
      sent = 0;
    }
  }else
  {
    if(counter1 < 1000)
    {
      counter1++;
      PORTD ^= (1<<PORTD3);
    }
    
    if(counter1 >= 1000)
    {
      sent = 1;
    }
  
    if(sent)
    {
      counter2++;
      PORTD &= ~(1<<PORTD3);
    }
    
    if(counter2 >= 3000)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;
      sent = 0;
    }
  }
  
  if(positie == -1)
  {
    positie = 7;
  }
}

int main()
{
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
  cli(); // stop interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  // set compare match register for 761904.7619047619 Hz increments
  OCR2A = 35; // = 16000000 / (1 * 37000) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (0<<WGM20)|(1 << WGM21);
  TCCR2B |= (0<<WGM22);
  // Set CS22, CS21 and CS20 bits for 1 prescaler
  TCCR2B |= (0 << CS22) | (1 << CS21) | (0 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}
