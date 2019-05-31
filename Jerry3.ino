/*UNDERWATER METAL DETECTOR - PROTOTYPE ONE MASTER PROGRAM
  DATE: SEPTEMBER 14TH, 2017
  DESCRIPTION: THIS PROGRAM IS TO LOG FREQUENCY AND HUMIDITY VALUES THAT CAN BE TRANSFERRED TO AN EXCEL FILE AND EVERY
  TIME YOU DISCONNECT AND RECONNECT THE POWER IT WILL SAVE A NEW FILE AS LONG AS IT IS A TWO DIGIT NUMBER. IT COLLECTS 
  FREQUENCY AND HUMIDITY VALUES.
  Latest entry on May 15th, 2019
 */

/* ================================= METAL DETECTOR ================================= */
#include <FreqMeasure.h>
// #define and const: 
// According to Arduino official reference, const is prefered in most cases as they are almost the same.
const int blue_led = 6;
const int red_led = 3;
const int green_led = 5;
static float frequency = 0; // pin 49 --> PIN # IS IN LIBRARY
double sum = 0;
int count = 0;
const int avg = 15;
unsigned long dataSavedTime = 0;
// static int x = 1;
char sample[]={"\nTime,Frequency (Hz),Humidity (%),Temperature (Celsius)"};

/*========== THRESHOLD ==========*/
const int red=10;
const int green=150;
const int yellow=250;
const int purple=400;
const int lblue=550;
const int whites=700;

/*=======================RTC========================================================*/
#include <DS3231.h>
DS3231 rtc(SDA,SCL);

/* ================================= SD READER ================================= */
#include <SD.h> //Load SD card library

int chipSelect = 10; //chipSelect pin for the SD card Reader
File mySensorData; //Data object you will write your sensor data to
static char filename[] = "JERRY-00.csv";

/* =================================    DHT    ================================= */
#include <DHT.h>
// using const will use more dynamic memory so here we should keep using #define
#define DHTPIN 9
#define DHTTYPE DHT11 // DHT11 sensor is smaller and blue the DHT22 is the white/larger 

DHT dht(DHTPIN, DHTTYPE); // This means(pin plugged in, type which is "DHT11")

/* =========================== VOID SETUP (RUN ONCE) =========================== */
void setup(){
  FreqMeasure.begin();
  pinMode(10, OUTPUT); //Must declare 10 an output and reserve it
  SD.begin(10); //Initialize the SD card reader
  pinMode(blue_led,OUTPUT);
  pinMode(red_led,OUTPUT);
  pinMode(green_led ,OUTPUT);
  rtc.begin();
  
//  if (x == 1){
//    //The following lines can be uncommented to set the date and time
//    //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
//    //rtc.setTime(11, 18, 0);     // Set the time to 12:00:00 (24hr format)
//    //rtc.setDate(15, 5, 2019);   // Set the date to January 1st, 2014 (dd, mm, yyyy)
//  }

  /* =========================== CREATE A NEW FILE =========================== */

  for (uint8_t i = 0; i < 100; i++){
    filename[6] = i / 10 + '0'; // ON THE 6TH CHARACTER IN YOUR FILE NAME IT RECOGNIZES THE NUMBER BASICALLY AND THIS LINE AND THE NEXT MAKES IT COUNT UP SO YOU DONT HAVE THE SAME FILE NAME 
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)){
      // only open a new file if it doesn't exist
      mySensorData = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }
  if (mySensorData){
    mySensorData.println("UnderWater Metal Detector Data Logging Accessory Rev2.0\nDate of the test:,");
    mySensorData.println(rtc.getDateStr());
    mySensorData.println("Starting time:,");
    mySensorData.println(rtc.getTimeStr());
    // mySensorData.println("\nTime,Frequency (Hz),Humidity (%),Temperature (Celsius),Red(10Hz),Green(150hz),Yellow(250Hz),Purple(400Hz),LightBlue(550Hz),LowWhite(700Hz)");
    mySensorData.close();
  }
}
    
void loop() {
  Frequency();
}

void Frequency(){
  // FreqMeasure needs "zero handling" method to set frequency to zero. 
  // References: https://www.pjrc.com/teensy/td_libs_FreqMeasure.html 
  if (FreqMeasure.available()) {
    // average several readings together
    dataSavedTime = millis();
    sum = sum + FreqMeasure.read();
    count = count + 1;
    if (count > avg) {
      frequency = FreqMeasure.countToFrequency(sum / count);
      ledLIGHT();
      sdCard();
      reset();
    }
  }
  else if ( millis() - dataSavedTime > 300 ){
    // This is a Zero Handling. 
    // Add calculation on how much time has passed since we could get data last time and then determine if we consider the frequency is 0.
    // In this statement means, it's already 300 milliseconds passed which is long enough for us to regard the frequency as 0.
    // And if the time that has passed is less than 300 milliseconds, then we do nothing. 
    // Because this is most likely just the interval of the FreqMeasure function.
      frequency = 0;
      digitalWrite( blue_led, LOW);
      analogWrite( red_led, 100);
      digitalWrite( green_led, LOW);
      sdCard();
  }
}

/* =========================== Logs information on SD Card =========================== */

void sdCard(){
  float h = dht.readHumidity(); //READING THE HUMIDITY %
  float t = dht.readTemperature();
  mySensorData = SD.open(filename, FILE_WRITE);
  mySensorData.print("\n");
  mySensorData.print(rtc.getTimeStr());
  mySensorData.print(",");
  mySensorData.print(frequency); //Print Your results
  mySensorData.print(",");
  mySensorData.print(h);
  mySensorData.print(",");
  mySensorData.print(t);
  // mySensorData.print(",10,150,250,400,550,700,");
  mySensorData.close();
  //close the file
}

void reset(){
  count = 0;
  sum = 0;
}

/* =================================    Visual LED Alerts    ================================= */
// Using if...else is more efficient than just if. 
// Becasue we don't need to check every situation once the frequency is and will only be in one of the range at a time.
// And putting those most likely happend condition in first places will also make the code a little more efficient.

void ledLIGHT() {
  // frequency <= red may be most likely to happen in real operation, so put it in the first place will make the if else statement more efficient.
  // Because when the frequency is less than red, then the code will excute the code inside that situation then jump out of the whole if else statement.
  
  if (frequency <= red){//Red - No Signal
    digitalWrite( blue_led, LOW);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
    //delay(500);
  }
  else if ( (frequency <= green) && (frequency > red) ){//Green
    digitalWrite( blue_led, LOW);
    digitalWrite( red_led, LOW);
    digitalWrite( green_led, HIGH);
    //delay(500);    
  }
  else if ( (frequency <= yellow) && (frequency > green) ){//Yellow
    digitalWrite(blue_led,LOW);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, HIGH);
    //delay(500);    
  }
  else if ( (frequency <= purple) && (frequency > yellow) ){//Purple
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
    //delay(500);  
  }
  else if ( (frequency <= lblue)&& (frequency > purple) ){//Lightblue
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, LOW);
    digitalWrite( green_led, HIGH);
    //delay(500);  
  }
  else if ( (frequency <= whites)&& (frequency > lblue) ){//Low White
    analogWrite( blue_led, 200);
    analogWrite( red_led, 100);
    analogWrite( green_led, 200);    
    //delay(500);
  }
  else {//Full White
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, HIGH);
    //delay(500);    
  }
}
