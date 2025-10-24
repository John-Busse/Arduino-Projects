// CPE 301- Semester Project; Swamp Cooler
// Team 14- John Busse, Dylan Sherrillo

/* Appropriate Libraries:
 *  Servo Motor
 *  DHT11
 *  LCD
 *  DS 1307 RTC
 * 
 * Unusable Libraries:
 *  DC Motor
 *  Water Sensor
 *  Potentiometers
 *  Thermister
 *  
 *  Wired Library:
 *  GPIO:
 *    [pinMode] may not be used
 *    [digitalRead] and [digitalWrite] may not be used
 *  ADC:
 *    [analogRead] and [analogWrite] may not be used
 *  We may use [Serial] library methods
 *  Timer configurations muse be performed as they were in class
 */
 
/*Components Needed:
 * Button       (Start/Stop)            [p.59, Lesson 5] Pin13 (PB7)
 * RGB LED      (State monitor)         [p.50, Lesson 4] Pin12, 11, 10 (PB6, PB5, PB4)
 * -220 resistor for each light
 * Fan + Motor                          [p.191,Lesson29] Pin9, 8, 7 (PH3, PE3, PG5)
 * -Power Supply
 * -L293D IC
 * DHT Temp/Humidity Sensor             [p.91, Lesson12] Pin 6, 5V (PE1)
 * Water Level Sensor                   [p.131,Lesson18] A0, 5V
 * LCD Screen   (Humidity/Temp readout) [p.152,Lesson22] Pin12-7 5V Pin 22, 23, 24, 25, 26, 27
 * -Potentiometer (10K)
 * Real Time Clock Module               [p.136,Lesson19] SDA/SCL pins, 5V
 */
 
/*
 * Completed Project Requirements:
 * Monitor the water levels in a reservoir and print an alert when the level is too low
 * Monitor and display the current air temp and humidity on an LCD screen
 * Start and stop a fan motor as needed when the temperature falls out of a specified range (high or low)
 * Allow a user to use a control to adjust the angle of an output vent from the system
 * Allow a user to enable or disable the system using an on/off button
 * Record the time and date every time the motor is turned on or off. This information should be transmitted to a host computer (over USB)
 */

#include <DHT.h>            // DHT Temperature/Humidity Sensor (Pin 6)
#include <LiquidCrystal.h>  // LCD Display (Pins 22-27)
#include <Wire.h>

// Define registers and whatnot

//DHT
DHT dht(6, DHT11); //DHT temp/humid sensor, pin 6, DHT type DHT11

// LCD
LiquidCrystal lcd(22, 23, 24, 25, 26, 27); // LCD Screen
//  [22] = (RS) Register Select
//  [23] = (E) Enabling pin
//  [24-27] = Data 

// Digital Port B Registers; Button (PB7), LCDs (PB6, PB5, PB4)  [Blue, Green, Red]
volatile unsigned char* port_b = (unsigned char*) 0x25;
volatile unsigned char* ddr_b = (unsigned char*) 0x24;
volatile unsigned char* pin_b = (unsigned char*) 0x23;

// Digital Port H Registers; Fan/Motor (PH6, PH5, PH4)
volatile unsigned char* port_h = (unsigned char*) 0x18;
volatile unsigned char* ddr_h = (unsigned char*) 0x17;
volatile unsigned char* pin_h = (unsigned char*) 0x16;

//Analog; Water Level Sensor
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADCH_DATA = (unsigned int*) 0x79;
volatile unsigned int* my_ADCL_DATA = (unsigned int*) 0x78;
unsigned char WATER_LEVEL_PORT = 0;

// Timer Pointers
volatile unsigned char *myTCCR1A  = (unsigned char*) 0x80;
volatile unsigned char *myTCCR1B  = (unsigned char*) 0x81;
volatile unsigned char *myTCCR1C  = (unsigned char*) 0x82;
volatile unsigned char *myTIMSK1  = (unsigned char*) 0x6F;
volatile unsigned char *myTIFR1   = (unsigned char*) 0x36;
volatile unsigned int  *myTCNT1   = (unsigned int* ) 0x84;

#define WATER_THRESHOLD 100
#define TEMP_THRESHOLD 70.0

//States
enum state {
 off = 0, //Yellow
 idle = 1, //Green
 error = 2, //Red
 cooling = 3 //Blue
};

// Start in off state
enum state cooler = off;

void setup() {
  adc_init();           // Initialize the ADC, used for the water level sensor
  
  Serial.begin(9600);   // Initialize Serial Port, 9600 Baud rate

  lcd.begin(16, 2);     // Set up lcd, 16 columns, 2 rows
  //use: lcd.print("Text or ");
  //      lcd.print(variable);

  dht.begin();          // Initialize DHT temp/humid sensor

  Wire.begin();         // Initialize the clock wire

  //Port B setup, set PB7 as input, PB 6-4 as output
  *ddr_b |= 0x70; //Set set bits 6, 5, and 4 to 1 to enable output
  *port_b &= 0x0F;  // Drive all relevant bits low
  // PB7 is the button
  // PB6 is the blue LED
  // PB5 is the green LED
  // PB4 is the red LED
  // Red + Green = yellow LED

  //Port H setup, set PB6-PB4 as output
  // x111xxxx
  *ddr_h &= 0x00;
  // PB6- Enable Fan, PB5 & 4 control direction
  
  *port_h |= 0x20;  //Establish fan direction
}

