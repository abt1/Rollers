//This code controls the complete core quencher- two rollers and sensors, but no peripherals except a red and a green LED
//Refer to Arduino Project Book for explanation of how to connect things
//Author: Alex Todd, 22/04/2020, University of Bath
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
int const targetA = 800;  //Targets set for each roller
int targetB = 800;
float powerA = 120;
float powerB = 120;
int n = 0;
int const greenLEDPin = 7; //LED pin numbers
int const redLEDPin = 4;
int convergence = 0;
int initial_convergence = 0;


int speed_control(int rpm, float *power, int target);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //Motors and output initiated
  pinMode(motorAPin, OUTPUT);
  pinMode(motorBPin, OUTPUT);
  analogWrite(motorAPin, 255-powerA);
  analogWrite(motorBPin, 255-powerB);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  digitalWrite(greenLEDPin, LOW);
  digitalWrite(redLEDPin, HIGH);
  delay(5000);  //Delay to allow motors to accelerate before optimisation
}

void loop() {
  // put your main code here, to run repeatedly:
  
  convergence = 0;
  
  //This code looks for a black section, then a white section, then only when it detects another black section, it records a rotation.  It then calculates the rpm from the period of that rotation.
  while(n == 0) {
    sensorAValue = analogRead(sensorAPin);
    //Serial.println(sensorAValue);
    if(sensorAValue < 500 && stateA == 1){  //Checks whether sensor is reading white section of disk, and records time if so
      if(firstTimeA == 0) {
        firstTimeA = micros();
      }  else {
        secondTimeA = micros();
      }
      stateA = 0;
    }
    if(sensorAValue > 500) {
      stateA = 1; //Ensures that a black section is recorded between the two white readings
    }
    if(secondTimeA !=0) {  //Computes RPM as time between white sections
      long revTimepre = firstTimeA-secondTimeA;
      long revTime = abs(revTimepre);
      long rpm = 60000000/ revTime;
      Serial.print("******RPM A*******");
      Serial.println(rpm);
      firstTimeA = 0;
      secondTimeA = 0;
      speed_control(rpm, &powerA, targetA, motorAPin);
      //targetB = rpm;
      n = 1;
      if(abs(rpm - targetA) < 200) { //Checks for convergence of the roller speeds on their targets
        if(initial_convergence == 0 && abs(rpm - targetA) < 100) {
          convergence += 1;
        } else if(initial_convergence == 1) {
        convergence += 1;
        }
      }
    }
  }
  n = 0;
  while(n == 0) {  //Repeats everything for the other roller
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
  n = 0;  //Lights green LED if rollers are converged, red one otherwise
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
    *power += 5;
  } else if(rpm > target + 100) {
    *power -= 5;
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
