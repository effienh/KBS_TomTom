#include <avr/io.h>
#include <avr/interrupt.h>
#include <Nunchuk.h>
#include <Wire.h>

int counter1 = 0;
int counter2 = 0;
int positie = 7;
int sent = 0;

int bytje = 0b00000000;

int counter = 0;
int prev_counter = 0;
int current_counter = 0;
int difference_counters = 0;
int count_interrupts = 0;
int bit_positie = 7;
int falling_edge = 1;

uint8_t data_correct = 0;

int middle, up, down, left, right, buttonc;

void timer2_setup();
void PCINT1_setup();
void setup_pin3();

void timer1_setup();
void IR_led_setup();
void setupWireNunchuk();
void nunchuk_send_byte();

ISR(TIMER1_COMPA_vect)
{
  if (~(bytje | ~(1 << positie)))
  {
    if (counter1 < 200)
    {
      counter1++;
      PORTD ^= (1 << PORTD6);
    }

    if (counter1 >= 200)
    {
      sent = 1;
    }

    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD6);
    }

    if (counter2 >= 200)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;
      sent = 0;
    }
  } else
  {
    if (counter1 < 200)
    {
      counter1++;
      PORTD ^= (1 << PORTD6);
    }

    if (counter1 >= 200)
    {
      sent = 1;
    }

    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD6);
    }

    if (counter2 >= 600)
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
    //Serial.println(difference_counters);
    counter = 0;
  }

  if (difference_counters >= 280 && difference_counters <= 480 && (count_interrupts % 2 == 0)) //is 1
  {
    bit_positie--;
    count_interrupts = 0;
    data_correct++;
  } else if (difference_counters <= 200 && difference_counters >= 100 && (count_interrupts % 2 == 0)) //is 0
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
  setupWireNunchuk();
  IR_led_setup();
  setup_pin3();
  timer1_setup();
  timer2_setup();
  PCINT1_setup();
  sei();

  while (1)
  {
    if (nunchuk_read())
    {
      nunchuk_send_byte();
    }
    check_buttonZ();

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

void IR_led_setup()
{
  DDRD |= (1 << DDD6);
}

void timer1_setup()
{
  // TIMER 1 for interrupt frequency 37735.84905660377 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 37735.84905660377 Hz increments
  OCR1A = 284; // = 16000000 / (8 * 37735.84905660377) - 1 (must be <256)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS22, CS21 and CS20 bits for 8 prescaler
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
  if (nunchuk_buttonC())
  {
    bytje = 0b00000000;
  }
}
