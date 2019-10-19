/*************************************************************************
Title:    Testing output to a HD44780 based LCD display.
Author:   Peter Fleury  <pfleury@gmx.ch>  http://tinyurl.com/peterfleury
File:     $Id: test_lcd.c,v 1.8 2015/01/31 18:04:08 peter Exp $
Software: AVR-GCC 4.x
Hardware: HD44780 compatible LCD text display
          AVR with external SRAM interface if memory-mapped LCD interface is
used
          any AVR with 7 free I/O pins if 4-bit IO port mode is used
**************************************************************************/
#include "lcd.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

uint8_t buffer[16];
volatile uint8_t write_marker = 0;
volatile uint8_t read_marker = 0;
uint8_t start_counting = 0;
volatile uint8_t character = 1;
/*
** constant definitions
*/
typedef void *(*states)(void);

/*
** function prototypes
*/

static inline void initTimer0(void);
static inline void disable_buzzer(void);
static inline void enable_buzzer(void);
inline uint8_t decode(uint8_t dit_dah);
void *state_idle(void);
void *state_dots(void);
void *state_dashes(void);
void *state_dpause(void);
void *state_cpause(void);

// State pointer
states statefunc = (states)state_idle;

void *state_idle(void) {
  disable_buzzer();
  if (0 == (PIND & (1 << PD3))) {
    enable_buzzer();
    start_counting = 0x80;
    OCR0B = 0x20;
    return state_dots;
  }

  if (start_counting) {
    start_counting--;
  } else if (character != 1) {
    start_counting = 0x40;
    buffer[(write_marker++) & 0x0F] = character;
    character = 1;
  }
  return state_idle;
}

void *state_dots(void) {
  start_counting--;
  if (PIND & (1 << PD3)) {
    start_counting = 0xFF;
    character = character << 1;
    return state_dpause;
  }

  if (start_counting) {
    return state_dots;
  } else {
    OCR0B = 0x10;
    character = (character << 1) + 1;
    start_counting = 0xF0;
    return state_dashes;
  }
}

void *state_dashes(void) {
  start_counting--;
  if (PIND & (1 << PD3)) {
    start_counting = 0xFF;
    return state_dpause;
  }

  if (0 == start_counting) {
    start_counting = 0xF0;
    return state_dpause;
  } else {
    return state_dashes;
  }
}

void *state_dpause(void) {
  start_counting--;
  if (PIND & (1 << PD3)) {
    start_counting = 0xFF;
    return state_idle;
  }

  if (start_counting)
    return state_dpause;
  else {
    start_counting = 0x80;
    buffer[(write_marker++) & 0x0F] = character;
    character = 1;
    return state_cpause;
  }
}

void *state_cpause(void) {
  start_counting--;
  if (start_counting)
    return state_cpause;
  else
    return state_idle;
}

ISR(TIMER0_OVF_vect) // Timer0 ISR
{
  statefunc = (*statefunc)();
}

static inline void disable_buzzer(void) {
  TCCR0A &= ~((1 << COM0B1) | (0 << COM0B0));
}

static inline void enable_buzzer(void) {
  TCCR0A |= ((1 << COM0B1) | (0 << COM0B0));
}

static inline void initTimer0(void) {
  // Timer 0 configuration : Fast PWM
  TCCR0B |= (1 << CS02) | (0 << CS01) | (0 << CS00) // Prescaler = 1024
            | (1 << WGM02);                         // Fast PWM
  TCCR0A |= ((1 << COM0B1) | (0 << COM0B0) | (1 << WGM01) | (1 << WGM00));

  TIMSK |= (1 << TOIE0);

  OCR0A = 0x40; // Counter Top
  OCR0B = 0x20;
}

static inline void initInt1(void) {
  MCUCR |= (1 << ISC11) | (0 << ISC10); // Failling edge
  GIMSK |= (1 << INT1);
}

inline uint8_t decode(uint8_t dit_dah) {
  uint8_t code;
  switch (dit_dah) {
  case 5:
    code = 'A';
    break;
  case 0x18:
    code = 'B';
    break;
  case 0x1A:
    code = 'C';
    break;
  case 0x0C:
    code = 'D';
    break;
  case 0x02:
    code = 'E';
    break;
  case 0x12:
    code = 'F';
    break;
  case 0x0E:
    code = 'G';
    break;
  case 0x10:
    code = 'H';
    break;
  case 0x04:
    code = 'I';
    break;
  case 0x17:
    code = 'J';
    break;
  case 0x0D:
    code = 'K';
    break;
  case 0x14:
    code = 'L';
    break;
  case 0x07:
    code = 'M';
    break;
  case 0x06:
    code = 'N';
    break;
  case 0x0F:
    code = 'O';
    break;
  case 0x16:
    code = 'P';
    break;
  case 0x1D:
    code = 'Q';
    break;
  case 0x0A:
    code = 'R';
    break;
  case 0x08:
    code = 'S';
    break;
  case 0x03:
    code = 'T';
    break;
  case 0x09:
    code = 'U';
    break;
  case 0x11:
    code = 'V';
    break;
  case 0x0B:
    code = 'W';
    break;
  case 0x19:
    code = 'X';
    break;
  case 0x1B:
    code = 'Y';
    break;
  case 0x1C:
    code = 'Z';
    break;
  case 0x2F:
    code = '1';
    break;
  case 0x27:
    code = '2';
    break;
  case 0x23:
    code = '3';
    break;
  case 0x21:
    code = '4';
    break;
  case 0x20:
    code = '5';
    break;
  case 0x30:
    code = '6';
    break;
  case 0x38:
    code = '7';
    break;
  case 0x3C:
    code = '8';
    break;
  case 0x3E:
    code = '9';
    break;
  case 0x3F:
    code = '0';
    break;
  default:
    code = '*';
  } /* Ende switch */
  return code;
}

int main(void) {
  //
  DDRD &= ~(1 << PD3);
  PORTD |= (1 << PD3);

  DDRD |= (1 << PD5);

  /* initialize display, cursor off */
  lcd_init(LCD_DISP_ON);

  statefunc = state_idle;
  initTimer0();

  sei();

  lcd_gotoxy(0, 0);
  for (;;) { /* loop forever */

    /* clear display and home cursor */

    uint8_t code;
    if (write_marker != read_marker) {
      uint8_t dit_dat = buffer[(read_marker++) & 0x0F];

      if ((write_marker & 0x0F) == 0) {
        lcd_clrscr();
      }
      code = decode(dit_dat);
      lcd_putc(code);
    }
//      lcd_putc('4');
  }
}
