
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position
bool g = true;
int i;
int instruction, amount, thespeed;


void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(90);
  pos = 90;
  delay(1000);
  Serial.begin(9600);
}

int look_left(int amount, int speed){
  //Relative to a person looking at the camera
  int delay_time = 1000/speed;
  if(pos-amount >= 0){
    for(i=0; i<amount; i++){
      pos -= 1;
      myservo.write(pos);
      delay(delay_time);
    }
  }
}

int look_right(int amount, int speed){
  //Relative to a person looking at the camera
  int delay_time = 1000/speed;
  if(pos+amount <=180){
    for(i=0; i<amount; i++){
      pos += 1;
      myservo.write(pos);
      delay(delay_time);
    }
  }
}

void loop() {
  if(false){
    myservo.write(90);
    pos = 90;
    delay(5000);
    look_left(40, 10);
    delay(1000);
    look_right(80, 10);
    delay(1000);
    g = false;
  }

  if(Serial.available() > 0){
    //instruction = Serial.parseInt();
    instruction = Serial.parseInt();
    amount = Serial.parseInt();
    thespeed = Serial.parseInt();
    Serial.read();
    // Serial.print(instruction);
    //Serial.print(",");
    //Serial.print(amount);
    //Serial.print(",");
    //Serial.print(thespeed);
    //Serial.print("\n");

    switch(instruction){
    case 0:
      look_left(amount, thespeed);
      break;
    case 1:
      look_right(amount, thespeed);
      break;
    default:
      break;
    }
    Serial.println(pos);
  }
}
  

