/*********************************************************************
  Synthesizer master clock
  with 1/8, 1/4, 1/2 and 1/1 time beats and Midi sync

  Code by Potatopasty https://tg-music.neocities.org
  potatopasty@pm.me

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see https://www.gnu.org/licenses/.

*********************************************************************/

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <SimpleTimer.h> // https://github.com/marcelloromani/Arduino-SimpleTimer
#include <NewEncoder.h> // https://github.com/gfvalvo/NewEncoder
#include <Button.h> // https://github.com/virgildisgr4ce/Button/
#include <uClock.h> // https://github.com/midilab/uClock 

#include "characters.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Set the timer
SimpleTimer cvtime;
int count = 0;
int run;

// Encoder settings:
// aPin, bPin, minValue, maxValue, initalValue, type (FULL_PULSE for quadrature pulse per detent)
NewEncoder encoder(2, 3, 40, 280, 100, FULL_PULSE);
int16_t prevEncoderValue;

Button buttonStart = Button(10, BUTTON_PULLUP_INTERNAL, true, 50);

#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

void ClockOut96PPQN(uint32_t * tick)
{
  Serial.write(MIDI_CLOCK);
}
void onClockStart()
{
  Serial.write(MIDI_START);
}
void onClockStop()
{
  Serial.write(MIDI_STOP);
}

float BPM;
bool started = false;

/*********************************************************************/

void setup() {

  Serial.begin(31250);

  run = false;

  NewEncoder::EncoderState state;
  encoder.begin();

  uClock.init();
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  uClock.setOnClockStartOutput(onClockStart);
  uClock.setOnClockStopOutput(onClockStop);

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(10, HIGH);

  // LCD settings
  lcd.begin();
  lcd.createChar(0, m);
  lcd.createChar(1, i);
  lcd.createChar(2, d);
  lcd.createChar(3, note);

  // Intro screen
  lcd.setCursor(0, 0);
  lcd.print("SYNTHESIZER");
  lcd.setCursor(0, 1);
  lcd.print("MASTER CLOCK");
  delay(5000); // Pause to show tittle screen
  lcd.clear();
}

void loop() {

  if (buttonStart.uniquePress())
  {
    if (run == false)
    {
      run = true;
      uClock.start();
    }
    else
    {
      run = false;
      uClock.stop();

      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(8, LOW);
    }
  }
  if (run > false)
  {
    cvtime.run();
    if (!started) {
      cycle_on();
      started = true;
    }
  }

  control();  // Rotary encoder function
  disp();     // Display function
  midiclock(); // Midi clock control
}

/*********************************************************************/

void cycle_off() {

  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);

  count++;
  if (count == 8) {
    count = 0;
  }
}

/*********************************************************************/

void cycle_on() {

  float duration_percentage =  map(analogRead(A1), 0, 1023, 1, 90);
  int cycletime = (30000 / BPM);
  long cycle_start = cycletime;
  long cycle_stop = (cycletime * (duration_percentage / 100));

  cvtime.setTimeout(cycle_start, cycle_on);
  cvtime.setTimeout(cycle_stop, cycle_off);

  switch (count) {
    case 0:
      digitalWrite(5, HIGH);  // 8 beat
      digitalWrite(6, HIGH);  // 4 beat
      digitalWrite(7, HIGH);  // 2 beat
      digitalWrite(8, HIGH);  // 1 beat
      break;

    case 1:
      digitalWrite(5, HIGH);
      break;

    case 2:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      break;

    case 3:
      digitalWrite(5, HIGH);
      break;

    case 4:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
      break;

    case 5:
      digitalWrite(5, HIGH);
      break;

    case 6:
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      break;

    case 7:
      digitalWrite(5, HIGH);
      break;
  }
}

/*********************************************************************/

void control() {

  int16_t currentValue;
  NewEncoder::EncoderState currentEncoderState;

  encoder.getState(currentEncoderState);
  currentValue = currentEncoderState.currentValue;

  if (currentValue != prevEncoderValue) {
  }
  BPM = (currentValue);
  prevEncoderValue = currentValue;
}

/*********************************************************************/

void disp() {

  float duration_percentage =  map(analogRead(A1), 0, 1023, 1, 90);

  lcd.setCursor(0, 0);
  lcd.write(3);    
  lcd.print("BPM: ");
  lcd.print(BPM, 0);
  if (BPM < 100) {
    lcd.setCursor(8, 0);
    lcd.print(" ");
  }
  lcd.setCursor(12, 0);
  lcd.write(0);
  lcd.write(1);
  lcd.write(2);
  lcd.write(1);
  lcd.setCursor(0, 1);
  lcd.print("Duty: ");
  lcd.print(duration_percentage, 0);
  if (duration_percentage < 10) {
    lcd.setCursor(7, 1);
    lcd.print(" ");
  }
}

/*********************************************************************/

void midiclock() {
  uClock.setTempo(BPM);
}

/*********************************************************************/
