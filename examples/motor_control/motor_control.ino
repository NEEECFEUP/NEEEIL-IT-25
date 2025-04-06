/*
*  This is a simple code example using the Nelito library for controling motors
*  with the NEEEIL IT 25 motor shield. It will make the motors spin in one direction
*  after the built in button is pressed.
*
*  NEEEIL IT 25 by NEEEC-FEUP
*/

#include <nelito.h>

// Motor shield built-in button.
#define BTN_PIN 22

// Encoder pins
// Only the A side of the encoders are needed to call the related 
// functions however, make sure each A and B pair are consecutive.
#define ENCL_A 11
#define ENCL_B 12
#define ENCR_A 20
#define ENCR_B 21

// Structs to hold motor and movement variables.
typedef struct{
  int v_left;
  int v_right;
  float slip_comp;
}motor_t;

typedef struct{
  float v_lin;
  float v_ang;
}move_t;

motor_t motor = {0, 0, 1.02};
move_t move = {0, 0};

// The following code will run only once
void setup() 
{
  // Begin communication
  Serial.begin(115200);
  
  // Initialize motors
  motor_init();
  // Initialize encoders
  encoder_init(ENCL_A, ENCR_A);
  // Use default sample rate
  // set_sample_rate(25);

  Serial.println("Press button to begin!");
  while(digitalRead(BTN_PIN) == LOW){} // Wait untill button is pressed
  delay(500);

  // Set linear and angular velocities as a percentage of the max value.
  move.v_lin = 60; // de 0 a 100
  move.v_ang = 0;

}

void loop() 
{
  // Map the values in percentage to the int range and compensate for slip.
  // In this case the left motor was rotating faster than the right motor for a given value
  // so we adjust the right velocity by some constant (motor.slip_comp). This value needs to be 
  // adjusted for your motors and you may also need to compensate the left side instead of the right.
  motor.v_left = map(move.v_lin + 0.5*move.v_ang, 0, 100, -32767, 32766);
  motor.v_right = map(move.v_lin*motor.slip_comp - 0.5*move.v_ang, 0, 100, -32767, 32766);

  // Then we need to actually update the values being sent to the motors.
  // These values will continue to be applied. If you want to stop the motor you need to set
  // their velocity to zero.
  set_motor(motor.v_left, motor.v_right);

}
