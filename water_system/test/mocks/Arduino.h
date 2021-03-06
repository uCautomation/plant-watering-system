/* MIT License

   Copyright (c) 2018-2019 Eddy Petrișor

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */
/*
   This file is providing the way to let Arduino specific code/classes to be tested as unit tests running on the host,
   instead of the target.
 */

#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#ifndef ULONG_MAX
#define ULONG_MAX UINT_MAX
#endif


extern bool Serial; //mock

typedef uint8_t byte;

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#define HIGH 1
#define LOW 0

#define CHANGE 1

#define A0 0
#define A1 1
#define A2 2
#define A3 3

#define LED_BUILTIN 13

int millis();
void delay(int delay_ms);
void pinMode(int pin, int mode);
int digitalRead(int pin);
void digitalWrite(int pin, int level);
int analogRead(int anpin);
void interrupts();
void noInterrupts();

// PROGMEM related APIs
#define PROGMEM
#define memcpy_P(src,dst,size) memcpy(src,dst,size)
#define F(msg) (msg)

#define digitalPinToInterrupt(x)
#define attachInterrupt(interupt,isr,mode)

#ifndef digitalPinToInterrupt
#define digitalPinToInterrupt(isr,isrno,evt) 0
#endif

long map(long x, long in_min, long in_max, long out_min, long out_max);
long constrain(long v, long min, long max);

#ifdef DEBUG_ON
    #define DEBUG(fmt, ...) printf(fmt "\n" , ##__VA_ARGS__)
    #define DEBUG_P(msg) DEBUG(msg)
#else
    #define DEBUG(fmt, ...)
    #define DEBUG_P(msg)
#endif

#endif
