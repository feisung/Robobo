/***************************************************************************************************
*  Necto Firmware version 1.0           Public Alpha                                               *
*  Firmware written by Fei Manheche based on Amarino's Library                                     *
*                                                                                                  *
*  Please note that Creative Commons Attribution Non-Commercial Share Alike licence applies to     *
*  this software.                                                                                  *
*                                                                                                  *
*  visit robobo.org for more info.                                                                 *
****************************************************************************************************/
 
#include <MeetAndroid.h>
#include <SoftwareSerial.h>
#include <Servo.h> 

// inits a SoftwareSerial BT connection on rxPin=2, txPin=3,
//****Ensure you set the BAUD Rate of your BT Module******
MeetAndroid meetAndroid(2,3, 9600);


//Incoming data from Android flags  
const float DIFF = 2;
const int X = 0;
const int Y = 1;
const int Z = 2;

float axis[3];    //Create an array for the 3 XYZ axis returned from accelerometer f(x) on Android

//Constants and variable decalaration for motion control of Hacked RC car
#define iforward 4
#define iback    5
#define ileft    6    //Change These values accordingly!!
#define iright   7    //Change this!!

//this controls the multicolored lamp app
// we need 3 PWM pins to control the leds
int redLed = 9;   
int greenLed = 10;
int blueLed = 11;

//These control the SRF05 Ultrasonic transducer
int echoPin = 12;              // the SRF05's echo pin
int initPin = 13;              // the SRF05's init pinint ultraSoundpin = 6;
unsigned long pulseTime = 0;  // variable for reading the pulse
const int numReadings = 10;   // set a variable for the number of readings to take
int distance = 0;   // variable for storing distance
int index = 0;                // the index of the current reading
int total = 0;                // the total of all readings
int average = 0;              // the average

//These constants are for the Temperature sensor on Analog pin 0
int temperaturePin = 1;    
float temperature = 0;
int reading = 0;

//Constants for the light metronome
volatile unsigned char t = 1;			//This is the time between the beats, enbles the
int delay_speed = t*(100);			//speed formula, increased depending on number of pushes

int loopvalue = 0;

void setup()  
{
  // use the baud rate your bluetooth module is configured to 
  // not all baud rates are working well, i.e. ATMEGA168 works best with 57600
  Serial.begin(9600);   //New Bluetooth Baud Rate
  initIO();             //This initiates the motors and motor control
/*
**To Do!!
**  Add flag and controllable initialization of pins for battery performance
*/
  //Initiate Lamps and Ultrasonic
  initLamp();
  initSRF05();
  // register callback functions, which will be called when an associated event occurs.
  // - the first parameter is the name of your function (see below)
  // - match the second parameter ('A', 'B', 'a', etc...) with the flag on your Android application
//  meetAndroid.registerFunction(accelerometer, 'A');
  meetAndroid.registerFunction(goforward, 'f');
  meetAndroid.registerFunction(goreverse, 'b');  
  meetAndroid.registerFunction(goleft, 'l');  
  meetAndroid.registerFunction(goright, 'r');    
  meetAndroid.registerFunction(gostopm, 's');
  meetAndroid.registerFunction(checkUsound, 'u');
  meetAndroid.registerFunction(ReadTemperature, 't');
  meetAndroid.registerFunction(loopcheck, 'L');    //This function inserts a function into the loop, i.e. contant action 0 == off
  //for multicolored lamp
    // register callback functions, which will be called when an associated event occurs.
  meetAndroid.registerFunction(red, 'o');
  meetAndroid.registerFunction(green, 'p');  
  meetAndroid.registerFunction(blue, 'q'); 
  //Metronome function
  meetAndroid.registerFunction(metronome, 'M'); 
  meetAndroid.registerFunction(metronomeTempo, 'N'); 

}

void loop()
{
  meetAndroid.receive(); // you need to keep this in your loop() to receive events
  loopfunction();
}



void reverse()
{
  digitalWrite(iback, LOW);
  digitalWrite(iforward, HIGH);
}

void forward()
{
  digitalWrite(iforward, LOW);
  digitalWrite(iback, HIGH);
}

void right()
{
  digitalWrite(iright, LOW);  
  digitalWrite(ileft, HIGH);
}

void left()
{
  digitalWrite(ileft, LOW);
  digitalWrite(iright, HIGH);
}

void stopm()
{
  digitalWrite(iforward, HIGH);
  digitalWrite(iback, HIGH);
  digitalWrite(ileft, HIGH);
  digitalWrite(iright, HIGH);
}

float string2Float(String str) 
{
  char arr[str.length()];
  str.toCharArray(arr, sizeof(arr));
  return atof(arr);
}

