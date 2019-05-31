/*UNDERWATER METAL DETECTOR - PROTOTYPE ONE MASTER PROGRAM
  DATE: SEPTEMBER 14TH, 2017
  DESCRIPTION: THIS PROGRAM IS TO LOG FREQUENCY AND HUMIDITY VALUES THAT CAN BE TRANSFERRED TO AN EXCEL FILE AND EVERY
  TIME YOU DISCONNECT AND RECONNECT THE POWER IT WILL SAVE A NEW FILE AS LONG AS IT IS A TWO DIGIT NUMBER. IT COLLECTS 
  FREQUENCY AND HUMIDITY VALUES.
  Latest entry on May 15th, 2019
 */

/* ================================= METAL DETECTOR ================================= */
#include <FreqMeasure.h>
#define blue_led 6
#define red_led 3
#define green_led 5
static long fq = 0 ;
static long frequency = 0; // pin 49 --> PIN # IS IN LIBRARY
long sum = 0;
int count = 0;
int avg = 15;
static int x = 1;
char sample[]={"\nTime,Frequency (Hz),Humidity (%),Temperature (Celsius)"};

/*========== THRESHOLD ==========*/
int red=10;
int green=150;
int yellow=250;
int purple=400;
int lblue=550;
int whites=700;

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
  
  if (x == 1){
    //The following lines can be uncommented to set the date and time
    //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
    //rtc.setTime(11, 18, 0);     // Set the time to 12:00:00 (24hr format)
    //rtc.setDate(15, 5, 2019);   // Set the date to January 1st, 2014 (dd, mm, yyyy)
  }

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
  if (! mySensorData){
    //Couldn't create file
  }
  else{
    /////////////
    mySensorData.println("UnderWater Metal Detector Data Logging Accessory Rev2.0\nDate of the test:,");
    mySensorData.println(rtc.getDateStr());
    mySensorData.println("Starting time:,");
    mySensorData.println(rtc.getTimeStr());
    mySensorData.println("\nTime,Frequency (Hz),Humidity (%),Temperature (Celsius),Red(10Hz),Green(150hz),Yellow(250Hz),Purple(400Hz),LightBlue(550Hz),LowWhite(700Hz)");
    mySensorData.close();
    ////////////
  }
}
    
void loop() {
  Frequency();
}

void Frequency(){
  if (FreqMeasure.available()) {
    // average several readings together
    sum = sum + FreqMeasure.read();
    count = count + 1;
    if (count > avg) {
      frequency = FreqMeasure.countToFrequency(sum / count);
      fq  = (((frequency+5)/10)*10);
      //This line above adds 5 to the fq then div by 10 then multiply by 10--- ex.. 441 +5 = 446 / 10 = 46 * 10 = 460---- 446 + 5 = 451 / 10 = 45 * 10 =450
      ledLIGHT();
      sdCard();
      reset();
    }
  }
  else{
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
  mySensorData.print(fq); //Print Your results
  mySensorData.print(",");
  mySensorData.print(h);
  mySensorData.print(",");
  mySensorData.print(t);
  mySensorData.print(",10,150,250,400,550,700,");
  mySensorData.close();
  //close the file
}

void reset(){
  count = 0;
  frequency = 0;
  sum = 0;
  fq = 0;
}

/* =================================    Visual LED Alerts    ================================= */

void ledLIGHT() {
  if ( (fq <= green) && (fq > red) ){//Green
    digitalWrite( blue_led, LOW);
    digitalWrite( red_led, LOW);
    digitalWrite( green_led, HIGH);
    //delay(500);    
  }
  if ( (fq <= yellow) && (fq > green) ){//Yellow
    digitalWrite(blue_led,LOW);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, HIGH);
    //delay(500);    
  }
  if ( (fq <= purple) && (fq > yellow) ){//Purple
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
    //delay(500);  
  }
  if ( (fq <= lblue)&& (fq > purple) ){//Lightblue
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, LOW);
    digitalWrite( green_led, HIGH);
    //delay(500);  
  }
  if ( (fq <= whites)&& (fq > lblue) ){//Low White
    analogWrite( blue_led, 200);
    analogWrite( red_led, 100);
    analogWrite( green_led, 200);    
    //delay(500);
  }
  if (fq > whites){//Full White
    digitalWrite( blue_led, HIGH);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, HIGH);
    //delay(500);    
  }
  if (fq <= red){//Red - No Signal
    digitalWrite( blue_led, LOW);
    digitalWrite( red_led, HIGH);
    digitalWrite( green_led, LOW);
    //delay(500);
  }
}
