// tinyIR_BaseStation for ATtiny13A - NEC
//
// IR remote control using an ATtiny 13A. Timer0 generates a 38kHz
// carrier frequency with a duty cycle of 25% on the output pin to the
// IR LED. The signal (NEC protocol) is modulated by toggling the pin
// to input/output. The protocol uses pulse distance modulation.
//
//       +---------+     +-+ +-+   +-+   +-+ +-    ON
//       |         |     | | | |   | |   | | |          bit0:  562.5us
//       |   9ms   |4.5ms| |0| | 1 | | 1 | |0| ...
//       |         |     | | | |   | |   | | |          bit1: 1687.5us
// ------+         +-----+ +-+ +---+ +---+ +-+     OFF
//
// IR telegram starts with a 9ms leading burst followed by a 4.5ms pause.
// Afterwards 4 data bytes are transmitted, least significant bit first.
// A "0" bit is a 562.5us burst followed by a 562.5us pause, a "1" bit is
// a 562.5us burst followed by a 1687.5us pause. A final 562.5us burst
// signifies the end of the transmission. The four data bytes are in order:
// - the 8-bit address for the receiving device,
// - the 8-bit logical inverse of the address,
// - the 8-bit command and
// - the 8-bit logical inverse of the command.
// The Extended NEC protocol uses 16-bit addresses. Instead of sending an
// 8-bit address and its logically inverse, first the low byte and then the
// high byte of the address is transmitted.
//
//                        +-\/-+
// KEY5 --- A0 (D5) PB5  1|    |8  Vcc
// KEY3 --- A3 (D3) PB3  2|    |7  PB2 (D2) A1 --- KEY2
// KEY4 --- A2 (D4) PB4  3|    |6  PB1 (D1) ------ IR LED
//                  GND  4|    |5  PB0 (D0) ------ KEY1
//                        +----+
//
// Controller: ATtiny13
// Core:       MicroCore (https://github.com/MCUdude/MicroCore)
// Clockspeed: 1.2 MHz internal
// BOD:        BOD disabled (energy saving)
// Timing:     Micros disabled (Timer0 is in use)
//
// Note: The internal oscillator may need to be calibrated for the device
//       to function properly.
//
//
// Based on the work of:
// - San Bergmans (https://www.sbprojects.net/knowledge/ir/index.php),
// - Christoph Niessen (http://chris.cnie.de/avr/tcm231421.html),
// - David Johnson-Davies (http://www.technoblogy.com/show?UVE).
// - Stefan Wagner (https://github.com/wagiminator)

// oscillator calibration value (uncomment and set if necessary)
// #define OSCCAL_VAL  48

// libraries
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// IR codes (use 16-bit address for extended NEC protocol)
#define ADDR 0x04 // Address: LG TV
#define KEY1 0x02 // Command: Volume+
#define KEY2 0x00 // Command: Channel+
#define KEY3 0x03 // Command: Volume-
#define KEY4 0x01 // Command: Channel-

// define values for 38kHz PWM frequency and 25% duty cycle
#define TOP 31 // 1200kHz / 38kHz - 1 = 31
#define DUTY 7 // 1200kHz / 38kHz / 4 - 1 = 7

// macros to switch on/off IR LED
#define IRon() DDRB |= 0b00000010  // PB1 as output = IR at OC0B (38 kHz)
#define IRoff() DDRB &= 0b11111101 // PB1 as input  = LED off

// macros to modulate the signals according to NEC protocol with compensated timings
#define startPulse()     \
    {                    \
        IRon();          \
        _delay_us(9000); \
        IRoff();         \
        _delay_us(4500); \
    }
#define repeatPulse()    \
    {                    \
        IRon();          \
        _delay_us(9000); \
        IRoff();         \
        _delay_us(2250); \
    }
#define normalPulse()   \
    {                   \
        IRon();         \
        _delay_us(562); \
        IRoff();        \
        _delay_us(557); \
    }
#define bit1Pause() _delay_us(1120) // 1687.5us - 562.5us = 1125us
#define repeatCode()   \
    {                  \
        _delay_ms(40); \
        repeatPulse(); \
        normalPulse(); \
        _delay_ms(56); \
    }

// send a single byte via IR
void sendByte(uint8_t value)
{
    for (uint8_t i = 8; i; i--, value >>= 1)
    {                  // send 8 bits, LSB first
        normalPulse(); // 562us burst, 562us pause
        if (value & 1)
            bit1Pause(); // extend pause if bit is 1
    }
}

// send complete telegram (start frame + address + command) via IR
void sendCode(uint8_t cmd)
{
    startPulse();          // 9ms burst + 4.5ms pause to signify start of transmission
#if ADDR > 0xFF            // if extended NEC protocol (16-bit address):
    sendByte(ADDR & 0xFF); // send address low byte
    sendByte(ADDR >> 8);   // send address high byte
#else                      // if standard NEC protocol (8-bit address):
    sendByte(ADDR);  // send address byte
    sendByte(~ADDR); // send inverse of address byte
#endif
    sendByte(cmd);  // send command byte
    sendByte(~cmd); // send inverse of command byte
    normalPulse();  // 562us burst to signify end of transmission
}

// main function
int main(void)
{
// set oscillator calibration value
#ifdef OSCCAL_VAL
    OSCCAL = OSCCAL_VAL; // set the value if defined above
#endif

    // set timer0 to toggle IR pin at 38 kHz
    TCCR0A = 0b00100011; // PWM on OC0B (PB1)
    TCCR0B = 0b00001001; // no prescaler
    OCR0A = TOP;         // 38 kHz PWM frequency
    OCR0B = DUTY;        // 25 % duty cycle

    // disable unused peripherals and set sleep mode to save power
    ADCSRA = 0b00000000;                 // disable ADC
    ACSR = 0b10000000;                   // disable analog comperator
    PRR = 0b00000001;                    // shut down ADC
    // set_sleep_mode(SLEEP_MODE_PWR_DOWN); // set sleep mode to power down

    // main loop
    while (1)
    {
        sendCode(0x23);
        _delay_ms(500);
    }
}
