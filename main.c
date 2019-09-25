/*************************************************************************
Title:    Testing output to a HD44780 based LCD display.
Author:   Peter Fleury  <pfleury@gmx.ch>  http://tinyurl.com/peterfleury
File:     $Id: test_lcd.c,v 1.8 2015/01/31 18:04:08 peter Exp $
Software: AVR-GCC 4.x
Hardware: HD44780 compatible LCD text display
          AVR with external SRAM interface if memory-mapped LCD interface is used
          any AVR with 7 free I/O pins if 4-bit IO port mode is used
**************************************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"


uint8_t buffer[16];
volatile uint8_t write_marker = 0;
volatile uint8_t read_marker = 0;
uint8_t  start_counting = 0;
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
void *state_idle(void);
void *state_dots(void);
void *state_dashes(void);
void *state_dpause(void);
void *state_cpause(void);


// State pointer
states statefunc = (states) state_idle;

void *state_idle(void)
{
    disable_buzzer();
    if(0 == (PIND & (1 << PD3)))
    {
        enable_buzzer();
        start_counting = 0x80;
        OCR0B = 0x20;
        return state_dots;
    }

    if(start_counting){
        start_counting--;
    }
    else
        if(character != 1)
    {
        start_counting = 0x40;
        buffer[(write_marker++)&0x0F] = character;
        character = 1;
    }
    return state_idle;
}


void *state_dots(void)
{
    start_counting--;
    if(PIND & (1 << PD3))
    {
        start_counting = 0xFF;
        character = character << 1;
        return state_dpause;
    }

    if(start_counting)
    {
        return state_dots;
    }
    else
    {
        OCR0B = 0x10;
        character = (character << 1) + 1;
        start_counting = 0xF0;
        return state_dashes;
    }
}

void *state_dashes(void)
{
    start_counting--;
    if(PIND & (1 << PD3))
    {
        start_counting = 0xFF;
        return state_dpause;
    }

    if(0 == start_counting){
        start_counting = 0xF0;
        return state_dpause;
    }
    else
    {
        return state_dashes;
    }
}

void *state_dpause(void)
{
    start_counting--;
    if(PIND & (1 << PD3))
    {
        start_counting = 0xFF;
        return state_idle;
    }

    if(start_counting)
        return state_dpause;
    else
    {
        start_counting = 0x80;
        buffer[(write_marker++)&0x0F] = character;
        character = 1;
        return state_cpause;
    }
}

void *state_cpause(void)
{
    start_counting--;
    if(start_counting)
        return state_cpause;
    else
        return state_idle;
}

ISR(TIMER0_OVF_vect)    // Timer0 ISR
{
    statefunc = (*statefunc)();
}

static inline void disable_buzzer(void){
    TCCR0A &= ~((1 << COM0B1) | (0 << COM0B0));
}

static inline void enable_buzzer(void){
    TCCR0A |= ((1 << COM0B1) | (0 << COM0B0));
}

static inline void initTimer0(void)
{
    // Timer 0 configuration : Fast PWM
    TCCR0B |= (1 << CS02) | (0 << CS01) | (0 << CS00)  // Prescaler = 1024
              | (1 << WGM02); //Fast PWM
    TCCR0A |= ((1 << COM0B1) | (0 << COM0B0) | (1 << WGM01) | (1 << WGM00));

    TIMSK |= (1 << TOIE0);

    OCR0A = 0x40; // Counter Top
    OCR0B = 0x20;
}

static inline void initInt1(void)
{
    MCUCR |= (1 << ISC11) | (0 << ISC10); // Failling edge
    GIMSK |= (1 << INT1);
}


int main(void)
{
//
    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);

    DDRD |= (1 << PD5);

    /* initialize display, cursor off */
    lcd_init(LCD_DISP_ON);

    statefunc = state_idle;
    initTimer0();

    sei();

    lcd_gotoxy(0,0);
    for (;;) {                           /* loop forever */

        /* clear display and home cursor */

        uint8_t b;
        if(write_marker != read_marker){
            uint8_t a = buffer[(read_marker++)&0x0F];

            if((write_marker&0x0F) == 0){
                lcd_clrscr();
            }
            switch(a) {
                case 5:
                    b = 'A';
                    break;
                case 0x18:
                    b = 'B';
                    break;
                case 0x1A:
                    b = 'C';
                    break;
                case 0x0C:
                    b = 'D';
                    break;
                case 0x02:
                    b = 'E';
                    break;
                case 0x12:
                    b = 'F';
                    break;
                case 0x0E:
                    b = 'G';
                    break;
                case 0x10:
                    b = 'H';
                    break;
                case 0x04:
                    b = 'I';
                    break;
                case 0x17:
                    b = 'J';
                    break;
                case 0x0D:
                    b = 'K';
                    break;
                case 0x14:
                    b = 'L';
                    break;
                case 0x07:
                    b = 'M';
                    break;
                case 0x06:
                    b = 'N';
                    break;
                case 0x0F:
                    b = 'O';
                    break;
                case 0x16:
                    b = 'P';
                    break;
                case 0x1D:
                    b = 'Q';
                    break;
                case 0x0A:
                    b = 'R';
                    break;
                case 0x08:
                    b = 'S';
                    break;
                case 0x03:
                    b = 'T';
                    break;
                case 0x09:
                    b = 'U';
                    break;
                case 0x11:
                    b = 'V';
                    break;
                case 0x0B:
                    b = 'W';
                    break;
                case 0x19:
                    b = 'X';
                    break;
                case 0x1B:
                    b = 'Y';
                    break;
                case 0x1C:
                    b = 'Z';
                    break;
                case 0x2F:
                    b = '1';
                    break;
                case 0x27:
                    b = '2';
                    break;
                case 0x23:
                    b = '3';
                    break;
                case 0x21:
                    b = '4';
                    break;
                case 0x20:
                    b = '5';
                    break;
                case 0x30:
                    b = '6';
                    break;
                case 0x38:
                    b = '7';
                    break;
                case 0x3C:
                    b = '8';
                    break;
                case 0x3E:
                    b = '9';
                    break;
                case 0x3F:
                    b = '0';
                    break;
                default:
                    b = '*';
            }      /* Ende switch */
            lcd_putc(b);
        }

    }
}
