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

int counter1 = 0;
int counter2 = 0;
int positie = 7;
int sent = 0;

uint8_t bytje = 0;

void timer2_setup();
void IR_led_setup();
void setupWireNunchuk();
void nunchuk_send_byte();
void check_buttonZ();

void timer1_setup();
void PCINT1_setup();
void setup_pin3();

ISR(TIMER1_COMPA_vect)
{
  counter++;
}

ISR(TIMER2_COMPA_vect)
{
  if (~(bytje | ~(1 << positie)))
  {
    if (counter1 < 100 /*&& !nunchuk_middle*/)
    {
      counter1++;
      PORTD ^= (1 << PORTD6);
    }

    if (counter1 >= 100)
    {
      sent = 1;
    }

    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD6);
    }

    if (counter2 >= 100)
    {
      counter1 = 0;
      counter2 = 0;
      positie--;
      sent = 0;
    }
  } else
  {
    if (counter1 < 100 /*&& !nunchuk_middle*/)
    {
      counter1++;
      PORTD ^= (1 << PORTD6);
    }

    if (counter1 >= 100)
    {
      sent = 1;
    }

    if (sent)
    {
      counter2++;
      PORTD &= ~(1 << PORTD6);
    }

    if (counter2 >= 300)
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

void timer2_setup()
{
  cli(); // stop interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2  = 0; // initialize counter value to 0
  // set compare match register for 761904.7619047619 Hz increments
  OCR2A = 52; // = 16000000 / (1 * 37000) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (0 << WGM20) | (1 << WGM21);
  TCCR2B |= (0 << WGM22);
  // Set CS22, CS21 and CS20 bits for 1 prescaler
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
  if(nunchuk_buttonC())
  {
    bytje = 0b00000000;
  }
}
