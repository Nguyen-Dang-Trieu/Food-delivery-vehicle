#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <math.h>

#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Servo.h>


//RF24//
RF24 radio(48,49); //=========================== CE, CSN corrected
const byte address[] = "PUI";


//SERVO//
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
int pos = 0;
int turn = 0;


//PIR//
int PIR = 11;
int led = 13;
int waitled = 14;
int wait = 0;

int PIRcount = 0;
int PIRmiss  = 0;
int PIRstate = 0;


//Infrared//
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(-1);
float roomTemp; //ambient temperature
float objectTemp,stemp; //Object temperature
int readcount = 0;
float threshold = 0;  // add giá trị tăng thêm

int warning = 0, done = 0;



//Ultrasonic preparation//
float echoPin = 6;
float trigPin = 5;

int minimumRange = 12;
int maximumRange = 25;
long duration, distance;
int far, dtime;
 
// **far** : Khi"Quá xa"đo trong thời gian quá lâu => kích hoạt giả



//---------------------Following line car base--------------------------//

//L298N - H bridge//
  const int motorA1      = A0;  // kết nối chân IN1 với chân 3 arduino
  const int motorA2      = A1;  // kết nối chân IN2 với chân 4 arduino
  const int motorAspeed  = 9;  // kết nối chân ENA với chân 5 arduino
  
  const int motorB1      = A2; // kết nối chân IN3 với chân 7 arduino
  const int motorB2      = A3; // kết nối chân IN4 với chân 8 arduino
  const int motorBspeed  = 10;  // kết nối chân ENB với chân 6 arduino

//FC-51 IR sensor//
   int L_S = 17;  // cb dò line trái 
   int C_S = 18;  // cb dò line giữa
   int R_S = 19; //cb dò line  phải 

void setup()
{ 
  Serial.begin(115200);

  //setting up the SERVOES//
  servo1.attach(8);
  servo2.attach(7);
  servo3.attach(15);
  servo4.attach(16);
  

  //setting up the Ulstrasonic
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);


  //setting up the PIR
  pinMode(PIR,INPUT);
  pinMode(led,OUTPUT);
  pinMode(waitled,OUTPUT);
  
  digitalWrite(waitled,LOW);
  delay(3000);
  digitalWrite(waitled,HIGH);
  delay(100);
  digitalWrite(waitled,LOW);
  delay(100);
  digitalWrite(waitled,HIGH);
  delay(100);
  digitalWrite(waitled,LOW);
  delay(100);
  
  
  // setting up the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay (100);
  display.clearDisplay();
  display.setTextColor(WHITE);
  delay(1000);

  
  // setting up the RF24
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
  
  while(!Serial);
  Serial.println("Số liệu thống kê:");
  Serial.print("\n");

  // setting up following line car base
  pinMode(L_S,INPUT); // chân cảm biến khai báo là đầu vào
  pinMode(C_S,INPUT);
  pinMode(R_S,INPUT);
  
  pinMode(motorA1, OUTPUT); 
  pinMode(motorA2, OUTPUT);
  pinMode(motorB1, OUTPUT);
  pinMode(motorB2, OUTPUT);
  pinMode(motorAspeed, OUTPUT);
  pinMode(motorBspeed, OUTPUT);
 
  Serial.begin(9600); 
  analogWrite(motorAspeed, 120); // tốc độ động cơ a ban đầu 120 ( 0 - 255)
  analogWrite(motorBspeed, 120);// tốc độ động cơ b ban đầu 120 ( 0 - 255)
  delay(1000);
  

  // LED show that the robot had ready to do it duty
    digitalWrite(waitled,HIGH);
    delay(100);
    digitalWrite(waitled,LOW);
    delay(100);
    digitalWrite(waitled,HIGH);
    delay(100);
    digitalWrite(waitled,LOW);
    delay(100);
    digitalWrite(waitled,HIGH);
    delay(100);
    digitalWrite(waitled,LOW);
    delay(500);
    
    digitalWrite(waitled,HIGH);
    delay(100);
    digitalWrite(waitled,LOW);
    delay(100);
    digitalWrite(waitled,HIGH);
    delay(100);
    digitalWrite(waitled,LOW);
    delay(100);
    digitalWrite(waitled,HIGH);
    delay(100);
    digitalWrite(waitled,LOW);
    delay(1000);
}

