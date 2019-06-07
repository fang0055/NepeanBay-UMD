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

// use float instead of long refer to the official documentation. References: https://www.pjrc.com/teensy/td_libs_FreqMeasure.html 
// use int won't cause any errors though. Will only get an int instead of float number.
static float frequency = 0; // pin 49 --> PIN # IS IN LIBRARY

// use double instead of long refer to the official documentation. References: https://www.pjrc.com/teensy/td_libs_FreqMeasure.html 
double sum = 0;
int count = 0;
const int avg = 30;
unsigned long dataSavedTime = 0;
const char sample[]={"\nTime,Frequency (Hz),Humidity (%),Temperature (Celsius)"};

/*================================== THRESHOLD ====================================*/

const int red=10;
const int green=150;
const int yellow=250;
const int purple=400;
const int lblue=550;
const int whites=700;

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
  Serial.begin(9600);
  pinMode(chipSelect, OUTPUT); //Must declare 10 an output and reserve it
  SD.begin(chipSelect); //Initialize the SD card reader
  pinMode(blue_led,OUTPUT);
  pinMode(red_led,OUTPUT);
  pinMode(green_led ,OUTPUT);
  rtc.begin();

  // rtc.setTime(14, 51, 07); // Set the time/calibrate the time (24hr format)

  SdFile::dateTimeCallback(dateTime);

  //CREATE A Folder and NEW FILE
  String month = rtc.getMonthStr();
  if(!SD.exists(month)){
    // create a folder using current month as its name
    SD.mkdir(month);
  }
  String timeString = rtc.getTimeStr();
  String dateDay = rtc.getDateStr();
  // the filename contains the full file path, such as "June/07112351"
  filename = month + '/' + dateDay[0] + dateDay[1] +timeString[0] + timeString[1] + timeString[3] + timeString[4] + timeString[6] + timeString[7] + ".csv";
  // the file will be created inside the "month folder"
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
  // frequency <= red may be most likely to happen in real operation, so put it in the first place will make the if else statement more efficient.
  // Because when the frequency is less than red, then the code will excute the code inside that situation then jump out of the whole if else statement.
  
  if (frequency <= red){//Red - No Signal
    digitalWrite( blue_led, LOW);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
  }
  else if ( (frequency <= green) && (frequency > red) ){//Green
    digitalWrite( blue_led, LOW);
    digitalWrite( red_led, LOW);
    digitalWrite( green_led, HIGH);
  }
  else if ( (frequency <= yellow) && (frequency > green) ){//Yellow
    digitalWrite(blue_led,LOW);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, HIGH);
  }
  else if ( (frequency <= purple) && (frequency > yellow) ){//Purple
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
  }
  else if ( (frequency <= lblue)&& (frequency > purple) ){//Lightblue
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, LOW);
    digitalWrite( green_led, HIGH);
  }
  else if ( (frequency <= whites)&& (frequency > lblue) ){//Low White
    analogWrite( blue_led, 200);
    analogWrite( red_led, 100);
    analogWrite( green_led, 200);    
  }
  else {//Full White
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, HIGH);
  }
}
