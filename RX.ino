#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <math.h>

RF24 radio(10,9); // CE, CSN
const byte address[] = "PUI";
int led = 4;

void setup() 
{
  pinMode(led,OUTPUT);
  
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  while(!Serial);
  Serial.println("Số liệu thống kê");
}

void loop() 
{
 //-----------------------------------------------//
struct package
{
  float X1 , X2  , X3;
};
typedef struct package Package;
Package data;
//--------------------------------------------------//
 
//=============================TRANSMIITION===================================//
  radio.startListening();
  if (radio.available())
  {
    while (radio.available()) 
       {
        radio.read(&data, sizeof(data));
        delay(5);
       } 
   if((data.X1 > 0) && (data.X2 > 0) && (data.X3 > 0) )
   {    
   Serial.print("\n");
   Serial.print("Trình diện:");
   //Serial.println(data.X0);
   
   Serial.print("\t");
   Serial.print("Nhiệt đô cơ thể:");
   Serial.println(data.X1);
   
   Serial.print("\t");
   Serial.print("Nhiệt độ phòng:");
   Serial.println(data.X2);
   
   Serial.print("\t");
   Serial.print("Khoảng cách đo:");
   Serial.println(data.X3);
   }
  
   //delay(100);
  }
}