void loop()
{
    if ((digitalRead(L_S) == 0)&&(digitalRead(C_S) == 1)&&(digitalRead(R_S) == 0)){forward();}// đi tiến
    
    if ((digitalRead(L_S) == 1)&&(digitalRead(C_S) == 1)&&(digitalRead(R_S) == 0)){turnRight();} 
    if ((digitalRead(L_S) == 1)&&(digitalRead(C_S) ==0)&&(digitalRead(R_S) == 0)) {turnRight();}
    
    if ((digitalRead(L_S) == 0)&&(digitalRead(C_S) == 1)&&(digitalRead(R_S) == 1)){turnLeft();}  
    if ((digitalRead(L_S) == 0)&&(digitalRead(C_S) == 0)&&(digitalRead(R_S) == 1)){turnLeft();}
    
    if ((digitalRead(L_S) == 1)&&(digitalRead(C_S) == 1)&&(digitalRead(R_S) == 1))
    {
      Stop();
      xoay();
      open_case();
      
      do
      {
        test();
        if(done == 1)
        {
          turn++;
          break;
        }
      }while(done == 0);
      
      close_case();
      reset_xoay();
      done = 0;
      delay(3000);
    
   //======= Move forward a little bit to get out of parking line========//
    analogWrite (motorAspeed, 90);
    analogWrite (motorBspeed, 90);
    digitalWrite (motorB1,HIGH);
    digitalWrite (motorA2,HIGH);
    delay(600);
    }
}

void test()
{
 display.clearDisplay();
 int PIRvalue = digitalRead(PIR);
 int PIRmiss = 0;
 do
 {
  PIRmiss += 1;
  Serial.println(PIRmiss);
  delay(100);
  
      if(PIRvalue == 1)
      {
        PIRstate = 1;
        Serial.println(PIRvalue);
        delay(100);
        break;
      }
      
      if(PIRmiss > 100)
      {
        done = 1;
        transmittion(); //====== call the police and doctor ======//
        delay(200);
        call_police();
        delay(200);
        break;
      }
 }while( digitalRead(PIR) < 1 );
 Serial.print("\n");
 
 do
 {
  measure();
  if(PIRstate == 0)
  {
    break;
  }
 }while(PIRstate = 1);
 
}

