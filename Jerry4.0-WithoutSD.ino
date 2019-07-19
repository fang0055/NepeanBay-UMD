/*UNDERWATER METAL DETECTOR - PROTOTYPE ONE MASTER PROGRAM
  DATE: SEPTEMBER 14TH, 2017
  DESCRIPTION: THIS PROGRAM IS TO LOG FREQUENCY AND HUMIDITY VALUES THAT CAN BE TRANSFERRED TO AN EXCEL FILE AND EVERY
  TIME YOU DISCONNECT AND RECONNECT THE POWER IT WILL SAVE A NEW FILE AS LONG AS IT IS A TWO DIGIT NUMBER. IT COLLECTS 
  FREQUENCY AND HUMIDITY VALUES.
  Latest entry on July 4th, 2019
 */

/* ==================================== LIBRARIES ==================================== */

#include <FreqMeasure.h>

/* ================================= METAL DETECTOR ================================= */

// #define and const: 
// According to Arduino official reference, const is prefered in most cases as they are almost the same.
const int blue_led = 6;
const int red_led = 3;
const int green_led = 5;
// use int for frequency, which is a float number, won't cause any errors. Will only get an integer instead of float number.
static int frequency = 0; // pin 49 --> PIN # IS IN LIBRARY

// use double instead of long refer to the official documentation. References: https://www.pjrc.com/teensy/td_libs_FreqMeasure.html 
double sum = 0;
int count = 0;
const int avg = 20;
unsigned long dataSavedTime = 0;

/*================================== FREQUENCY THRESHOLD ====================================*/

const int red=50;
const int purple=450;

/* =========================== VOID SETUP (RUN ONCE) =========================== */

void setup(){
  FreqMeasure.begin();
  pinMode(blue_led,OUTPUT);
  pinMode(red_led,OUTPUT);
  pinMode(green_led ,OUTPUT);
}
    
void loop() {
  // Measure the frequency
  // FreqMeasure needs "zero handling" method to determine if the frequency should be zero or not. 
  // References: https://www.pjrc.com/teensy/td_libs_FreqMeasure.html 
  if (FreqMeasure.available()) {
    // average several frequency readings together, refered to official documentation: https://www.pjrc.com/teensy/td_libs_FreqMeasure.html 
    dataSavedTime = millis();
    sum = sum + FreqMeasure.read();
    count = count + 1;
    if (count > avg) {
      frequency = FreqMeasure.countToFrequency(sum / count);
      ledLIGHT();
      reset();
    }
  }
  else if ( millis() - dataSavedTime > 100 ){
    // This is a Zero Handling. 
    // Add calculation on how much time has passed since we could get data last time and then determine if we consider the frequency is 0.
    // In this statement means, it's already 160 milliseconds passed which is long enough for us to regard the frequency as 0.
    // And if the time that has passed is less than 160 milliseconds, then we do nothing. 
    // Because this is most likely just the interval of the FreqMeasure function.
    frequency = 0;
    digitalWrite( blue_led, LOW);
    analogWrite( red_led, 100);
    digitalWrite( green_led, LOW);
  }
}

/* =========================== Reset the value =========================== */

void reset(){
  count = 0;
  sum = 0;
}

/* =============================== Visual LED Alerts =============================== */

// Using if...else is more efficient than just if. 
// Becasue we don't need to check every situation once the frequency is and will only be in one of the range at a time.
// And putting those most likely happend condition in first places will also make the code a little more efficient.

void ledLIGHT() {
  // frequency <= red, or yellow < frequency <= purple, or frequency > white may most likely to happen in real operation, so put them in the first place will make the if else statement more efficient.
  // Because when the frequency meets one of those 3 ranges, the code will excute the lines inside that situation then jump out of the whole if else statement.

  if (frequency <= red){ //Red - Noise
    digitalWrite( blue_led, LOW);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
  }
  else if ( (frequency <= purple) && (frequency > red) ){ //Purple
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
  }
  else { // White
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, HIGH);
  }
}
