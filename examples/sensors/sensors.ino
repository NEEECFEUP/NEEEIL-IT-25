/*
*  This examples prints the values being read by NEEEC Sense and the
*  IR receiver connected to the NEEEIL IT 25 motor shield using
*  the Nelito library.
*
*  NEEEIL IT 25 by NEEEC-FEUP
*/

#include <nelito.h>

// Built-in button pin
#define BTN_PIN 22
// IR receiver module pin
#define IR_RCV_PIN 12

// NEEEC SENSE DEFINES
#define IR1 13
#define IR2 16
#define IR3 17
#define IR4 18
#define IR5 19
#define PWM_SENSE 20

// Is a shortcut being detected?
bool shortcut = false;

// Sense IR sensor value array
bool IR_values[5];

// Just used for the formated string
char message[100];

void setup() 
{
  // Begin communication
  Serial.begin(115200);

  // Initialize Sense
  sense_init(IR1, IR2, IR3, IR4, IR5);
  // If we are using PWM mode to control the comparator refrence input
  // of the IR sensores in Sense we need to supply a PWM signal.
  analogWrite(PWM_SENSE, 150); // This value may need to be adjusted for your sensor.

  // Initialize the pin connected to the IR Reciever as an input.
  pinMode(IR_RCV_PIN, INPUT);
}

void loop() 
{
  // Read IR module 
  if (!digitalRead(IR_RCV_PIN))
    shortcut = true;
  else
    shortcut = false;

  // Read IR sensor values from sense
  // Keep in mind that the values you are reading are inverted!
  // This means that in the presence of a line, you are supposed to read
  // a '0' on that pin.
  sense_read(IR_values);

  // Print everything to the monitor every 250ms.
  sprintf(message, "|Sensor: %d%d%d%d%d||Side sensor: %d|", IR_values[0], IR_values[1], IR_values[2],
                                                            IR_values[3], IR_values[4], shortcut);
  Serial.println(message);

  delay(250);
}
