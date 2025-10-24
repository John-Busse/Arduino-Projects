# Race Clock (WIP)
This is a personal project to do something fun with a four-digit seven-segment number display. A basic race clock, featuring a startup phase counting down to race start, and a race phase featuring a digital timer with a button to signal a new lap, with an LCD screen showing the fastest lap.
### The code and system are not yet finished

## Project Parameters
Implement a basic timer for a race, uses the following components:
- RGB LED (flashes from red, to yellow, to green during intial race countdown)
- Passive Buzzer (plays tones in time with the RGB LED during race countdown)
- Push switch (When pressed, signals a new lap, saves the fastest lap time overall)
- Four-digit seven-segment number display (Displays the current lap time in seconds)
	- 74HC595 bit-shift chip
- LCD display (Displays the fastest lap time)
	- 10K potentiometer