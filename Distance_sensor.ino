#include <LiquidCrystal.h>
#include <Servo.h>
#define LED_1_PIN 11
#define LED_2_PIN 12
#define LED_3_PIN 13
#define ECHO_PIN 3
#define TRIGGER_PIN 4
#define SERVO_PIN 10
#define LCD_RS_PIN A5
#define LCD_E_PIN A4
#define LCD_D4_PIN 6
#define LCD_D5_PIN 7
#define LCD_D6_PIN 8
#define LCD_D7_PIN 9

LiquidCrystal lcd(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN,
                  LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);


int cursorLine=0;
Servo myservo;
unsigned long lastTimeUltrasonicTrigger = millis();
unsigned long ultrasonicTriggerDelay = 100;

volatile unsigned long pulseInTimeBegin;
volatile unsigned long pulseInTimeEnd;
volatile bool newDistanceAvilable=false;

double previousDistance = 400.0;

void triggerUltrasonicSensor()
{
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
}

double getUltrasonicDistance()
{
  double durationMicros = pulseInTimeEnd - pulseInTimeBegin;
  //Serial.println(durationMicros);
  double distance = durationMicros / 55.0; // cm (140.0: inches)
  if(distance > 400.0)
   {
    return previousDistance;
   }
  distance = previousDistance * 0.7 + distance * 0.3;
  previousDistance=distance;
  return distance;
}

void echoPinInterrupt()
{
  if(digitalRead(ECHO_PIN) == HIGH)
  {
    pulseInTimeBegin = micros();
  }
  else
  {
    pulseInTimeEnd = micros();
    newDistanceAvilable= true;
  }
}

void powerLedFromDistance(double distance)
{
   if(distance >= 100)
   {
     //power on green
     digitalWrite(LED_1_PIN, LOW);
     digitalWrite(LED_2_PIN, LOW);
     digitalWrite(LED_3_PIN, HIGH);
     myservo.write(60);
   }
   else if(distance >= 15)
   {
     // power on yellow
     digitalWrite(LED_1_PIN, LOW);
     digitalWrite(LED_2_PIN, HIGH);
     digitalWrite(LED_3_PIN, LOW);
     myservo.write(60);
   }
  else
  {
     // power on red
     digitalWrite(LED_1_PIN, HIGH);
     digitalWrite(LED_2_PIN, LOW);

     digitalWrite(LED_3_PIN, LOW);
     //Serial.print("There is an obstacle @ distance: ");
     Serial.println(distance);
     //Serial.println(" cm");
     myservo.write(155);
  }
}

void printDistanceOnLcd(double distance)
{
   lcd.setCursor(0, 1);
   lcd.print("Distance: ");
   lcd.print(distance);
   lcd.print("  ");
}

void setup() {
  Serial.begin(115200);
  myservo.attach(SERVO_PIN);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Rate: ");
  lcd.print(ultrasonicTriggerDelay);
  lcd.print(" ms. ");
  
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN),
                   echoPinInterrupt,
                   CHANGE);
}

void loop() {
  unsigned long timeNow = millis();
    
  if (timeNow - lastTimeUltrasonicTrigger > ultrasonicTriggerDelay) {
    lastTimeUltrasonicTrigger += ultrasonicTriggerDelay;
    triggerUltrasonicSensor();
  }
  if(newDistanceAvilable)
  {
     newDistanceAvilable=false;
     double distance = getUltrasonicDistance();
     printDistanceOnLcd(distance);
     powerLedFromDistance(distance);
     //Serial.println(distance);
  }
  //----------------------------SERVO PART-----------------------------------------------------

    
  
}