/*
* Control using app buttons
* The ideology here is that when the button on the app is pressed the motors move forward
* and once the button is released the motor stops.
*/
void goforward(byte flag, byte numOfValues)
{
  if(meetAndroid.getInt() > 0){
     forward();
  }
  else
    digitalWrite(iforward, HIGH);
}

void goreverse(byte flag, byte numOfValues)
{
    if(meetAndroid.getInt() > 0){
         reverse();
    }
    else
    digitalWrite(iback, HIGH);
}

void goleft(byte flag, byte numOfValues)
{
    if(meetAndroid.getInt() > 0){
     left();
    }
    else
    digitalWrite(ileft, HIGH);
}

void goright(byte flag, byte numOfValues)
{
    if(meetAndroid.getInt() > 0){
     right();
    }
    else
    digitalWrite(iright, HIGH);
}

void gostopm(byte flag, byte numOfValues)
{
     stopm();
}

/************************************************************************************************************
*               THis is the Motor Handling code (Still being debugged)                                      *
*                        DC MOTOR CONTROL PARAMETERS                                                        *
*************************************************************************************************************/

//Function that initiates the System for the motors
void initIO(){
  pinMode(iforward, OUTPUT);
  pinMode(iback, OUTPUT);
  pinMode(ileft, OUTPUT);
  pinMode(iright, OUTPUT);
  digitalWrite(iforward, HIGH);
  digitalWrite(iback, HIGH);
  digitalWrite(ileft, HIGH);
  digitalWrite(iright, HIGH);
  
}

//Function that initiates the Multicolored Lamps
void initLamp(){// set all color leds as output pins
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  
  // just set all leds to high so that we see they are working well
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, HIGH);
  digitalWrite(blueLed, HIGH);
}

//Function that initiaes the SRF05 Ultrasonic transducer
void initSRF05(){
      // make the init pin an output:
  pinMode(initPin, OUTPUT);
  // make the echo pin an input:
  pinMode(echoPin, INPUT);
  digitalWrite(initPin, LOW);
  delayMicroseconds(50);
}

//###############################################################End of Function##########################################

//Multicolored lamp functions

/*
 * Whenever the multicolor lamp app changes the red value
 * this function will be called
 */
void red(byte flag, byte numOfValues)
{
  analogWrite(redLed, meetAndroid.getInt());
}

/*
 * Whenever the multicolor lamp app changes the green value
 * this function will be called
 */
void green(byte flag, byte numOfValues)
{
  analogWrite(greenLed, meetAndroid.getInt());
}

/*
 * Whenever the multicolor lamp app changes the blue value
 * this function will be called
 */
void blue(byte flag, byte numOfValues)
{
  analogWrite(blueLed, meetAndroid.getInt());
}


/*************************************Function for the SRF05****************************
*    Code written by Fei Manheche                                                      *  
*    Last updated: 05.03.2012                                                          *
****************************************************************************************/

void checkUsound(byte flag, byte numOfValues){
        meetAndroid.send(ultrasound());
          delay(100);    //insert a delay just to not overload the receiver If loads of android errors are captured, increase this value!!
}

unsigned int ultrasound(){       
        delayMicroseconds(60);                                 // wait 60 microseconds before trying to request readings again
        digitalWrite(initPin, HIGH);                           // send signal
        delayMicroseconds(50);                                 // wait 50 microseconds for it to return
        digitalWrite(initPin, LOW);                            // close signal
        pulseTime = pulseIn(echoPin, HIGH);                    // calculate time for signal to return
        distance = pulseTime/58;                               // convert to centimetres
        
  // output
      return distance;
}

//////////////////This is a generic loop function/////////////////////////////////////////////
/////This function places a function we wish to be run in a loop on the microprocessor////////

void loopcheck(byte flag, byte numOfValues){
        loopvalue = meetAndroid.getInt();     
}

void loopfunction(){
  switch (loopvalue){
  case 0:
    //do nothing i.e. no function or command will be looped
    break;
  case 1:
    meetAndroid.send(ultrasound());
    break;     
  case 2:
    meetAndroid.send(temperatureReading());
    break;  
    default:
    break;
  } 
  
  
  delay(100);    //insert a delay just to not overload the receiver
}

/*############################################################################################
#             Function that controls the analog input for the Temperature sensor             #
#This function will implement a new method where the hard work happens on the microcontroller#
#and only updating changes to the phone. Means better efficiency in battery terms.           #
#                                                                                            #
#  instruction available at: http://www.ladyada.net/learn/sensors/tmp36.html                 #
#  Author: Fei Manheche                                                                      #
#  Last modified: 08.03.2012                                                                 #
#############################################################################################*/

