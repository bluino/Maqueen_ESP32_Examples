#include <Ps3Controller.h>
#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

#define ENABLE_RIGHT_MOTOR_PIN 12
#define RIGHT_MOTOR_PIN 14
#define ENABLE_LEFT_MOTOR_PIN 13
#define LEFT_MOTOR_PIN 15
#define BUZZER_PIN 4
#define LIGHT_LEFT_PIN 18
#define LIGHT_RIGHT_PIN 19
#define SERVO_PIN 32
#define RGB_PIN 23
#define HCSR04_TR_PIN 16
#define HCSR04_EC_PIN 17
#define LDR_PIN 27
#define IR_LINE_RIGHT_PIN 34
#define IR_LINE_LEFT_PIN 35
#define IR_RIGHT_PIN 39
#define IR_LEFT_PIN 36
#define BUTTON_PIN 5

#define LED_COUNT 4

int MAX_SPEED=255;
int MIN_SPEED=0;

Adafruit_NeoPixel strip(LED_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

Servo myservo;
uint32_t color;
bool redS = 0;
bool greenS = 0;
bool blueS = 0;
int red = 0;
int green = 0;
int blue = 0;

bool lightLeftStatus=false;
bool lightRightStatus=false;
bool buttonStatus=false;

const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int rightMotorPWMSpeedChannel = 4;
const int leftMotorPWMSpeedChannel = 5;

int brightRGB=50;
int colorRGB;
int numColorRGB=13;

long duration;
int distance;
bool lastTurn;

void setup(){
  Serial.begin(115200);

  pinMode(ENABLE_LEFT_MOTOR_PIN,OUTPUT);
  pinMode(LEFT_MOTOR_PIN,OUTPUT);
  pinMode(ENABLE_RIGHT_MOTOR_PIN,OUTPUT);
  pinMode(RIGHT_MOTOR_PIN,OUTPUT);

  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(LIGHT_LEFT_PIN,OUTPUT);
  pinMode(LIGHT_RIGHT_PIN,OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  pinMode(HCSR04_TR_PIN, OUTPUT);
  pinMode(HCSR04_EC_PIN, INPUT);
  pinMode(IR_LINE_RIGHT_PIN, INPUT);
  pinMode(IR_LINE_LEFT_PIN, INPUT);
  pinMode(IR_RIGHT_PIN, INPUT);
  pinMode(IR_LEFT_PIN, INPUT);

  //Set up PWM for motor speed
  ledcSetup(rightMotorPWMSpeedChannel, PWMFreq, PWMResolution);
  ledcSetup(leftMotorPWMSpeedChannel, PWMFreq, PWMResolution);  
  ledcAttachPin(ENABLE_RIGHT_MOTOR_PIN, rightMotorPWMSpeedChannel);
  ledcAttachPin(ENABLE_LEFT_MOTOR_PIN, leftMotorPWMSpeedChannel);  

  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVO_PIN, 1000, 2000); // attaches the servo on pin 18 to the servo object
  myservo.write(90);  
  delay(15);

  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.attachOnDisconnect(onDisConnect);
  Ps3.begin();
  String address = Ps3.getAddress();
  Serial.print("The ESP32's Bluetooth MAC address is: ");
  Serial.println(address);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  color =  strip.Color(red,   green,   blue);
  strip.setPixelColor(0, color);
  strip.setPixelColor(1, color);
  strip.setPixelColor(2, color);
  strip.setPixelColor(3, color);
  strip.show();
  
}

void loop(){
}

void notify(){
  int yAxisValue =(Ps3.data.analog.stick.ly);  //Left stick  - y axis - forward/backward car movement
  int xAxisValue =(Ps3.data.analog.stick.lx);  //Right stick - x axis - left/right car movement
  int xServoValue =(Ps3.data.analog.stick.ry);  //Right stick - x axis - left/right car movement

  int throttle = map( yAxisValue, 127, -128, -255, 255);
  int steering = map( xAxisValue, -128, 127, -255, 255);  
  int angelServo = map( xServoValue, 127, -128, 0, 180);

  if(throttle>50){
      if(steering>50){
        Serial.print("FR: t");
        Serial.print(throttle);
        Serial.print(" - s");
        Serial.println(steering);
        int speed = abs(throttle)-abs(steering/2);
        if(speed<0) speed=0;
        digitalWrite(RIGHT_MOTOR_PIN,HIGH);
        digitalWrite(LEFT_MOTOR_PIN,HIGH);
        ledcWrite(rightMotorPWMSpeedChannel, speed);
        ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
      } else if(steering<-50){
        Serial.print("FL: ");
        Serial.println(steering);
        int speed = abs(throttle)-abs(steering/2);
        if(speed<0) speed=0;
        digitalWrite(RIGHT_MOTOR_PIN,HIGH);
        digitalWrite(LEFT_MOTOR_PIN,HIGH);
        ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
        ledcWrite(leftMotorPWMSpeedChannel, speed); 
      } else {
        Serial.print("F: ");
        Serial.println(throttle);
        digitalWrite(RIGHT_MOTOR_PIN,HIGH);
        digitalWrite(LEFT_MOTOR_PIN,HIGH);
        ledcWrite(rightMotorPWMSpeedChannel, abs(throttle));
        ledcWrite(leftMotorPWMSpeedChannel, abs(throttle)); 
      }
  } else if(throttle<-50){
      if(steering<-50){
        Serial.print("BR: ");
        Serial.println(steering);
        int speed = abs(throttle)-abs(steering/2);
        if(speed<0) speed=0;
        digitalWrite(RIGHT_MOTOR_PIN,LOW);
        digitalWrite(LEFT_MOTOR_PIN,LOW);
        ledcWrite(rightMotorPWMSpeedChannel, speed);
        ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
      } else if(steering>50){
        Serial.print("BL: ");
        Serial.println(steering);
        int speed = abs(throttle)-abs(steering/2);
        if(speed<0) speed=0;
        digitalWrite(RIGHT_MOTOR_PIN,LOW);
        digitalWrite(LEFT_MOTOR_PIN,LOW);
        ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
        ledcWrite(leftMotorPWMSpeedChannel, speed); 
      } else {
        Serial.print("B: ");
        Serial.println(throttle);
        digitalWrite(RIGHT_MOTOR_PIN,LOW);
        digitalWrite(LEFT_MOTOR_PIN,LOW);
        ledcWrite(rightMotorPWMSpeedChannel, abs(throttle));
        ledcWrite(leftMotorPWMSpeedChannel, abs(throttle)); 
      }
  } else if (steering>50){
      Serial.print("R: ");
      Serial.println(steering);
      digitalWrite(RIGHT_MOTOR_PIN,LOW);
      digitalWrite(LEFT_MOTOR_PIN,HIGH);
      ledcWrite(rightMotorPWMSpeedChannel, abs(steering));
      ledcWrite(leftMotorPWMSpeedChannel, abs(steering)); 
  } else if (steering<-50){
      Serial.print("L: ");
      Serial.println(steering);
      digitalWrite(RIGHT_MOTOR_PIN,HIGH);
      digitalWrite(LEFT_MOTOR_PIN,LOW);
      ledcWrite(rightMotorPWMSpeedChannel, abs(steering));
      ledcWrite(leftMotorPWMSpeedChannel, abs(steering)); 
  } else if ((throttle>-50 && throttle<50) || (steering>-50 && steering<50)){
      ledcWrite(rightMotorPWMSpeedChannel, MIN_SPEED);
      ledcWrite(leftMotorPWMSpeedChannel, MIN_SPEED); 
  }

  myservo.write(angelServo);

  if(Ps3.event.button_down.cross){
    digitalWrite(BUZZER_PIN,HIGH);
    delay(100);
  } else if (Ps3.event.button_up.cross){
    digitalWrite(BUZZER_PIN,LOW);
  }

  if(Ps3.event.button_down.cross){
    digitalWrite(BUZZER_PIN,HIGH);
    delay(10);
  } 
  else if(Ps3.event.button_up.cross){
    digitalWrite(BUZZER_PIN,LOW);
  }

  if(Ps3.event.button_down.r1){
    lightRightStatus=!lightRightStatus;
    digitalWrite(LIGHT_RIGHT_PIN,lightRightStatus);
  }
  if(Ps3.event.button_down.l1){
    lightLeftStatus=!lightLeftStatus;
    digitalWrite(LIGHT_LEFT_PIN,lightLeftStatus);
  }

  if(Ps3.event.button_down.left){
    colorRGB--;
    if(colorRGB<0) colorRGB=numColorRGB;
    pickColor();
  }

  if(Ps3.event.button_down.right){
    colorRGB++;
    if(colorRGB>numColorRGB) colorRGB=0;
    pickColor();
  }

  if(Ps3.event.button_down.up){
    brightRGB=brightRGB+25;
    if(brightRGB>255) brightRGB=255;
    strip.setBrightness(brightRGB);
    pickColor();
    strip.show();
    delay(10);
  }

  if(Ps3.event.button_down.down){
    brightRGB=brightRGB-25;
    if(brightRGB<0) brightRGB=0;
    strip.setBrightness(brightRGB);
    pickColor();
    strip.show();
    delay(10);
  }

}

void pickColor(){
  switch (colorRGB)
  {
  case 0:
    red=0;green=0;blue=0;
    break;
  case 1:
    red=255;green=0;blue=0;
    break;
  case 2:
    red=255;green=128;blue=0;
    break;
  case 3:
    red=255;green=255;blue=0;
    break;
  case 4:
    red=128;green=255;blue=0;
    break;
  case 5:
    red=0;green=255;blue=0;
    break;
  case 6:
    red=0;green=255;blue=128;
    break;
  case 7:
    red=0;green=255;blue=123;
    break;
  case 8:
    red=0;green=128;blue=255;
    break;
  case 9:
    red=0;green=0;blue=255;   //blue
    break;
  case 10:
    red=128;green=0;blue=255;
    break;
  case 11:
    red=255;green=0;blue=255;
    break;
  case 12:
    red=255;green=0;blue=128;
    break;
  case 13:
    red=255;green=255;blue=255;
    break;
  
  default:
    break;
  }

  color =  strip.Color(red,   green,   blue);
  strip.setPixelColor(0, color);
  strip.setPixelColor(1, color);
  strip.setPixelColor(2, color);
  strip.setPixelColor(3, color);
  strip.show();
  delay(10);
}

void onConnect(){
}

void onDisConnect(){
  rotateMotor(0, 0);   
}

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed){
  if (rightMotorSpeed < 0){
    digitalWrite(RIGHT_MOTOR_PIN,LOW);
  }
  else if (rightMotorSpeed > 0){
    digitalWrite(RIGHT_MOTOR_PIN,HIGH);
  }
  else{
    digitalWrite(RIGHT_MOTOR_PIN,LOW);
  }
  
  if (leftMotorSpeed < 0){
    digitalWrite(LEFT_MOTOR_PIN,LOW);
  }
  else if (leftMotorSpeed > 0){
    digitalWrite(LEFT_MOTOR_PIN,HIGH);
  }
  else{
    digitalWrite(LEFT_MOTOR_PIN,LOW);
  } 

  ledcWrite(rightMotorPWMSpeedChannel, abs(rightMotorSpeed));
  ledcWrite(leftMotorPWMSpeedChannel, abs(leftMotorSpeed));   
}