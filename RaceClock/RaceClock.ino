/*
Race Clock
Components:
RGB LED (countdown)
-3 DIGITAL
--3 220 O Resistor
Passvie Buzzer (countdown)
-1 PVM
4-digit 7-segment LED (Timer)
-3 PVM
-4 Digital
Button (New Lap)
-1 PVM
LCD Screen (Fastest Lap)
-6 PVM
RT Clock (Track Time)
-SDA, SDCL
*/
//#include "pitches.h"  //if I want to replace the melody

#include <LiquidCrystal.h>

#define ULONG_MAX 0UL - 1UL;

//RGB Light
#define RED 23
#define GREEN 22

int colorRed[] = { HIGH, HIGH, LOW };
int colorGreen[] = { LOW, HIGH, HIGH };

//Speaker
#define SPEAKER 9
#define NOTE_B5 988
#define NOTE_C6 1047

int melody[] = {NOTE_B5, NOTE_B5, NOTE_C6};

//Button
#define BUTTON 10

//4-Digit 7-segment number display
#define NUMLATCH 12
#define NUMCLOCK 13
#define NUMDATA 11
unsigned int dataPins[4] = {27, 26, 25, 24};

// LCD Screen
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

void setup()
{
  //RGB Light
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  //no BLUE pin because we don't use it

  //Passive Buzzer
  
  //Button
  pinMode(BUTTON, INPUT_PULLUP);

  //Number display
  pinMode(NUMLATCH, OUTPUT);
  pinMode(NUMCLOCK, OUTPUT);
  pinMode(NUMDATA, OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(dataPins[i], OUTPUT);

  //lcd
  lcd.begin(16, 2);
}

//4-digit 7-segment display
unsigned long int lapTime = 0, fastestLap = ULONG_MAX;
unsigned short digitData[4] = {0};

//each char is a bitmask for the 7-segment digits 0-9
unsigned char numTable[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};

//button
bool buttonPressed = false;

//race time variables
unsigned long startTime = 0, currentTime = 0, prevUpdate = 0;

bool countdown = true;

//update the 7-segment number display
void Display(int digit, unsigned char num)
{
  digitalWrite(NUMLATCH, LOW);
  //the id==2 check lights up the center decimal point (would be the : on a clock)
  shiftOut(NUMDATA, NUMCLOCK, MSBFIRST, numTable[num]);// + (digit == 2 ? 0x80 : 0)); 
  digitalWrite(NUMLATCH, HIGH);
  for (int i = 0; i < 4; i++) digitalWrite(dataPins[i], HIGH);
  digitalWrite(dataPins[digit], LOW);
}

void UpdateDigits() {
  for (int i = 0; i < 4; i++)
  {
    Display(i, digitData[i]);
    delay(5);
  }
}

void loop() {
  if (countdown)
  {
    for (int i = 0; i < 3; i ++)
    {
      digitalWrite(RED, colorRed[i]);
      digitalWrite(GREEN, colorGreen[i]);
      tone(SPEAKER, melody[i], 500);

      delay(1000);
    }
    countdown = false;
    startTime = millis();
    digitalWrite(GREEN, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Fastest Lap: ");
  }
  else  //race timer
  {
    //if the button is pressed
    if (digitalRead(BUTTON) == LOW && !buttonPressed)
    {
      buttonPressed = true;
      startTime = millis(); //reset the counter
      if (currentTime < fastestLap)
      {
        fastestLap = currentTime;
        //updateLCD with new Fastest Lap
        lcd.setCursor(0, 1);
        lcd.print(fastestLap / 1000);
      }
    }
    //if the button is released (after pressing it)
    if (digitalRead(BUTTON) == HIGH && buttonPressed)
    {
      buttonPressed = false;
    }

    UpdateDigits(); //update the counter

    //update the time
    currentTime = millis() - startTime;
    if (currentTime - prevUpdate > updateTime)
    {
      prevUpdate = currentTime;
      //lapTime is current time in seconds
      lapTime = currentTime / 1000;
      if (lapTime >= 10000) lapTime -= 10000; //prevent counter overflow
      //populate the digit display
      for (int i = 0; i < 4; i++) {
        digitData[i] = lapTime % 10;
        lapTime /= 10;
      }
    }
  }
}
