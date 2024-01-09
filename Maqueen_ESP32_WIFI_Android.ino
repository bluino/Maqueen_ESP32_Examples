#include <WiFi.h>
#include <ESPAsyncWebServer.h>
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
int angelServo=90;

long duration;
int distance;
bool lastTurn;

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
unsigned long previousMillis = 0;

String sta_ssid = "WiFi-Name";      // set Wifi networks you want to connect to
String sta_password = "12345678";  // set password for Wifi networks

void setup(){
  Serial.begin(115200);    // set up Serial library at 115200 bps
  Serial.println();
  Serial.println("*ESP32 WiFi Robot Remote Control Mode*");
  Serial.println("--------------------------------------");

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

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  color =  strip.Color(red,   green,   blue);
  strip.setPixelColor(0, color);
  strip.setPixelColor(1, color);
  strip.setPixelColor(2, color);
  strip.setPixelColor(3, color);
  strip.show();

  // set ESP32 Wifi hostname based on chip mac address
  char chip_id[15];
  snprintf(chip_id, 15, "%04X", (uint16_t)(ESP.getEfuseMac()>>32));
  String hostname = "Maqueen32-" + String(chip_id);
  
  Serial.println();
  Serial.println("Hostname: "+hostname);

  // first, set NodeMCU as STA mode to connect with a Wifi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.println("");
  Serial.print("Connecting to: ");
  Serial.println(sta_ssid);
  Serial.print("Password: ");
  Serial.println(sta_password);

  // try to connect with Wifi network about 10 seconds
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 5000) {
    delay(500);
    Serial.print(".");
    currentMillis = millis();
  }

  // if failed to connect with Wifi network set NodeMCU as AP mode
 if (WiFi.status() == WL_CONNECTED) {
   Serial.println("");
   Serial.println("*WiFi-STA-Mode*");
   Serial.print("IP: ");
   Serial.println(WiFi.localIP());
   delay(3000);
 } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("");
    Serial.println("WiFi failed connected to " + sta_ssid);
    Serial.println("");
    Serial.println("*WiFi-AP-Mode*");
    Serial.print("AP IP address: ");
    Serial.println(myIP);
   delay(2000);
 }


  // Send a GET request to <ESP_IP>/?fader=<inputValue>
    server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputValue;
    String inputMessage;

    // Get value for Forward/Backward
    if (request->hasParam("State")) {
      inputValue = request->getParam("State")->value();
      
      if (inputValue.equals("F")) Forward();          // check string then call a function or set a value
      else if (inputValue.equals("B")) Backward();
      else if (inputValue.equals("R")) TurnRight();
      else if (inputValue.equals("L")) TurnLeft();
      else if (inputValue.equals("G")) ForwardLeft();
      else if (inputValue.equals("H")) BackwardLeft();
      else if (inputValue.equals("I")) ForwardRight();
      else if (inputValue.equals("J")) BackwardRight();
      else if (inputValue.equals("S")) Stop();
      else if (inputValue.equals("V")) BeepHorn();
      else if (inputValue.equals("W")) TurnLightOn();
      else if (inputValue.equals("w")) TurnLightOff();
      else if (inputValue.equals("0")) {angelServo=0; colorRGB=0; SetServoRGB();}
      else if (inputValue.equals("1")) {angelServo=20; colorRGB=1; SetServoRGB();}
      else if (inputValue.equals("2")) {angelServo=40; colorRGB=3; SetServoRGB();}
      else if (inputValue.equals("3")) {angelServo=60; colorRGB=5; SetServoRGB();}
      else if (inputValue.equals("4")) {angelServo=80; colorRGB=7; SetServoRGB();}
      else if (inputValue.equals("5")) {angelServo=90; colorRGB=8; SetServoRGB();}
      else if (inputValue.equals("6")) {angelServo=100; colorRGB=9; SetServoRGB();}
      else if (inputValue.equals("7")) {angelServo=120; colorRGB=10; SetServoRGB();}
      else if (inputValue.equals("8")) {angelServo=140; colorRGB=11; SetServoRGB();}
      else if (inputValue.equals("9")) {angelServo=160; colorRGB=12; SetServoRGB();}
      else if (inputValue.equals("q")) {angelServo=180; colorRGB=13; SetServoRGB();}
      else inputValue = "No message sent";
    }
     
    Serial.println(inputValue);
    inputValue="";
    request->send(200, "text/text", "");
  });

  server.onNotFound (notFound);    // when a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();
  
}

void loop(){
}

// function to move forward
void Forward(){ 
  digitalWrite(RIGHT_MOTOR_PIN, HIGH);
  digitalWrite(LEFT_MOTOR_PIN, HIGH);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
}

// function to move backward
void Backward(){
  digitalWrite(RIGHT_MOTOR_PIN, LOW);
  digitalWrite(LEFT_MOTOR_PIN, LOW);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
}

// function to turn right
void TurnRight(){
  digitalWrite(RIGHT_MOTOR_PIN, LOW);
  digitalWrite(LEFT_MOTOR_PIN, HIGH);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
}

// function to turn left
void TurnLeft(){
  digitalWrite(RIGHT_MOTOR_PIN, HIGH);
  digitalWrite(LEFT_MOTOR_PIN, LOW);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
}

// function to move forward left
void ForwardLeft(){
  digitalWrite(RIGHT_MOTOR_PIN, HIGH);
  digitalWrite(LEFT_MOTOR_PIN, HIGH);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED*0.5); 
}

// function to move backward left
void BackwardLeft(){
  digitalWrite(RIGHT_MOTOR_PIN, LOW);
  digitalWrite(LEFT_MOTOR_PIN, LOW);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED*0.5); 
}

// function to move forward right
void ForwardRight(){
  digitalWrite(RIGHT_MOTOR_PIN, HIGH);
  digitalWrite(LEFT_MOTOR_PIN, HIGH);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED*0.5);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
}

// function to move backward left
void BackwardRight(){ 
  digitalWrite(RIGHT_MOTOR_PIN, LOW);
  digitalWrite(LEFT_MOTOR_PIN, LOW);
  ledcWrite(rightMotorPWMSpeedChannel, MAX_SPEED*0.5);
  ledcWrite(leftMotorPWMSpeedChannel, MAX_SPEED); 
}

// function to stop motors
void Stop(){  
  digitalWrite(RIGHT_MOTOR_PIN, LOW);
  digitalWrite(LEFT_MOTOR_PIN, LOW);
  ledcWrite(rightMotorPWMSpeedChannel, MIN_SPEED);
  ledcWrite(leftMotorPWMSpeedChannel, MIN_SPEED); 
}

// function to beep a buzzer
void BeepHorn(){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(150);
  digitalWrite(BUZZER_PIN, LOW);
  delay(80);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(150);
  digitalWrite(BUZZER_PIN, LOW);
  delay(80);
}

// function to turn on LED
void TurnLightOn(){
  digitalWrite(LIGHT_LEFT_PIN, HIGH);
  digitalWrite(LIGHT_RIGHT_PIN, HIGH);
}

// function to turn off LED
void TurnLightOff(){
  digitalWrite(LIGHT_LEFT_PIN, LOW);
  digitalWrite(LIGHT_RIGHT_PIN, LOW);
}

// function to move Servo and set RGB
void SetServoRGB(){
    myservo.write(angelServo);
    pickColor();
    delay(10);
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