void loop() {
  delay(1000);
 
  unsigned int water = adc_read(0);
  float temp = dht.readTemperature(true); //Reads the temp in fahrenheit
  float humi = dht.readHumidity();
  
  Serial.print("Humidity: ");
  Serial.print(humi);
  Serial.print("% Temperature: ");
  Serial.print(temp);
  Serial.print(" Water: ");
  Serial.print(water);
  Serial.print('\n');
  
  switch(cooler)
  {
    case off:
      Serial.print("Disabled\n");
      off_state();
      break;
    case idle:
      Serial.print("Idle\n");
      idle_state();
      break;
    case error:
      Serial.print("Error\n");
      error_state();
      break;
    case cooling:
      Serial.print("Cooling\n");
      cooling_state;
      break;
    default:
      break;
  }
}

// Machine States
/*DISABLED state: Yellow LED
  No monitoring of temp or water
  Only monitor start button*/
void off_state()
{
  lcd.clear();      // clear the LCD screen
  lcd.noDisplay();  // turn off the LCD display

  *port_h &= 0xBF;  // Turn off fan
  *port_b &= 0x80;  // Turn the LEDs off
  *port_b |= 0x30;  // Turn yellow on

  //Wait for the button on PB7 to be pressed
  while ((*pin_b & (1 << 7)) == 0) { }

  cooler = idle;
  lcd.display();    // turn the LCD back on
}

/* Idle state: Green LED
    Monitor water level and temp
    if Water gets too low, go to ERROR state
    if Temp goes too high, go to COOLING state
    record exact transition times using real time clock*/
void idle_state()
{
  //~~record start time~~
  *port_h &= 0xBF;  // Turn off fan
  *port_b &= 0x80;  // Turn the LEDs off
  *port_b |= 0x20;  // Turn green on

  unsigned int water = adc_read(0); // Read water level
  float temp = dht.readTemperature(true); //Temp in fahrenheit
  float humi = dht.readHumidity();

  lcd.print("Temperature: ");
  lcd.print(temp);
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humi);

  if (water < WATER_THRESHOLD)
    {
      //record transition time
      cooler = error;
    }
  else if (temp > TEMP_THRESHOLD)
    {
      //record transition time
      cooler = cooling; 
    }
}

/* ERROR state: Red LED
 Motor off
 Transition to idle when water is above THRESHOLD
 Display error message on LCD
 */
void error_state()
{
  *port_h &= 0xBF;  // Turn off fan
  *port_b &= 0x80;  // Turn the LEDs off
  *port_b |= 0x10;  // Turn red on

  lcd.clear();
  lcd.print("Low Water");

  unsigned int water = adc_read(0); // Read water level

  while (water < WATER_THRESHOLD)
  {
    delay(1000);
      water = adc_read(0);
      lcd.setCursor(0, 1);  //Force the LCD to the second line
      lcd.print("Level: ");
      lcd.print(water);
  }

  cooler = idle;
  lcd.clear();
}

// Cooling state: Blue LED, Fan on, check water level and temperature
void cooling_state() //
{
  // Enable fan and blue light
  *port_b &= 0x80; // Turn the LEDs off
  *port_b |= 0x40; // Turn blue on

  *port_h |= 0x40; // Turn on the fan

  //Serial.print("Fan on\n");
  //lcd.print("Fan on");

  
  // Check water level and tempearture
  // If water level is below THRESHOLD, cooler = error
  // If tempurature is below THRESHOLD, cooler = idle
  // otherwise, print temp and humidity, cooler = cooling
}

// Initialize ADC
void adc_init()
{
  // set up the A register
  // set bit 7 to 1 to enable ADC
  // set bit 5 to 0 to disable ADC trigger
  // set bit 3 to 0 to disable ADC interrupt
  // set bit 2-0 to 0 to set prescaler selection to slow reading
  // 1X0X0000
  *my_ADCSRA |= 0x80; // set bit 7 to 1 to enable the ADC
  *my_ADCSRA &= 0XD0; // clear bits 5, 3-0 to 0
  
  // set up the B register
  // bit 3 to 0 to reset channel and gain bits
  // bits 2-0 to 0 to set free running mode
  // XXXX0000
  *my_ADCSRB &= 0xF0; // clear bits 3-0 to 0
  
  // set up the MUX Register
  // bit 7 to 0 for AVCC reference
  // bit 5 to 0 for right adjust result
  // bit 4-0 to 0 to reset the channel and gain bits
  // bit 6 to 1 for AVCC analog reference
  // 01000000
  *my_ADMUX  &= 0x00; // clear all bits to 0
  *my_ADMUX  |= 0x40; // set bit 6 to 1
}

unsigned int adc_read(unsigned char adc_channel_num)
{
  // reset the channel and gain bits (MUX 4:0)
  // XXX00000;
  *my_ADMUX  &= 0xE0;
  
  // clear the channel selection bits (MUX 5)
  // XXXX0XXX
  *my_ADCSRB &= 0xF7;
  
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    
    // set MUX bit 
    *my_ADCSRB |= 0x01;
  }
  
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x60;
  
  // wait for the conversion to complete
  // while flag bit = 0
  while((*my_ADCSRA&(1<<ADIF))==0);
  // reset ADC interrupt flag bit 4
  *my_ADCSRA |= 0x10;
  
  // return the result in the ADC data register
  return *my_ADCH_DATA;
}