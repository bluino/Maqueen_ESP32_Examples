/* 
  Install ultrasonic sensor HC-SR04
  Press Button connected to Pin 5 for start/stop Maquee32 robot move
*/

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
  Serial.begin(115200);    // set up Serial library at 115200 bps
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
  // Press button connected pin 5 to start robot as obstacle
  if(digitalRead(BUTTON_PIN)==0){
    buttonStatus=!buttonStatus;
    delay(200);
  }
  
  if(buttonStatus){
    ObstacleAvoidance();
  } else {
    colorRGB=0;
    pickColor();
    digitalWrite(RIGHT_MOTOR_PIN,LOW);
    digitalWrite(LEFT_MOTOR_PIN,LOW);
    ledcWrite(rightMotorPWMSpeedChannel, MIN_SPEED);
    ledcWrite(leftMotorPWMSpeedChannel, MIN_SPEED);
  }
}

void ObstacleAvoidance(){
  digitalWrite(HCSR04_TR_PIN, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(HCSR04_TR_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(HCSR04_TR_PIN, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(HCSR04_EC_PIN, HIGH);

  // Calculating the distance
  distance= duration/58.26;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  if(distance<13){
    Serial.print("Distance: ");
    Serial.println(distance);
    if(digitalRead(IR_LEFT_PIN)){
      colorRGB=1;
      pickColor();
      digitalWrite(RIGHT_MOTOR_PIN,LOW);
      digitalWrite(LEFT_MOTOR_PIN,HIGH);
      ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
      ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
      lastTurn=0;
    } else if(digitalRead(IR_RIGHT_PIN)){
      colorRGB=5;
      pickColor();
      digitalWrite(RIGHT_MOTOR_PIN,HIGH);
      digitalWrite(LEFT_MOTOR_PIN,LOW);
      ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
      ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
      lastTurn=1;
    } else {
      if(lastTurn){
        colorRGB=1;
        pickColor();
        digitalWrite(RIGHT_MOTOR_PIN,HIGH);
        digitalWrite(LEFT_MOTOR_PIN,LOW);
        ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
        ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
      } else {
        colorRGB=5;
        pickColor();
        digitalWrite(RIGHT_MOTOR_PIN,LOW);
        digitalWrite(LEFT_MOTOR_PIN,HIGH);
        ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
        ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
      }
    }
  } else if(digitalRead(IR_LEFT_PIN)){
    colorRGB=1;
    pickColor();
    lastTurn=0;
    digitalWrite(RIGHT_MOTOR_PIN,LOW);
    digitalWrite(LEFT_MOTOR_PIN,HIGH);
    ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED*0.5);
    ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
  } else if(digitalRead(IR_RIGHT_PIN)){
    colorRGB=5;
    pickColor();
    lastTurn=1;
    digitalWrite(RIGHT_MOTOR_PIN,HIGH);
    digitalWrite(LEFT_MOTOR_PIN,LOW);
    ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
    ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED*0.5); 
  }
  else {
    colorRGB=9;
    pickColor();
    digitalWrite(RIGHT_MOTOR_PIN,HIGH);
    digitalWrite(LEFT_MOTOR_PIN,HIGH);
    ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
    ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
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
}