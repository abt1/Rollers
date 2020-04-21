//This script runs one roller at a controlled speed
//See Arduino Project Book for explanation of how to connect things
int const sensorPin = A0;  //Pin sensor is connected to
int sensorValue = 0;
const int motorPin = 9;  //Pin motor is connected to
long firstTime = 0;
long secondTime = 0;
int state = 1;
int const target = 1500;    //Speed target set here
float power = 100;    //Initial power
int speed_control(int rpm, float *power, int target);
int n;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(motorPin, OUTPUT);
  analogWrite(motorPin, power);  //Starts motor running
  delay(3000); //Delay allows motor to accelerate before optimisation begins.
}

void loop() {
  n = 0;
  // put your main code here, to run repeatedly:
  while(n == 0) {
    sensorValue = analogRead(sensorPin); //Reads sensor value
    //Serial.println(sensorValue);  //Prints out sensor value (for debugging)
    if(sensorValue < 600 && state == 1){ //Checks colour of disk segment in view, and records time if white
      if(firstTime == 0) {
        firstTime = micros();
      }  else {
        secondTime = micros();
      }
      state = 0;
    }
    if(sensorValue > 600) {
       state = 1;
    }
    if(firstTime != 0 && secondTime !=0) { //RPM calculated from first and second white section times
      long prerevTime = firstTime-secondTime;
      long revTime = abs(prerevTime);
      int rpm = abs(60000000/ revTime);
      rpm = rpm;
      Serial.print("******RPM*******");
      Serial.println(rpm);
      firstTime = 0;
      secondTime = 0;
      speed_control(rpm, &power, target);
      n = 1;
    }
  }
}

int speed_control(int rpm, float *power, int target) {
  //Function compares the current rpm to the target, and adjusts accordingly
  //Note power is inverted due to arrangement of transistors
  if(rpm < target - 100) {
    *power -= 0.5;
  } else if(rpm > target + 100) {
    *power += 0.5;
  }
  if(*power > 255) {
    *power = 255;
    Serial.println("***MIN POWER REACHED***");
  }
  if(*power < 1) {
    *power = 0;
    Serial.println("***MAX POWER REACHED***");
  }
  float writePower = 255-*power;
  analogWrite(motorPin, *power);
  Serial.print("POWER: ");
  Serial.println(255-*power);
  if(abs(target - rpm) > 500){
    delay(103);
  } else{
    delay(203);
  }
  return 1;
}