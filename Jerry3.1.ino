/*UNDERWATER METAL DETECTOR - PROTOTYPE ONE MASTER PROGRAM
  DATE: SEPTEMBER 14TH, 2017
  DESCRIPTION: THIS PROGRAM IS TO LOG FREQUENCY AND HUMIDITY VALUES THAT CAN BE TRANSFERRED TO AN EXCEL FILE AND EVERY
  TIME YOU DISCONNECT AND RECONNECT THE POWER IT WILL SAVE A NEW FILE AS LONG AS IT IS A TWO DIGIT NUMBER. IT COLLECTS 
  FREQUENCY AND HUMIDITY VALUES.
  Latest entry on May 15th, 2019
 */

/* ==================================== LIBRARIES ==================================== */

#include <FreqMeasure.h>
#include <SD.h> // SD card library
#include <DS3231.h> // real time clock
#include <DHT.h>

/* ================================= METAL DETECTOR ================================= */

// #define and const: 
// According to Arduino official reference, const is prefered in most cases as they are almost the same.
const int blue_led = 6;
const int red_led = 3;
const int green_led = 5;
// use int for frequency, which is float number, won't cause any errors. Will only get an integer instead of float number.
static int frequency = 0; // pin 49 --> PIN # IS IN LIBRARY

// use double instead of long refer to the official documentation. References: https://www.pjrc.com/teensy/td_libs_FreqMeasure.html 
double sum = 0;
int count = 0;
const int avg = 20;
unsigned long dataSavedTime = 0;

/*================================== FREQUENCY THRESHOLD ====================================*/

//const int red=10;
//const int green=150;
//const int yellow=250;
//const int purple=400;
//const int lblue=550;
//const int whites=700;

const int red=50;
const int purple=450;

/*===================================== RTC ======================================*/

DS3231 rtc(SDA,SCL);

/* ================================= SD READER ================================== */

int chipSelect = 10; //chipSelect pin for the SD card Reader
File mySensorData; //Data object you will write your sensor data to
static String filename;

/* ==================================== DHT ===================================== */

// using const will use more dynamic memory so here we should keep using #define
#define DHTPIN 9
#define DHTTYPE DHT11 // DHT11 sensor is smaller and blue the DHT22 is the white/larger 
DHT dht(DHTPIN, DHTTYPE); // This means(pin plugged in, type which is "DHT11")

/* =========================== SET TIME ATTRIBUTE TO DATA FILES =========================== */

void dateTime(unsigned int* date, unsigned int* time) {
 // return date using FAT_DATE macro to format fields
 *date = FAT_DATE(rtc.getTime().year, rtc.getTime().mon, rtc.getTime().date);  
 // return time using FAT_TIME macro to format fields
 *time = FAT_TIME(rtc.getTime().hour, rtc.getTime().min, rtc.getTime().sec);
}

/* =========================== VOID SETUP (RUN ONCE) =========================== */

