/*
 * ------------------------------------------------------------------
 *  RC Plane Autopilot - automatic takeoff, demo turn and landing
 * ------------------------------------------------------------------
 *  Board  : Arduino Uno / Nano (ATmega328P)
 *  Wiring : D10 -> ESC signal (throttle)
 *           D9  -> aileron servo
 *           D11 -> elevator servo
 *           D8  -> ultrasonic sensor TRIG
 *           D7  -> ultrasonic sensor ECHO
 *           GND -> common ground for all devices
 *  Author : Swayam Khantwal (github.com/Swayam7024)
 *  License: MIT
 *
 *  The flight sequence starts automatically on power-up.
 *  ALWAYS remove the propeller when testing on the bench.
 * ------------------------------------------------------------------
 */

#include <Servo.h>

// ---------------- Pins ----------------
const int THROTTLE_PIN = 10;   // ESC (Electronic Speed Controller) signal
const int AILERON_PIN  = 9;    // aileron servo
const int ELEVATOR_PIN = 11;   // elevator servo
const int TRIG_PIN     = 8;    // ultrasonic TRIG
const int ECHO_PIN     = 7;    // ultrasonic ECHO

// ---------------- Throttle (µs) ----------------
const int THROTTLE_OFF = 900;  // ESC minimum / motor off

// ---------------- Ultrasonic ----------------
const unsigned long ECHO_TIMEOUT_US = 30000; // ~5 m round trip; stops pulseIn() from blocking
const int NO_ECHO_DISTANCE_CM = 500;         // value used when no echo comes back (out of range)

// ---------------- Altitude limits (cm) ----------------
const int TAKEOFF_FAIL_CM  = 20;   // still this close to the ground -> takeoff failed
const int TAKEOFF_OK_CM    = 50;   // at or above this height -> takeoff successful
const int LANDING_FLARE_CM = 100;  // within this height -> flare and cut throttle

// ---------------- Safety timeouts (ms) ----------------
const unsigned long TAKEOFF_CLIMB_TIMEOUT_MS = 10000; // must reach TAKEOFF_OK_CM within this time
const unsigned long LANDING_TIMEOUT_MS       = 20000; // cut throttle anyway after this time

int throttleValue;
Servo throttle;
Servo aileron;
Servo elevator;
int disincm;

// Measure the distance to the ground in centimetres.
// Returns NO_ECHO_DISTANCE_CM if no echo is received within the timeout.
int readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);          // HC-SR04 needs a 10 µs trigger pulse
  digitalWrite(TRIG_PIN, LOW);

  unsigned long timeinmicro = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (timeinmicro == 0) {
    return NO_ECHO_DISTANCE_CM;   // no echo -> treat as out of range
  }
  return timeinmicro / 29 / 2;    // distance in centimetres
}

// Stop the motor, neutralise the control surfaces and stay there forever.
void enterSafeIdle(const char *reason) {
  throttle.writeMicroseconds(THROTTLE_OFF);
  aileron.write(90);
  elevator.write(90);
  Serial.print("SAFE IDLE: ");
  Serial.println(reason);
  while (true) {
    throttle.writeMicroseconds(THROTTLE_OFF);  // keep the motor off
    delay(100);
  }
}

void setup() {
  Serial.begin(9600);

  throttle.attach(THROTTLE_PIN);   // ESC signal wire on pin 10
  aileron.attach(AILERON_PIN);     // aileron servo on pin 9
  elevator.attach(ELEVATOR_PIN);   // elevator servo on pin 11
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Arm the ESC by setting the throttle to the minimum position
  throttle.writeMicroseconds(THROTTLE_OFF);
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

  // Use the ultrasonic sensor to check the height above the ground.
  unsigned long climbStart = millis();
  while (true) {
    disincm = readDistanceCm();
    Serial.println(disincm);

    if (disincm <= TAKEOFF_FAIL_CM) {
      // Plane failed to take off -> throttle to 0 % and stop the sequence
      enterSafeIdle("takeoff failed (too close to the ground)");
    } else if (disincm >= TAKEOFF_OK_CM) {
      break;  // takeoff successful
    }

    if (millis() - climbStart > TAKEOFF_CLIMB_TIMEOUT_MS) {
      enterSafeIdle("takeoff failed (did not climb in time)");
    }
    delay(60);  // let the previous echo die out before the next measurement
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

  // Wait until the plane is low, then flare (elevator up) and cut the throttle.
  unsigned long landingStart = millis();
  while (true) {
    disincm = readDistanceCm();
    Serial.println(disincm);

    if (disincm <= LANDING_FLARE_CM) {
      elevator.write(60);
      throttle.writeMicroseconds(THROTTLE_OFF);
      delay(500);
      break;  // flare done
    }

    if (millis() - landingStart > LANDING_TIMEOUT_MS) {
      Serial.println("Landing timeout - cutting throttle.");
      break;
    }
    delay(60);
  }

  // Landing finished: motor off, surfaces neutral, stay here.
  enterSafeIdle("landing complete");
}
