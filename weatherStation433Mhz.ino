#include <RH_ASK.h>

#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

// AM2315 sensor wiring
// Connect RED to 5.0V
// Connect BLACK to Ground
// Connect WHITE to i2c clock to Analog 5
// Connect YELLOW to i2c data to Analog 4

Adafruit_AM2315 am2315;
Adafruit_BMP280 bme;
RH_ASK driver(5000);

String weather = "AO,";

const int tranmissionDelayTime = 5000;
int sizeWeather;

//tipping bucket collector diamter 90mm
//volumne of bucket .7mm
//amount im mm with every tip 0.1mm
//connect one wire to ground
//connect 2nd wire to pin digital interrupt pin 2

//wind direction in degrees

const int windVaneSampleSize = 16;
long newWindVaneTime;
long lastWindVaneTime = 0;
long windVaneCompare;
String windVaneDirection = "";
int count;
int arrayCounter;
bool RESETCOUNTER;

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

  PCICR |= (1 << PCIE2);
  //Enables Interrupt on Pin 9
  pinMode(7,INPUT_PULLUP);
  
  PCMSK2 |= (1 << PCINT23);
  lastWindVaneTime = micros();

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
 /*
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    //Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }
  */ 
  
  //configure interrupt for tipping bucket scale
  
  pinMode(2,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), rainISR, FALLING);
  
  //configure interrupt for wind speed
  pinMode(3,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), windISR, FALLING);
  
}

void loop() {

  rotations = 0;
  delay(tranmissionDelayTime);
  windSpeedCurrent = rotations * resolution;
  generateWeatherString();
  RESETCOUNTER = false;
  count = 0;
  
}

ISR(PCINT2_vect) {

  //grabs stream of data coming from wind vane, and stores the final 4 bits in a string for transmission

  newWindVaneTime = micros();
  windVaneCompare = newWindVaneTime - lastWindVaneTime;
  
  //finds the start bit in the datastream, times appears to be around 3000ms
  if((windVaneCompare ) < 4000 and (windVaneCompare) > 2000){
     count = 0;
     arrayCounter = 0;
     windVaneDirection = "";
   }

  if(RESETCOUNTER == false){ 
    if(count < windVaneSampleSize){
    
      //modulus function differentiates between the rise and fall of the pin to know when to start reading time of the bit
      
      if((count % 2) == 1){
        if((windVaneCompare)>700){
        
        //greater than 700ms is a 1, below is a zero
        //stores the final 4 bits in the array 
        //final 4 bits contains the directional data
         
          if(arrayCounter > 3 and arrayCounter < 8){
            windVaneDirection.concat(1);  
          }
          arrayCounter ++;
        }else{
        if(arrayCounter > 3 and arrayCounter < 8){
          windVaneDirection.concat(0);  
        }
        arrayCounter ++;
        if(arrayCounter > 8){
          RESETCOUNTER == true;
        }
        }
        
      }
       count++;
      }
      lastWindVaneTime = newWindVaneTime;
    }
}

void rainISR(){
  
   //If more than 10 ms has elapsed since the last time pin 2 went high
   if ((millis() - lastRiseTimeRain) > 10)
  {

    tippingBucketTips++;
    
  }
  
    lastRiseTimeRain = millis();

} 

void windISR(){
  
   //If more than 10 ms has elapsed since the last time pin 2 went high
   if ((millis() - lastRiseTimeWind) > 20)
  {
  
   rotations++;
    
  }
  
  lastRiseTimeWind = millis();

} 

void generateWeatherString(){

      //display.clearDisplay();

      weather = "";   
      weather.concat(am2315.readTemperature());

      weather.concat(",");
      weather.concat(am2315.readHumidity());
      weather.concat(",");
      weather.concat(tippingBucketTips);
      weather.concat(",");
      weather.concat(windVaneDirection);
      weather.concat(",");
      weather.concat(windSpeedCurrent);
      weather.concat(",");
      weather.concat(int(bme.readPressure()/100));

      sizeWeather = weather.length() +1;
      
      char s[int(sizeWeather)];

      weather.toCharArray(s,weather.length()+1);
      
      driver.send((uint8_t *)s, RH_ASK_MAX_MESSAGE_LEN);

      Serial.println(s);

      driver.waitPacketSent();

}
