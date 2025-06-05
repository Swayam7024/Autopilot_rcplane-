#include <Servo.h>

int throttleValue;
Servo throttle;
Servo aileron;
Servo elevator;
int trig = 8;
int echo = 7;
int timeinmicro;
int disincm;
int i;


void setup() {
  Serial.begin(9600);

  throttle.attach(10);   // Connect the ESC (Electronic Speed Controller) signal wire to pin 9
  aileron.attach(9);   // Connect aileron servo to pin 10
  elevator.attach(11);  // Connect elevator servo to pin 11
  pinMode(8, OUTPUT);
  pinMode(7, INPUT);

  // Arm the ESC by setting the throttle to the minimum position from 700 to 2300
  throttle.writeMicroseconds(900);
  delay(4000);  // Wait for 4 seconds to allow ESC to initialize

  Serial.println("RC plane ready for takeoff!");
}

void loop() {
  // Gradually increase throttle for takeoff
  for (throttleValue = 1000; throttleValue <= 1500; throttleValue += 10) {
    delay(100);
    throttle.writeMicroseconds(throttleValue);
    Serial.println(throttleValue);
  }

  // Set elevator to a slightly up position
  elevator.write(60);
  delay(2000);

  // using ultrasonic sensor for distance measurement from ground.

   for (i = 0; ; i++) {
    digitalWrite(trig, LOW);
    delay(20);
    digitalWrite(trig, HIGH);
    delay(100);
    digitalWrite(trig, LOW);

    timeinmicro = pulseIn(echo, HIGH);
    disincm = timeinmicro / 29 / 2;     // distance is stored in (disincm) in centimeter.
    Serial.println(disincm);
 
    if (disincm <= 20) {       // if plane fails to takeoff then the throttle will be reduced to (0%)
      throttle.writeMicroseconds(900);
    } else if (disincm >= 50) {
      break;
   }
  }
  
  

  // Adjust aileron and elevator for stabilization after takeoff
  aileron.write(90);    // Set aileron to neutral position
  elevator.write(90);   // Set elevator to a neutral position
  delay(2000);

  // Wait for a moment after reaching the takeoff throttle
  delay(2000);

  // Reduce throttle to idle
  throttle.writeMicroseconds(1500);
  delay(200);
  throttle.writeMicroseconds(1400);
  delay(200);
  throttle.writeMicroseconds(1300);
  delay(200);

  Serial.println("Takeoff complete. Plane is at idle throttle.");
  delay(4000);

  // Turn right for demonstration purposes
  aileron.write(60);
  elevator.write(60);
  delay(5000);

  // Set aileron and elevator to neutral position
  aileron.write(90);
  elevator.write(90);

  // Prepare for landing
  for (throttleValue = 1300; throttleValue >= 1000; throttleValue -= 10) {
    delay(500);
    throttle.writeMicroseconds(throttleValue);
    Serial.println(throttleValue);
  }

    // Increase elevator angle for landing
    elevator.write(90);
    delay(500);
    elevator.write(60);
    delay(1000);
    elevator.write(90);
    delay(500);

    // If altitude is low, reduce throttle and elevator angle
     for (i = 0; ; i++) {
    digitalWrite(trig, LOW);
    delay(20);
    digitalWrite(trig, HIGH);
    delay(100);
    digitalWrite(trig, LOW);

    timeinmicro = pulseIn(echo, HIGH);
    disincm = timeinmicro / 29 / 2;     // distance is stored in (disincm) in centimeter.
    Serial.println(disincm);
     
    if (disincm <= 100) {
       elevator.write(60);
       throttle.writeMicroseconds(900);
       delay(500);
    }
    else if (disincm >= 50) {
      break;
     }
     }
      elevator.write(90);
  

  // Set throttle to idle
  throttle.writeMicroseconds(900);

  // Loop indefinitely
  while (true) {
  }
}

  