void measure()
{
//------------------------------------------------------------------------------------//
  
  digitalWrite(trigPin,HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
 
  
  //Caculate the distance(in cm) based on the speed of sound.
  distance = duration*0.034/2;


  //reading object and ambient temperature
  objectTemp = threshold + mlx.readObjectTempC();
  roomTemp = mlx.readAmbientTempC();


  //display on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,25);
  display.print("Xa:" + String(distance) + "cm");
  
  display.setCursor(60,25);
  display.print("Phong:" + String(roomTemp).substring(0,4)+ "C");
  
  display.setCursor(0,15);
  display.print("YOUR:" + String(objectTemp).substring(0,4)+ "C");
  
  display.display();
  display.setCursor(0,0);
  
  if(distance > maximumRange)
  {
    display.print("QUA XA, GAN LAI XIU ");
    far++;
    Serial.print("\t");
    Serial.print(far);
    if(far > 10)
    {
      far = 0;
      PIRstate = 0;
    }
  }

  else if(distance < minimumRange)
  {
    display.print("QUA GAN, XA RA XIU");
  }

  else if((distance >= minimumRange) && (distance <= maximumRange))
  {
    if(readcount == 5)  //after reading 5 consecutive times
    {
      disptemp();
      PIRcount = 0;
      PIRmiss  = 0;
      PIRstate = 0;
      close_case();
      cang();
      //=========  mở/ đóng để chào  ==========//
      for (pos = 100; pos <= 180; pos += 1)  
      { 
        servo1.write(pos);             
        delay(14);                       
      }
      //=======================================//
    }
    else 
    {
      display.print("HAY GIU VI TRI");
      stemp = stemp + objectTemp;
      readcount++;
      dtime = 200;       // until approximate 5 x 200ms = 1 sec
    }
  }

  else // if user is out off range, reset calculation
   {
    readcount = 0;
    stemp = 0;
    dtime = 5;
   }
display.display();
delay(dtime);
//----------------------------------------------------------------------------------//
}


void disptemp()
{
  objectTemp = stemp/5;     //get the average reading of temp
  display.setTextSize(1);
  display.print("NHIET DO CUA BAN:");
  display.setTextSize(2);
  display.setCursor(65,8);  
  display.print(String(objectTemp).substring(0,4) + "C");
  display.display();
  readcount = 0;
  stemp = 0;
  if (objectTemp >= 37.5)
  {
    play_alert();
    warning = 1;
  }
  else
  {
    play_ok();
    warning = 0;
  }
  done = 1;
  transmittion();
  dtime = 1000;//----------------------------------------wait for 5 sec
}

void transmittion()
{
//=====================================TRANSMITTION===================================//
    struct package
    {
      float  X0 = objectTemp , X1 = roomTemp , X2 = distance ;
      int    X3 = PIRstate   , X4 = warning ;
    };
    typedef struct package Package;
    Package data;
    radio.write(&data, sizeof(data));
    delay(10);
//====================================================================================//
}

void xoay()
{
  if(turn %2 == 0)
      {
        for (pos = 100; pos <= 180; pos += 1) //======== LEFT =========// 
        { 
          servo2.write(pos);             
          delay(14);                       
        }
      }
   else 
      {
        for (pos = 100; pos >= 0; pos -= 1) //======== RIGHT =========// 
        { 
          servo2.write(pos);             
          delay(14);                       
        }
      }
}

void reset_xoay()
{
  if(turn %2 == 0)
      {
        for (pos = 0; pos <= 100; pos += 1) //======== Reset RIGHT =========// 
        { 
          servo2.write(pos);             
          delay(14);                       
        }
      }
  else
      {
        for (pos = 180; pos >= 100; pos -= 1) //======== Reset LEFT =========// 
        { 
          servo2.write(pos);             
          delay(14);                       
        }
      }
}

void open_case()
{
  for (pos = 100; pos <= 180; pos += 1) //=========  mở  ==========// 
  { 
    servo1.write(pos);             
    delay(14);                       
  }
}

void close_case()
{
  for (pos = 180; pos >= 100; pos -= 1) //========  đóng  =========// 
  { 
    servo1.write(pos);             
    delay(14);                       
  }
}

void cang() //==========mở xong tự đóng lại============//
{
  if(turn %2 == 0)
  {
    for (pos = 90; pos >= 0; pos -= 1) //======== mở càng =========// 
      { 
        servo4.write(pos);             
        delay(14);                       
      }
    delay(15000);

    for(pos = 0; pos <= 90; pos += 2) //========= đóng càng ======// 
      {
        servo4.write(pos);             
        delay(14);
      }
  }
  else
  {
    for (pos = 90; pos >= 0; pos -= 1) //======== mở càng =========// 
      { 
        servo3.write(pos);             
        delay(14);                       
      }
    delay(15000);

    for(pos = 0; pos <= 90; pos += 2) //========= đóng càng ======// 
      {
        servo3.write(pos);             
        delay(14);
      }
  }
}
void play_alert()
{
  tone(13, 2000, 1000);      // pin, frequency, duration
  delay(1000);
  tone(13, 3000, 1000);
  delay(1000);
  tone(13, 4000, 1000);
  delay(1000);
  noTone(3);
}


void play_ok()
{
  tone(13, 600, 1000);
  delay(200);
  tone(13, 750, 500);
  delay(100);
  tone(13, 1000, 500);
  delay(200);
  noTone(3);
}


void call_police()
{
  for( int i = 0 ; i <3 ; i++)
  {
  tone(13, 2000, 1000);      
  delay(1000);
  tone(13, 3000, 1000);
  delay(1000);
  noTone(3);
  }
}

//////==========================  xe dò line =================================/////

void setMotors(int speedA, int speedB, bool dirA1, bool dirA2, bool dirB1, bool dirB2) {
  analogWrite(motorAspeed, speedA);
  analogWrite(motorBspeed, speedB);
  
  digitalWrite(motorA1, dirA1);
  digitalWrite(motorA2, dirA2);                       
  digitalWrite(motorB1, dirB1);
  digitalWrite(motorB2, dirB2);
}

void forward() {
  setMotors(90, 90, LOW, HIGH, HIGH, LOW);
}

void turnRight() {
  setMotors(200, 200, HIGH, LOW, HIGH, LOW);
  delay(200);
}

void turnLeft() {
  setMotors(200, 200, LOW, HIGH, LOW, HIGH);
  delay(200);
}

void Stop() {
  setMotors(0, 0, LOW, LOW, LOW, LOW);
}