void setup(){
  FreqMeasure.begin();
  pinMode(chipSelect, OUTPUT); //Must declare 10 an output and reserve it
  SD.begin(chipSelect); //Initialize the SD card reader
  pinMode(blue_led,OUTPUT);
  pinMode(red_led,OUTPUT);
  pinMode(green_led ,OUTPUT);
  rtc.begin();

//  Can uncomment lines below to reset/calibrate the data and time. 
//  MUST comment the line begins with SdFile before or after uncomment the lines below. Sometimes without doing so will cause error to compile.
//  rtc.setTime(14, 44, 07); // Set/calibrate the time (24hr format, hh,mm,ss)
//  rtc.setDate(26, 6, 2019); // Set/calibrate the date to June 13th, 2019 (dd, mm, yyyy)

//  MUST comment the line below before or after uncomment the lines to set/calibrate the time. Sometimes without doing so will cause error to compile.
  SdFile::dateTimeCallback(dateTime); // call dateTime function to add time attribute to the files.

// CREATE A Folder and NEW FILE
// If the real-time clock works properly then the code will use current time as the file name which is more efficient
// And if it doesn't work properly then the code will use increasing numbers as the file name which is less efficient
//  if (rtc.getTime().year > 2018){
//  String month = rtc.getMonthStr();
//  if(!SD.exists(month)){
//    // create a folder using current month as its name
//    SD.mkdir(month);
//  }
//  String timeString = rtc.getTimeStr();
//  String dateDay = rtc.getDateStr();
//  // the filename contains the full file path, such as "June/07112351"
//  // the file will be created inside the "month folder"
//  filename = month + '/' + dateDay[0] + dateDay[1] +timeString[0] + timeString[1] + timeString[3] + timeString[4] + timeString[6] + timeString[7] + ".csv";
//  }
//  else{
    for (uint8_t i = 1; i < 1000; i++){
      filename = "DATA-" + String(i) + ".csv";
    if (! SD.exists(filename)){
      // only open/create a new file if it doesn't exist
      mySensorData = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
      }
    }
//  }
  mySensorData = SD.open(filename, FILE_WRITE);
  mySensorData.println("UnderWater Metal Detector Data Logging Accessory Rev2.0\nDate of the test:,");
  mySensorData.println(rtc.getDateStr());
  mySensorData.println("Starting time:,");
  mySensorData.println(rtc.getTimeStr());
  mySensorData.println("Color range:,");
  mySensorData.println("Red(10Hz) Green(150hz) Yellow(250Hz) Purple(400Hz) LightBlue(550Hz) LowWhite(700Hz)");
  mySensorData.println("\nTime,Frequency (Hz),Humidity (%),Temperature (Celsius)");
  mySensorData.close();
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
      sdCard();
      reset();
    }
  }
  else if ( millis() - dataSavedTime > 200 ){
    // This is a Zero Handling. 
    // Add calculation on how much time has passed since we could get data last time and then determine if we consider the frequency is 0.
    // In this statement means, it's already 200 milliseconds passed which is long enough for us to regard the frequency as 0.
    // And if the time that has passed is less than 200 milliseconds, then we do nothing. 
    // Because this is most likely just the interval of the FreqMeasure function.
    frequency = 0;
    digitalWrite( blue_led, LOW);
    analogWrite( red_led, 100);
    digitalWrite( green_led, LOW);
    sdCard();
  }
  else{
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
  mySensorData.close();
}

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

//  if (frequency <= red){//Red - No Signal
//    digitalWrite( blue_led, LOW);
//    digitalWrite( red_led, HIGH);
//    digitalWrite( green_led, LOW);
//  }
//  else if ( (frequency <= purple) && (frequency > yellow) ){//Purple
//    digitalWrite( blue_led, HIGH);
//    digitalWrite( red_led, HIGH);
//    digitalWrite( green_led, LOW);
//  }
//  else if ( frequency > whites ){//Full White
//    digitalWrite( blue_led, HIGH);
//    digitalWrite( red_led, HIGH);
//    digitalWrite( green_led, HIGH);
//  }
//  else if ( (frequency <= green) && (frequency > red) ){//Green
//    digitalWrite( blue_led, LOW);
//    digitalWrite( red_led, LOW);
//    digitalWrite( green_led, HIGH);
//  }
//  else if ( (frequency <= yellow) && (frequency > green) ){//Yellow
//    digitalWrite(blue_led,LOW);
//    digitalWrite( red_led, HIGH);
//    digitalWrite( green_led, HIGH);
//  }
//  else if ( (frequency <= lblue)&& (frequency > purple) ){//Lightblue
//    digitalWrite( blue_led, HIGH);
//    digitalWrite( red_led, LOW);
//    digitalWrite( green_led, HIGH);
//  }
//  else {//Low White
//    analogWrite( blue_led, 200);
//    analogWrite( red_led, 100);
//    analogWrite( green_led, 200);    
//  }
}
