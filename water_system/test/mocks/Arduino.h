/* MIT License

   Copyright (c) 2018 Eddy Petrișor

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

extern bool Serial; //mock

typedef uint8_t byte;

#define OUTPUT 1
#define INPUT 2
#define HIGH 1
#define LOW 0

#define A0 0
#define A1 1
#define A2 2
#define A3 3

#define LED_BUILTIN 13

int millis();
void delay(int delay_ms);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int level);
int analogRead(int anpin);
void panicLEDToggle();

#endif
