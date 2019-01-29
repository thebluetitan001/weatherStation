#include <RH_ASK.h>
//#include <SPI.h>

#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

// Connect RED of the AM2315 sensor to 5.0V
// Connect BLACK to Ground
// Connect WHITE to i2c clock to Analog 5
// Connect YELLOW to i2c data to Analog 4

//Create objects of sensor types AM2315, BMP280, RH_ASK

Adafruit_AM2315 am2315;
Adafruit_BMP280 bme;
RH_ASK driver(5000);

int sizeWeather;
//int transmissionLength = 60;

//tipping bucket collector diamter 90mm
//volumne of bucket .7mm
//amount im mm with every tip 0.1mm
//connect one wire to ground
//connect 2nd wire to pin digital interrupt pin 2

//wind direction in degrees
int WindVanePin = A0;
float windVaneCosTotal = 0.0;
float windVaneSinTotal = 0.0;

//wind speed in km
unsigned long resolution = 0;
volatile unsigned long rotations;
volatile double windSpeedCurrent = 0;

//number of times the buckets has tipped
volatile int tippingBucketTips = 0;
volatile long lastRiseTimeRain = 0;
volatile long lastRiseTimeWind = 0;

void setup() {

  Serial.begin(9600);
  //Serial.println("working");
    //enables driver for am2315 temperature and humidty sensor
  if (! am2315.begin()) {
     //Serial.println("Sensor not found, check wiring & pullups!");
     while (1);
  }
 // delay(500);
  if (!bme.begin()) {  
   //Serial.println("Could not find a valid BMP280 sensor, check wiring!");
   while (1);
  }

  //enables 433mhz transmitter
  if (!driver.init()){
      // Serial.println("init failed");
  }
 
  //configure interrupt for tipping bucket scale
  pinMode(2,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), rain, FALLING);
  
  //configure interrupt for wind speed
  pinMode(3,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), wind, FALLING);
  
}

void loop() {
  
  captureWindVane();
  
  rotations = 0;
  delay(3000);
  windSpeedCurrent = rotations * resolution;
  
  generateWeatherString();

}

void rain(){
  
   //If more than 10 ms has elapsed since the last time pin 2 went high
   if ((millis() - lastRiseTimeRain) > 20)
  {

   //Serial.println("rain");
    tippingBucketTips++;
    
  }
  
    lastRiseTimeRain = millis();

} 

void wind(){
  
   //If more than 10 ms has elapsed since the last time pin 2 went high
   if ((millis() - lastRiseTimeWind) > 20)
  {
  
   rotations++;
    
  }
  
    lastRiseTimeWind = millis();

} 

void generateWeatherString(){

      String weather = "AO,";
      weather = "";
    
      weather.concat(am2315.readTemperature());
      weather.concat(",");
      weather.concat(am2315.readHumidity());
      weather.concat(",");
      weather.concat(tippingBucketTips);
      weather.concat(",");
      //weather.concat(getWindVane());
      weather.concat(",");
      weather.concat(windSpeedCurrent);
      weather.concat(",");
      weather.concat(int(bme.readPressure()/100));
      weather.concat(",");
      weather.concat("DDMMYY");

      sizeWeather = weather.length() +1;
      
      char s[int(sizeWeather)];

      weather.toCharArray(s,weather.length()+1);
    

      
      driver.send((uint8_t *)s, RH_ASK_MAX_MESSAGE_LEN);
      
      Serial.println(s);
      
      driver.waitPacketSent();

}
