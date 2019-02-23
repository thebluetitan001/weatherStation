#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 9); //Define radio (CE-PIN,CSN-PIN)
//const byte rxAddr[6] = "00001"; //RX adress (Same as on TX)
String received_data;
int counter =0;


void setup()
{
// Start the serial
  Serial.begin(9600);
  Serial.println("NRF24 Receiver");

  radio.begin();
  radio.setPALevel(RF24_PA_LOW); // Transmit Power (MAX,HIGH,LOW,MIN)
  radio.setChannel(0x76);
  radio.setDataRate( RF24_1MBPS ); //Transmit Speeed (250 Kbits)
  radio.openReadingPipe(1,0xF0F0F0F0E1LL);
  radio.startListening();

  pinMode(6, OUTPUT);

}


void loop()
{
  if (radio.available()){    {
    char text[100] = {0}; //Buffer
    
    radio.read(&text, sizeof(text));
    received_data = String(text);
    
    Serial.println(received_data);
    Serial.println(counter);
    
    counter++;
  } 
}
