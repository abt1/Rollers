//This code controls the full quencher, with both rollers and all peripheral devices
//For any electronics issues, please check the Arduino Project Book
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 8, 6, 13);  //Connection pins for LCD screen
int const sensorAPin = A0;  //Sensor pin numbers
int const sensorBPin = A1;
int sensorAValue = 0;
int sensorBValue = 0;
const int motorAPin = 9;  //Motor pin numbers
const int motorBPin = 3;
long firstTimeA = 0;
long secondTimeA = 0;
long firstTimeB = 0;
long secondTimeB = 0;
long thirdTimeB = 0;
int stateA = 1;
int stateB = 1;
int targetA = 800;  //Initial targets for roller speeds in rpm
int targetB = 800;
float powerA = 120;
float powerB = 120;
int n = 0;
int const greenLEDPin = 7;  //LED pin numbers
int const redLEDPin = 4;
int convergence = 0;
int initial_convergence = 0;
int const switchPin = 2;  //Button pin number
int switchState = 0;
int const potPin = A2;  //Potentiometer pin number
int potVal = 0;
int target;
int targetTrunc;


int speed_control(int rpm, float *power, int target);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
  analogWrite(motorAPin, 0); //Keeps both rollers off until target is set
  analogWrite(motorBPin, 0);
  pinMode(switchPin, INPUT);
  pinMode(potPin, INPUT);
  switchState = digitalRead(switchPin);
  do {  //Uses potentiometer to set target speed.  Speed is confirmed when button is pressed.
    switchState = digitalRead(switchPin);
    potVal = analogRead(potPin);
    target = map(potVal, 0, 1023, 500, 1200);
    targetTrunc = target/50;
    targetA = targetTrunc*50;
    targetB = targetTrunc * 50;
    Serial.println(targetB);
    lcd.setCursor(0,0);
    lcd.print("Target:");
    lcd.print(targetB);
    lcd.print("   ");
    delay(100);
  } while(switchState == 0);
  Serial.print("Target: ");  //Target confirmed and rollers initiated
  Serial.println(targetA);
  pinMode(motorAPin, OUTPUT);
  pinMode(motorBPin, OUTPUT);
  analogWrite(motorAPin, 255-powerA);
  analogWrite(motorBPin, 255-powerB);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  digitalWrite(greenLEDPin, LOW);
  digitalWrite(redLEDPin, HIGH);
  delay(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  convergence = 0;
  
  //This code looks for a black section, then a white section, then only when it detects another black section, it records a rotation.  
  //It then calculates the rpm from the period of that rotation.
  while(n == 0) {
    sensorAValue = analogRead(sensorAPin);
    //Serial.println(sensorAValue);
    if(sensorAValue < 500 && stateA == 1){
      if(firstTimeA == 0) {
        firstTimeA = micros();
      }  else {
        secondTimeA = micros();
      }
      stateA = 0;
    }
    if(sensorAValue > 500) {
      stateA = 1;
    }
    if(secondTimeA !=0) {
      long revTimepre = firstTimeA-secondTimeA;
      long revTime = abs(revTimepre);
      long rpm = 60000000/ revTime;
      lcd.setCursor(0,1);
      lcd.print("A: ");
      lcd.print(rpm);
      if(rpm < 1000) {
        lcd.setCursor(6, 1);
        lcd.print("  ");
      } else {
        lcd.setCursor(7, 1);
        lcd.print(" ");
      }
      Serial.print("******RPM A*******");
      Serial.println(rpm);
      firstTimeA = 0;
      secondTimeA = 0;
      speed_control(rpm, &powerA, targetA, motorAPin);
      //targetB = rpm;
      n = 1;
      if(abs(rpm - targetA) < 200) {
        if(initial_convergence == 0 && abs(rpm - targetA) < 100) {
          convergence += 1;
        } else if(initial_convergence == 1) {
        convergence += 1;
        }
      }
    }
  }
  n = 0;
  while(n == 0) {
    sensorBValue = analogRead(sensorBPin);
    //Serial.println(sensorAValue);
    if(sensorBValue < 500 && stateB == 1){
      if(firstTimeB == 0) {
        firstTimeB = micros();
      }  else {
        secondTimeB = micros();
      }
      stateB = 0;
    }
    if(sensorBValue > 900) {
      stateB = 1;
    }
    if(secondTimeB !=0) {
      long revTimepre = firstTimeB-secondTimeB;
      long revTime = abs(revTimepre);
      long rpm = abs(60000000/ revTime);
      Serial.print("******RPM B*******");
      Serial.println(rpm);
      lcd.setCursor(8,1);
      lcd.print("B: ");
      lcd.print(rpm);
      if(rpm < 1000) {
        lcd.setCursor(14, 1);
        lcd.print("  ");
      } else {
        lcd.setCursor(15, 1);
        lcd.print(" ");
      }
      firstTimeB = 0;
      secondTimeB = 0;
      speed_control(rpm, &powerB, targetB, motorBPin);
      n = 1;
      if(abs(rpm - targetB) < 200) {
        if(initial_convergence == 0 && abs(rpm - targetB) < 100) {
          convergence += 1;
        } else if(initial_convergence == 1) {
        convergence += 1;
        }
      }
    }
  }
  n = 0;
  if(convergence == 2) {
    digitalWrite(greenLEDPin, HIGH);
    digitalWrite(redLEDPin, LOW);
    initial_convergence = 1;
  } else {
    digitalWrite(greenLEDPin, LOW);
    digitalWrite(redLEDPin, HIGH);
  }
}

int speed_control(int rpm, float *power, int target, int pinNo) {
  //This function takes in the speed, power, and target speed of a motor, and adjusts the power to move the speed closer to the target.
  if(rpm < target - 100) {
    *power += 0.5;
  } else if(rpm > target + 100) {
    *power -= 0.5;
  } else if(rpm > target + 50) {
    *power -= 0.25;
  } else if(rpm < target - 50) {
    *power += 0.25;
  } else if(rpm < target - 20) {
    *power += 0.1;
  } else if(rpm > target + 20) {
    *power -= 0.1;
  }
  
  if(*power > 255) {
    *power = 255;
    Serial.println("***MAX POWER REACHED***");
  }
  if(*power < 1) {
    *power = 0;
    Serial.println("***MIN POWER REACHED***");
  }
  float writePower = 255 - *power;
  analogWrite(pinNo, writePower);
  Serial.print("POWER: ");
  Serial.println(*power);
  if(abs(target - rpm) > 500){
    delay(100);
  } else{
    delay(200);
  }
  return 1;
}