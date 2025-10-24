# Swamp Cooler
- This was my final project for CPE 301- Embedded Systems Design at the University of Nevada, Reno, during the Fall 2021 semester.

### Project parameters
Prototype a swamp cooler using the following components:
- Push switch (to start/stop the simulation)
- RGB LED (changes color indicating the cooler's state)
- DHT Temperature/Humidity sensor
- Water Level sensor
- Fan + Motor (turns on when the temperature falls out of the needed range)
- LCD Screen (display Humidity and temperature)
	- 10K potentiometer

### Project restrictions
We weren't allowed to use the following GPIC or ADC functions: pinMode, digitalRead, digitalWrite, analogRead, or analogWrite. With this restriction, we had to use the processor pin addresses to access components, and binary operators to manipulate the data.