//initiate the temperature sensor pin
float temperatureReading(){
    //we get the reading for the sensor
    reading = analogRead(temperaturePin);
    // In our case we use the 5 volts pin from the arduino, so the formula is:
    //Voltage at pin in milliVolts = (reading from ADC) * (5000/1024) 
   float voltage = reading * (5000/1024);
     //     voltage = voltage/1024.0; 
   // now print out the temperature
   temperature = (voltage - 500) /10;  //converting from 10 mv per degree wit 500 mV offset
                                                 //to degrees ((volatge - 500mV) times 100)                                              
     //temperature = ((reading * (5000/1024))-500)/10;
  //output in degrees other conversions can be made in the phone itself.
  return temperature;
}

void ReadTemperature(byte flag, byte numOfValues){
        meetAndroid.send(temperatureReading());
}
  
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+        Function for Light Metronome Controlled Remotely Functions locally                   +
+ This Function was written by Fei Manheche                                                   +
+ Parts of the code may have been modified from the Author's Group Design Project whilst at UEL+
+ Copyrght                            Reserved                          2012                  +
+                                                                                             +
+  This Function is in Alpha Development: Caution!!
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//*********************************************Functions for each of the Preset beats************************************************

//This function controls how the different beats are set
void metronome(byte flag, byte numOfValues)
{
  int beats = meetAndroid.getInt();

  switch(beats)
  {
         case 0:
     two_beats();
     break;
    
//    state = up_one;

    case 1:
    three_beats();
    break;
  
//    state = up_two;
    
    case 2:
    four_a_beats();
    break;  
//    state = up_three;
    
    case 3:
    four_b_beats();
    break; 
//    state = up_four;
    
    case 4:
    six_beats();
    break;  
    
    case 5:
    seven_beats();
    break;
    
    case 6:
    eight_a_beats();
    break;
    
    case 7:
    eight_b_beats();
    break;
    
    case 8:
    eight_b_beats();
    break;
    
    case 9:
    eight_c_beats();
    break;
    
    case 10:
    nine_beats();
    break;
    
    case 11:
    twelve_beats();
    break;
    
    default:
    two_beats();
    break;
  }
}

//This function adjusts the tempo
void metronomeTempo(byte flag, byte numOfValues){
  t = meetAndroid.getInt();
  delay_speed = t*(100);
}


void blinkgreenLed()                             //This is the predefined function that blinks the White LED
{
  digitalWrite(greenLed, HIGH);
  delay(100);
  digitalWrite(greenLed, LOW);
  delay(100);
}

void blinkRedLED()                               //This is the predefined function that blinks the Red LED
{
  digitalWrite(redLed, HIGH);
  delay(100);
  digitalWrite(redLed, LOW);
  delay(100);
}

void blinkredLed()                             //This is the predefined function that blinks the Yellow LED
{
  digitalWrite(redLed, HIGH);  
  delay(100);
  digitalWrite(redLed, LOW);  
  delay(100);
  
}
  

void two_beats()                    //Main preset function for 2 beats: white then red
{
  // for 2 beats to the bar
    blinkgreenLed();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
} 

void three_beats()
{
  //for 3 beats 96 on metronome website
    two_beats();
    blinkRedLED();
    delay(delay_speed);
}

void four_a_beats()
{
    //for 4a beats ?? on metronome website
    three_beats();
    blinkRedLED();
    delay(delay_speed);
}

void four_b_beats()
{
    //for 4b beats ?? on metronome website
    two_beats();
    blinkredLed();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
}
 void five_beats()
{  
    //for 5 beats ?? on metronome website
    three_beats();
    blinkredLed();
    delay(delay_speed);
    blinkredLed();
    delay(delay_speed);
}
 

void six_beats()
{
    five_beats();
    blinkRedLED();
    delay(delay_speed);
}

void seven_beats()
{
    four_a_beats();
    delay(delay_speed);
    blinkredLed();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
}

void eight_a_beats()    //must check these beats
{
    seven_beats();
    blinkRedLED();
    delay(delay_speed);
}

void eight_b_beats()      //to be checked!!
{
    six_beats();
    blinkredLed();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
}

void eight_c_beats()
{
    four_b_beats();
    blinkredLed();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
    blinkredLed();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
    blinkRedLED();
    delay(delay_speed);
}

void nine_beats()		// to be verified as is a repeat sequence.
{
    eight_b_beats();
    blinkRedLED();
    delay(delay_speed);
}

void twelve_beats()		//to be verified as is the same as 9 beats
{
eight_b_beats();
    blinkRedLED();
    delay(delay_speed);
}
