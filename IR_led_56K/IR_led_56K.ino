#include <avr/io.h>
#include <avr/interrupt.h>
#include <Nunchuk.h>
#include <Wire.h>

int counter1 = 0;
int counter2 = 0;
uint8_t positie = 7;
uint8_t sent = 0;

uint8_t bytje = 0;

void timer2_setup();
void IR_led_setup();
void setupWireNunchuk();
void nunchuk_send_byte();

ISR(TIMER2_COMPA_vect)
{
  if (~(bytje | ~(1 << positie)))
  {
    if (counter1 < 1000)
    {
      counter1++;
      PORTD ^= (1 << PORTD3);
    }

    if (counter1 >= 1000)
    {
      sent = 1;
    }

    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD3);
    }

    if (counter2 >= 1000)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;
      sent = 0;
    }
  } else
  {
    if (counter1 < 1000)
    {
      counter1++;
      PORTD ^= (1 << PORTD3);
    }

    if (counter1 >= 1000)
    {
      sent = 1;
    }

    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD3);
    }

    if (counter2 >= 3000)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;
      sent = 0;
    }
  }

  if (positie == -1)
  {
    positie = 7;
  }
}

int main()
{
  setupWireNunchuk();
  timer2_setup();
  IR_led_setup();
  sei();

  while (1)
  {
    if (nunchuk_read())
    {
      nunchuk_send_byte();
    }
    check_buttonZ();
  }
}

void IR_led_setup()
{
  DDRD |= (1 << DDD6);
}

void timer2_setup()
{
  cli(); // stop interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  // set compare match register for 761904.7619047619 Hz increments
  OCR2A = 17; // = 16000000 / (1 * 37000) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (0 << WGM20) | (1 << WGM21);
  TCCR2B |= (0 << WGM22);
  // Set CS22, CS21 and CS20 bits for 1 prescaler
  TCCR2B |= (0 << CS22) | (1 << CS21) | (0 << CS20);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}

void setupWireNunchuk()
{
  init();
  Wire.begin(0x52);
  Serial.begin(9600);
  nunchuk_init();
}

void nunchuk_send_byte()
{
  if (nunchuk_joystickY_raw() > 200) // up
  {
    bytje = 0b00000001;
  } else if (nunchuk_joystickY_raw() < 55) //down
  {
    bytje = 0b00000011;
  } else if ((nunchuk_joystickY_raw() < 135 && nunchuk_joystickY_raw() > 120) && (nunchuk_joystickX_raw() < 135 && nunchuk_joystickX_raw() > 120))
  {
    bytje = 0b00011111;
  } else if (nunchuk_joystickX_raw() < 55) //links
  {
    bytje = 0b00000111;
  } else if (nunchuk_joystickX_raw() > 200)//rechts
  {
    bytje = 0b00001111;
  }
}

void check_buttonZ()
{
  if(nunchuk_buttonC())
  {
    bytje = 0b00000000;
  }
}
